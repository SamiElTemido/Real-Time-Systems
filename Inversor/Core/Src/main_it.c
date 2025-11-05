#include "main.h"
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);  // Limpia flags y dispara HAL_UART_RxCpltCallback
}
void TIM1_UP_TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}
