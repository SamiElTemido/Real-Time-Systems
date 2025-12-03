#include "main.h"
UART_HandleTypeDef huart1 ={0};
void myFirstTask(void *);
void secondTask(void *);
const char *msg1= "hola\n\r";
const char *msg2= "adios\n\r";

int main(void)
{
	System_Init();
	xTaskCreate(myFirstTask, "FirstTask", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(secondTask, "SecondTask", configMINIMAL_STACK_SIZE, (void*)msg1, 1, NULL);
	xTaskCreate(secondTask, "ThirdTask", configMINIMAL_STACK_SIZE, (void*)msg2, 0, NULL);
	vTaskStartScheduler();
	return 0;
}
void myFirstTask(void *pvParameters)
{
	while (1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}
void secondTask(void *pvParameters)
{
	char *msg = (char *)pvParameters;

	while (1)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
		for (int i = 0; i < 4000; i++);
	}
}
