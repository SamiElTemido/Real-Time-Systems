#include "main.h"

int main(void)
{
    HAL_Init();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef pin = {0}; // LED
    pin.Pin = GPIO_PIN_13;
    pin.Mode = GPIO_MODE_OUTPUT_OD; // push-pull
    pin.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &pin);

    GPIO_InitTypeDef btn = {0}; // botón
    btn.Pin = GPIO_PIN_0;
    btn.Mode = GPIO_MODE_IT_FALLING;
    btn.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &btn);
    HAL_NVIC_SetPriority(EXTI0_IRQn,15,0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    while (1)
    {

    }
}
void EXTI0_IRQHandler(void)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}
