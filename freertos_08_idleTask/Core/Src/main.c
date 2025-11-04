//Samuel Michael Garcia Gonzalez
#include "main.h"

unsigned short int count =0;

UART_HandleTypeDef huart1={0};

TimerHandle_t xTimer;
QueueHandle_t xqueue;

int main(void)
{
	System_Init();

	xqueue = xQueueCreate(16,sizeof(char));
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);

	xTaskCreate(main_task, "Main", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(display_task, "Display", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTimerStart(xTimer,10);
	vTaskStartScheduler();
	return 0;
}

void main_task(void *pvParameters){
	UNUSED(pvParameters);

	while(1){
		count = (count+1)%10000;
		vTaskDelay(pdMS_TO_TICKS(300));
	}
}

void display_task(void *pvParameters){
	UNUSED(pvParameters);

	char buffer[32];
	unsigned char buflen;

	while(1){

		buflen = sprintf(buffer,"cuenta:	%i\r\n",count);
		for(int i =0;i< buflen; i++)
			xQueueSend(xqueue,buffer+i,portMAX_DELAY);
		vTaskDelay(pdMS_TO_TICKS(1));

	}
}
void blinkFunction(TimerHandle_t xTimer)	//timers no deben bloquear al procesador //no ciclos infinitos
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void vApplicationIdleHook(void){

	unsigned char byteToSend;

	if(xQueueReceive(xqueue, &byteToSend,0) == pdPASS)
	{
		HAL_UART_Transmit(&huart1, &byteToSend, 1, HAL_MAX_DELAY);
	}
}


