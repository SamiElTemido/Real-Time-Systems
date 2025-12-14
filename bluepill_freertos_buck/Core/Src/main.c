#include "main.h"
TIM_HandleTypeDef htim1;
UART_HandleTypeDef huart1;
ADC_HandleTypeDef hadc1;

TimerHandle_t xTimer;
QueueHandle_t xqueue;
SemaphoreHandle_t semParse;
SemaphoreHandle_t semSample;


 float ref = 12;
 int32_t ref_value = 1830;
 int32_t error = 0;
 int32_t adc_value = 0;
 int32_t DutyCycle = 1000;

 float Kp = 0.25;
 float ki = 6.0;
 float Kd = 0.01;

 uint8_t idx, byteRec, buffin[64];

 typedef struct {
     float voltage;
     int32_t adc_ref;
 } LutEntry_t;

 #define TABLE_SIZE 18

 const LutEntry_t ADC_LUT[TABLE_SIZE] = {
	 { 2.0, 320},
	 { 3.0, 470  },
     { 4.0, 625  },
     { 5.0, 785  },
     { 6.0, 930  },
     { 7.0, 1075 },
     { 8.0, 1230 },
     { 9.0, 1385 },
     { 10.0, 1535 },
     { 11.0, 1680 },
     { 12.0, 1830 },
     { 13.0, 1990 },
     { 14.0, 2140 },
     { 15.0, 2295 },
     { 16.0, 2445 },
     { 17.0, 2595 },
     { 18.0, 2745 },
	 { 19.0, 2900}
 };

int main(void)
{
	System_Init();
	xqueue = xQueueCreate(64, sizeof(char));
	semParse = xSemaphoreCreateBinary();
	semSample = xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
	xTimerStart(xTimer, 0);
	xTaskCreate(control_task, "Control", configMINIMAL_STACK_SIZE * 4, NULL, 4, NULL);
	xTaskCreate(parse_task, "Parse", configMINIMAL_STACK_SIZE * 8, NULL, 1, NULL);
	vTaskStartScheduler();
	return 0;
}
void control_task(void *pvParameter)
{
    UNUSED(pvParameter);
    char buffer[64];
    unsigned char buflen;

    #define ADC_AVG_SAMPLES 16
    static uint32_t adc_buffer[ADC_AVG_SAMPLES];
    static uint8_t  adc_buffer_index = 0;
    static uint32_t adc_sum = 0;

    // Initialize ADC buffer
    for (int i = 0; i < ADC_AVG_SAMPLES; i++) {
        adc_buffer[i] = HAL_ADC_GetValue(&hadc1);
        adc_sum += adc_buffer[i];
        vTaskDelay(pdMS_TO_TICKS(1));
        ref_value = GET_REFVALUE(ref);
    }
    adc_value = adc_sum / ADC_AVG_SAMPLES;

    int32_t pre_error = 0;
	float i_term = 0.0f;

	uint16_t print_counter = 0;
	const uint16_t PRINT_INTERVAL = 6;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1);

    xLastWakeTime = xTaskGetTickCount();

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        adc_sum -= adc_buffer[adc_buffer_index];

        uint32_t new_adc_sample = HAL_ADC_GetValue(&hadc1);

        adc_buffer[adc_buffer_index] = new_adc_sample;

        adc_sum += new_adc_sample;

        adc_buffer_index = (adc_buffer_index + 1) % ADC_AVG_SAMPLES;

        adc_value = adc_sum / ADC_AVG_SAMPLES;

        error = -adc_value + ref_value;

        float Up = Kp * (float)error;

		i_term += (float)error * TS;
		if (i_term > EMAX) i_term = EMAX;
		if (i_term < -EMAX) i_term = -EMAX;
		float Ui = ki * i_term;

		float Ud = Kd * ((float)(error - pre_error));

		pre_error = error;

		float unsat = Up + Ui + Ud;

		if (unsat > CMAX) DutyCycle = CMAX;
		else if (unsat < CMIN) DutyCycle = CMIN;
		else DutyCycle = unsat;

		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, DutyCycle);

		print_counter++;
		if (print_counter >= PRINT_INTERVAL)
		{
		    print_counter = 0;

		    buflen = sprintf(buffer, "ADC: %li\tREF: %0.2f\tERR: %li\tDC: %li\tref_v: %li\r\n",
		                     adc_value, ref, error, DutyCycle,ref_value);

		    for(int i = 0; i < buflen; i++) {
		        xQueueSend(xqueue, buffer+i, 0);
		    }
		}
    }
}

void blinkFunction(TimerHandle_t xTimer)
{
	UNUSED(xTimer);
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
void vApplicationIdleHook(void){

	unsigned char byteToSend;

	if(xQueueReceive(xqueue, &byteToSend,0) == pdPASS)
	{
		HAL_UART_Transmit(&huart1, &byteToSend, 1, HAL_MAX_DELAY);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	if (byteRec == '\n')
	{
		buffin[idx++] = '\0';
		idx = 0;
		xSemaphoreGiveFromISR(semParse, &xHigherPriorityTaskWoken);
	}
	else if (byteRec != '\r')
	{
		buffin[idx++] = byteRec;
	}
	HAL_UART_Receive_IT(&huart1, &byteRec, 1);

	if (xHigherPriorityTaskWoken == pdTRUE)
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int32_t GET_REFVALUE(float V_ref)
{
    if (V_ref <= ADC_LUT[0].voltage) {
        return ADC_LUT[0].adc_ref;
    }
    if (V_ref >= ADC_LUT[TABLE_SIZE - 1].voltage) {
        return ADC_LUT[TABLE_SIZE - 1].adc_ref;
    }

    int low = 0;
    int high = TABLE_SIZE - 1;
    int mid;

    while (low < high - 1) {
        mid = low + (high - low) / 2;

        if (V_ref < ADC_LUT[mid].voltage) {
            high = mid;
        } else {
            low = mid;
        }
    }

    float V0 = ADC_LUT[low].voltage;
    float V1 = ADC_LUT[high].voltage;
    int32_t ADC_0 = ADC_LUT[low].adc_ref;
    int32_t ADC_1 = ADC_LUT[high].adc_ref;

    float slope = (float)(ADC_1 - ADC_0) / (V1 - V0);

    float adc_interpolado = (float)ADC_0 + (V_ref - V0) * slope;

    return (int32_t)(adc_interpolado + 0.5f);
}

void parse_task(void *pvParameter)
{
    UNUSED(pvParameter);
    HAL_UART_Receive_IT(&huart1, &byteRec, 1);

    while(1)
    {
        xSemaphoreTake(semParse, portMAX_DELAY);

        sscanf((const char*)buffin, "%f", &ref);

		if (ref > REF_MAX)
		{
			ref = REF_MAX;
		}
		else if (ref < REF_MIN)
		{
			ref = REF_MIN;
		}

		ref_value = GET_REFVALUE(ref);
    }
}


