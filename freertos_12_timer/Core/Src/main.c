#include "main.h"
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;

TimerHandle_t xTimer;
QueueHandle_t xqueue;
SemaphoreHandle_t semParse;
SemaphoreHandle_t semSample;


 int32_t pos=0;
 int32_t ref=0;
 int32_t error = 0;
 float Kp=0.33*Vcons;
 float ki=0.0*Vcons;
 float Kd=0.*Vcons;
 float uOut=0.0;
 float dt=0.001;
 int32_t pos_samples[SAMPLES];
 int32_t uout_samples[SAMPLES];

 uint8_t idx,byteRec,buffin[32];
int main(void)
{
	System_Init();
	xqueue = xQueueCreate(64,sizeof(char));
	semParse=xSemaphoreCreateBinary();
	semSample=xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
	xTimerStart(xTimer, 0);
	xTaskCreate(control_task,"Control",configMINIMAL_STACK_SIZE*2, NULL, 4, NULL);
	xTaskCreate(sample_task, "Sample", configMINIMAL_STACK_SIZE*2, NULL, 0, NULL);
	xTaskCreate(parse_task, "Parse", configMINIMAL_STACK_SIZE*2, NULL, 1, NULL);
	vTaskStartScheduler();
	return 0;
}
void control_task(void *pvParameter)
{
    UNUSED(pvParameter);

    // Variables PID

    int32_t pre_error = 0;
    float i_term = 0.0f;

    const float dt = 0.001f; // 1 ms

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // 1. Leer posición del encoder (32 bits)
        pos = __HAL_TIM_GET_COUNTER(&htim2);

        // 2. Calcular error
        error = ref - pos;

        // 3. Calcular PID
        float Up = Kp * (float)error;

        i_term += (float)error * dt;
        if (i_term > EMAX) i_term = EMAX;
        if (i_term < -EMAX) i_term = -EMAX;
        float Ui = ki * i_term;

        float Ud = Kd * ((float)(error - pre_error));

        pre_error = error;

        float unsat = Up + Ui + Ud;

        // 4. Saturar salida
        if (unsat > UMAX) uOut = UMAX;
        else if (unsat < -UMAX) uOut = -UMAX;
        else uOut = unsat;


        if (uOut >= 0.0f)
        {
        	HAL_GPIO_WritePin(DIR_GPIO, DIR_A_PIN, GPIO_PIN_SET);
        	HAL_GPIO_WritePin(DIR_GPIO, DIR_B_PIN, GPIO_PIN_RESET); // Sentido A
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (uint16_t)uOut);
        }
        else
        {
        	HAL_GPIO_WritePin(DIR_GPIO, DIR_A_PIN, GPIO_PIN_RESET);
        	HAL_GPIO_WritePin(DIR_GPIO, DIR_B_PIN, GPIO_PIN_SET);// Sentido B
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, (uint16_t)(-uOut));
        }
    }
}

void sample_task(void *pvParameter)
{
    char buffer[64];
    unsigned char buflen;
    UNUSED(pvParameter);

    while(1){

        xSemaphoreTake(semSample,portMAX_DELAY);

        // 1. TOMA DE MUESTRAS (La lógica actual que usa vTaskDelay(1))
        for(int i = 0; i < SAMPLES; i++)
        {
            // Nota: Estos valores pueden no ser los valores exactos al final del ciclo de control.
            pos_samples[i] = pos;
            uout_samples[i] = (int32_t)uOut;
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        // 2. IMPRESIÓN DE ENCABEZADO (Una sola vez, formato correcto)
        buflen = sprintf(buffer,"Time(s)\tPosition\tUout\r\n");
        for(int i = 0; i < buflen; i++) {
            xQueueSend(xqueue, buffer + i, portMAX_DELAY);
        }

        // 3. IMPRESIÓN DE DATOS MUESTREADOS
        for(int i = 0; i < SAMPLES; i++)
        {
            // El tiempo (i*dt) es preciso según tu vTaskDelay(1)
            buflen=sprintf(buffer,"%0.3f\t%li\t%li\r\n", i*dt, pos_samples[i], uout_samples[i]);
            for(int j = 0; j < buflen; j++) {
                xQueueSend(xqueue, buffer + j, portMAX_DELAY);
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
void parse_task(void *pvParameter)
{
    UNUSED(pvParameter);
    HAL_UART_Receive_IT(&huart1, &byteRec, 1);

    while(1)
    {
        // Esperar hasta recibir una cadena completa (terminada en '\n')
        xSemaphoreTake(semParse, portMAX_DELAY);

        // Convertir el texto recibido a número entero (por ejemplo, "123\n" → ref = 123)
        sscanf((const char*)buffin, "%li", &ref);
        xSemaphoreGive(semSample);
    }
}

