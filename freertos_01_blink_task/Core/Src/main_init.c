#include <main.h>
extern UART_HandleTypeDef huart1;
void System_Init(void){

	Clock_Init();
	UART1_Init();
	GPIO_Init();

}
void Clock_Init(void){

}

void UART1_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef uart_pins = {0};
    uart_pins.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    uart_pins.Mode      = GPIO_MODE_AF_PP;
    //uart_pins.Pull      = GPIO_PULLUP;
    uart_pins.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &uart_pins);

    HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}
void GPIO_Init(void){

}

void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}
