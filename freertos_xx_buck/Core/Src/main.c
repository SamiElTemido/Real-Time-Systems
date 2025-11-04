#include "main.h"

 UART_HandleTypeDef huart1;
 TIM_HandleTypeDef htim1; // Ahora usamos TIM1
 ADC_HandleTypeDef hadc1;
 TimerHandle_t xTimer;
 int main(void)
 {
     System_Init();
     xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
     xTimerStart(xTimer,10);
     vTaskStartScheduler();
     return 0;

 }

 void blinkFunction(TimerHandle_t xTimer)
 {
 	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
 }


