#include "main.h"

int main(void)
{
    HAL_Init();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef pin = {0};
    pin.Pin = GPIO_PIN_13;
    pin.Mode = GPIO_MODE_OUTPUT_PP;
    pin.Pull = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &pin);

    GPIO_InitTypeDef btn = {0};
    btn.Pin = GPIO_PIN_0;
    btn.Mode = GPIO_MODE_INPUT;
    btn.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &btn);

    while (1)
    {
        uint8_t state = !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, state);
        // Si tu LED es activo en bajo, invierte: !state
    }
}
