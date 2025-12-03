#include "main.h"
unsigned short int count =0;
TimerHandle_t xTimer;
int main(void)
{
	System_Init();

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
		count = (count)%10000;
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
void display_task(void *pvParameters){
	UNUSED(pvParameters);

	while(1){
	}
}
void blinkFunction(TimerHandle_t xTimer)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}



