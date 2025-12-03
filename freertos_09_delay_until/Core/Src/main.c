#include "main.h"

unsigned short int count = 0;

UART_HandleTypeDef huart1 = {0};
TimerHandle_t xTimer;

/* Programa de ejemplo: temporizador software para parpadear LED y tarea de display con delay until*/
int main(void)
{
	System_Init();

	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);

	xTaskCreate(main_task, "Main", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(display_task, "Display", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTimerStart(xTimer, 10);
	vTaskStartScheduler();
	return 0;
}

void main_task(void *pvParameters)
{
	(void)pvParameters;

	while (1)
	{
		count = (count + 1) % 10000;
		vTaskDelay(pdMS_TO_TICKS(300));
	}
}

void display_task(void *pvParameters)
{
	(void)pvParameters;

	char buffer[32];
	unsigned char buflen = 0;

	TickType_t xLasWakeTime;
	const TickType_t xFrecuency = pdMS_TO_TICKS(10);
	xLasWakeTime = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&xLasWakeTime, xFrecuency);
		GPIOB->ODR = GPIOB->IDR | 0x01;
		buflen = sprintf(buffer, "cuenta: %i\r\n", count);
		HAL_UART_Transmit(&huart1, (uint8_t *)buffer, buflen, HAL_MAX_DELAY);
	}
}

/* Temporizador: parpadeo de LED (no bloquear dentro del callback) */
void blinkFunction(TimerHandle_t xTimer)
{
	(void)xTimer;
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

