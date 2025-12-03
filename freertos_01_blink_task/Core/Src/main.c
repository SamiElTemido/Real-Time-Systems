#include "main.h"
UART_HandleTypeDef huart1 ={0};
void myFirstTask(void *);
void secondTask(void *);

int main(void)
{
	xTaskCreate(myFirstTask, "FirstTask", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(secondTask, "SecondTask", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	vTaskStartScheduler();
	return 0;
}

void myFirstTask(void *pvParameters)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitTypeDef pin = {0};
	pin.Pin = GPIO_PIN_13;
	pin.Mode = GPIO_MODE_OUTPUT_PP;
	pin.Pull = GPIO_NOPULL;
	pin.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &pin);

	while(1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		for(uint32_t i = 0; i < 160000; i++);
	}
}
void secondTask(void *pvParameters)
{

}
