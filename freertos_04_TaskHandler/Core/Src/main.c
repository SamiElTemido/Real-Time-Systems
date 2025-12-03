#include "main.h"
UART_HandleTypeDef huart1 = {0};
TaskHandle_t task1;
TaskHandle_t task2;
void myFirstTask(void *);
void secondTask(void *);
void thirdTask(void *);
const char *msg1 = "hola\n\r";
const char *msg2 = "adios\n\r";
int main(void)
{
	System_Init();
	/* Crear tareas:
	 * - myFirstTask: hace toggle en el LED y coordina entre task1 y task2
	 * - secondTask / thirdTask: envían mensajes por UART y ajustan prioridades
	 */
	xTaskCreate(myFirstTask, "FirstTask", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(secondTask, "SecondTask", configMINIMAL_STACK_SIZE, (void *)msg1, 1, &task1);
	xTaskCreate(thirdTask, "ThirdTask", configMINIMAL_STACK_SIZE, (void *)msg2, 0, &task2);
	vTaskStartScheduler();
	return 0;
}

/* Alterna el LED y cambia qué tarea de mensaje está activa mediante suspend/resume */
void myFirstTask(void *pvParameters)
{
	(void)pvParameters;
	uint8_t flag = 0;
	while (1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		for (int i = 0; i < 160000; i++);

		if (!flag)
		{
			vTaskSuspend(task1);
			vTaskResume(task2);
		}
		else
		{
			vTaskSuspend(task2);
			vTaskResume(task1);
		}
		flag = !flag;
	}
}
/* Envía msg1 continuamente y ajusta prioridades para favorecer a task2 */
void secondTask(void *pvParameters)
{
	char *msg = (char *)pvParameters;
	while (1)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
		for (int i = 0; i < 4000; i++);
		vTaskPrioritySet(task2, 1);
		vTaskPrioritySet(task1, 0);
	}
}

/* Envía msg2 continuamente y ajusta prioridades para favorecer a task1 */
void thirdTask(void *pvParameters)
{
	char *msg = (char *)pvParameters;
	while (1)
	{
		HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
		for (int i = 0; i < 4000; i++);
		vTaskPrioritySet(task1, 1);
		vTaskPrioritySet(task2, 0);
	}
}