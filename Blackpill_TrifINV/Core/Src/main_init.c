#include "main.h"

// Declaración externa para que main.c pueda usarlas
extern TIM_HandleTypeDef htim1;

void System_Init(void){
    HAL_Init();
    Clock_Init();
    GPIO_Init();
    TIM1_Init_60Deg();
}

void Clock_Init(void)
{
    // Configuración para 100 MHz (Asumiendo cristal de 25MHz)
    RCC_OscInitTypeDef osc_config = {0};
    osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc_config.HSEState = RCC_HSE_ON;
    osc_config.PLL.PLLState = RCC_PLL_ON;
    osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc_config.PLL.PLLM = 25;
    osc_config.PLL.PLLN = 200;
    osc_config.PLL.PLLP = 2; // 200/2 = 100MHz
    osc_config.PLL.PLLQ = 4;
    if(HAL_RCC_OscConfig(&osc_config) != HAL_OK) Error_Handler();

    RCC_ClkInitTypeDef clk_config = { 0 };
    clk_config.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk_config.APB1CLKDivider = RCC_HCLK_DIV2; // 50MHz
    clk_config.APB2CLKDivider = RCC_HCLK_DIV1; // 100MHz
    if (HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_3) != HAL_OK) Error_Handler();
}

void GPIO_Init(void){
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // LED de la placa (Opcional)
    GPIO_InitTypeDef pin = {0};
    pin.Pin = GPIO_PIN_13;
    pin.Mode = GPIO_MODE_OUTPUT_PP;
    pin.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &pin);
}

void TIM1_Init_60Deg(void)
{
    // 1. Habilitar Relojes
    __HAL_RCC_TIM1_CLK_ENABLE();

    // 2. Configurar Pines PWM (PA8, PA9, PA10) y Complementarios (PB13, PB14, PB15)
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Fases Altas (A, B, C)
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Fases Bajas (A_N, B_N, C_N)
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM1; // Misma función alterna
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3. Configurar Timer Base
    // Queremos una interrupción cada 60 grados.
    // 50 Hz = 20ms periodo.
    // 60 grados = 1/6 del periodo = 3.333 ms (3333 us).
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 99;       // 100MHz / 100 = 1 MHz (1 tick = 1us)
    htim1.Init.Period = 3333;        // Cuenta hasta 3333us y se desborda
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK) Error_Handler();

    // 4. Configurar Canales PWM
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0; // Iniciamos en 0
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);

    // 5. Configurar Dead Time (Tiempo Muerto)
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 150; // Ajuste seguro (~1.5 us)
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE; // ¡IMPORTANTE!

    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) Error_Handler();

    // 6. Configurar Interrupción
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
}

void Error_Handler(void)
{
    while(1) { HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); HAL_Delay(100); }
}
