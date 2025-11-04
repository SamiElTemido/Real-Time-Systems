#include "main.h"
void myFirstTask(void *);
int main(void)
{
	xTaskCreate(myFirstTask, "FirstTask", configMINIMAL_STACK_SIZE, NULL, 0, NULL);\
	vTaskStartScheduler();
	return 0;
}

void myFirstTask(void *pvParameters){

	while(1)
	{

	}
}
