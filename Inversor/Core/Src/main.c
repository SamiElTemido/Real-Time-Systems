#include "main.h"
TIM_HandleTypeDef htim1;
UART_HandleTypeDef huart1;
ADC_HandleTypeDef hadc1;

TimerHandle_t xTimer;
QueueHandle_t xqueue;
SemaphoreHandle_t semParse;
SemaphoreHandle_t semSample;


 float ref=12;
 int32_t ref_value=1830;
 int32_t error = 0;
 int32_t adc_value=0;
 int32_t DutyCycle=1000;

 float Kp= 0.25;
 float ki=6.0;
 float Kd=0.01;

 uint8_t idx,byteRec,buffin[64];

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
     { 6.0, 950  },
     { 7.0, 1125 },
     { 8.0, 1330 },
     { 9.0, 1518 },
     { 10.0, 1680 },
     { 11.0, 1820 },
     { 12.0, 1970 },
     { 13.0, 2110 },
     { 14.0, 2212 },
     { 15.0, 2295 },
     { 16.0, 2445 },
     { 17.0, 2595 },
     { 18.0, 2745 },
	 { 19.0, 2900}
 };

int main(void)
{

	System_Init();
	xqueue = xQueueCreate(64,sizeof(char));
	semParse=xSemaphoreCreateBinary();
	semSample=xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(50), pdTRUE, NULL, blinkFunction);
	xTimerStart(xTimer, 0);
	xTaskCreate(control_task,"Control",configMINIMAL_STACK_SIZE*4, NULL, 4, NULL);
	//xTaskCreate(sample_task, "Sample", configMINIMAL_STACK_SIZE*2, NULL, 0, NULL);
	//xTaskCreate(parse_task, "Parse", configMINIMAL_STACK_SIZE*2, NULL, 1, NULL);
	vTaskStartScheduler();
	return 0;
}
void control_task(void *pvParameter)
{
    UNUSED(pvParameter);

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1); // 1ms

    xLastWakeTime = xTaskGetTickCount();

    // Bucle principal: se ejecuta CADA 1ms
    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency); // DELAY DE 1ms

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
	BaseType_t xHigerPriorityTaskWoken = pdFALSE;
	if(byteRec== '\n')
	{
		/*The whole string is received*/
		buffin[idx]='\0';
		idx=0;
		xSemaphoreGiveFromISR(semParse,&xHigerPriorityTaskWoken);
	}
	else
	{
		buffin[idx++]=byteRec;
	}
	HAL_UART_Receive_IT(&huart1, &byteRec, 1);
	if(xHigerPriorityTaskWoken==pdTRUE)
	{
		portYIELD_FROM_ISR(xHigerPriorityTaskWoken);
	}

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

    // ------------------------------------

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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint8_t estado = 0; // 4 estados (0, 1, 2, 3)

    if (htim->Instance == TIM1)
    {
        switch (estado)
        {
            // Secuencia: PA8=0, PA7=1, PA9=0, PB14=1
            case 0          :
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

                estado = 1;
                break;

            // Secuencia: PA8=1, PA7=0, PA9=0, PB14=1
            case 1:
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

                estado = 2;
                break;

            // Secuencia: PA8=0, PA7=1, PA9=0, PB14=1 <-- ¡AQUÍ ESTÁ EL CAMBIO!
            case 2:
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
                // Antes era SET, ahora es RESET para ser el negado de PB14
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET); // <--- CAMBIO
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);

                estado = 3;
                break;

            // Secuencia: PA8=0, PA7=1, PA9=1, PB14=0
            case 3:
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);

                estado = 0; // Reinicia el ciclo
                break;
        }
    }
}
