#include "main.h"
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;

TimerHandle_t xTimer;

 int32_t pos;
 int32_t ref;
 float Kp=6.0;
 float ki=0.0;
 float Kd=24.0;
 float uOut=100;

int main(void)
{
	System_Init();
//
//	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
//	xTimerStart(xTimer, 0);
//	xTaskCreate(control_task,"Control",configMINIMAL_STACK_SIZE, NULL, 4, NULL);
//	vTaskStartScheduler();
	while(1) {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_6);
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(100);
	}
	return 0;
}
void control_task(void *pvParameter)
{
	int16_t pre_error=0;
	int16_t sum_error=0;
	UNUSED(pvParameter);
	TickType_t xLasWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS(1);
	xLasWakeTime = xTaskGetTickCount();

	while(1)
	{
		vTaskDelayUntil(&xLasWakeTime, xPeriod);
//		pos= __HAL_TIM_GET_COUNTER(&htim2);
//		int16_t error = ref-pos;
//		float Up= Kp*error;
//		float Ud= Kd*(error-pre_error);
//		pre_error= error;
//		sum_error+= error;
//		if(sum_error>EMAX)sum_error=EMAX;
//		if(sum_error<-EMAX)sum_error=-EMAX;
//
//		float Ui= ki*sum_error;
//		float unsat = uOut = Up+Ui+Ud;
//		if(unsat>UMAX)uOut=UMAX;
//		if(unsat<-UMAX)uOut=-UMAX;

//		if(uOut>=0){
//			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,(uint16_t)uOut);
//							//pin de direccion
//		}
//		else
//		{
//			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,(uint16_t)-uOut);
//							//pin de direccion contraria
//		}

	}
}

void blinkFunction(TimerHandle_t xTimer)
{
	UNUSED(xTimer);
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
