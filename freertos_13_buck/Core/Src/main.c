#include "main.h"
#include <low_pass.h>

UART_HandleTypeDef huart1 = {0};
TIM_HandleTypeDef htim1 = {0};
ADC_HandleTypeDef hadc1 = {0};

TimerHandle_t xTimer;
QueueHandle_t xqueue;
SemaphoreHandle_t xSem;

float vout,vref=0;
float vfilt;
uint16_t duty_cycle=0;

uint8_t idx,byteRec,buffin[32];

int main(void)
{
	System_Init();
	xqueue = xQueueCreate(64,sizeof(char));
	xSem= xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
	xTaskCreate(control_task,"Control", configMINIMAL_STACK_SIZE, NULL, 4, NULL);
	xTaskCreate(parse_task,"Parse", 2*configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(debug_task, "Debug", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	vTaskStartScheduler();
	return 0;
}
void control_task(void *pvParameters)
{
	UNUSED(pvParameters);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(1);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
        vout= adc_value*0.0008056640625;
        vfilt=filter_compute(vout);
	}
}

void parse_task(void *pvParameters)
{
	UNUSED(pvParameters);

    while (1)
    {
    	xSemaphoreTake(xSem,portMAX_DELAY);

	}
}
void debug_task(void *pvParameters)
{
	UNUSED(pvParameters);
	char buffer[64];
	unsigned char buflen;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(50);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
        buflen=sprintf(buffer,"VOUT:\t%0f\r\n",vout);
		for(int j = 0; j < buflen; j++) {
			xQueueSend(xqueue, buffer + j, portMAX_DELAY);
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
		xSemaphoreGiveFromISR(xSem,&xHigerPriorityTaskWoken);
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


