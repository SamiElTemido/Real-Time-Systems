#include "main.h"
extern UART_HandleTypeDef huart1;

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);  // Limpia flags y dispara HAL_UART_RxCpltCallback
}
