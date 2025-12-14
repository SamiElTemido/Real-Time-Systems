#include "main.h"

// Este archivo solo contiene funciones de inicialización.
// No se definen variables globales aquí.

void System_Init(void){
    HAL_Init();
    Clock_Init();

    // Habilita el contador de ciclos DWT para retardos de precisión en microsegundos
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    UART1_Init();
    GPIO_Init();
    ADC1_Init();
    //TIM3_Init_60Hz_Sim();
    TIM2_Init();
    TIM4_Init_Delay();
    EXTI_Init_ZC_Detect();
    EXTI_Init_PB8();
}

void Clock_Init(void){
    RCC_OscInitTypeDef osc_config = {0};
    osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc_config.HSEState = RCC_HSE_ON;
    osc_config.PLL.PLLState = RCC_PLL_ON;
    osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc_config.PLL.PLLM = 25;
    osc_config.PLL.PLLN = 200;
    osc_config.PLL.PLLP = 2;
    if(HAL_RCC_OscConfig(&osc_config) != HAL_OK) Error_Handler();

    RCC_ClkInitTypeDef clk_config = { 0 };
    clk_config.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk_config.APB1CLKDivider = RCC_HCLK_DIV2;
    clk_config.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_4) != HAL_OK) Error_Handler();
}

void GPIO_Init(void){
    GPIO_InitTypeDef pin = {0};

    // Habilitar relojes de los puertos a usar
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // LED de estado (PC13)
    pin.Pin = GPIO_PIN_13;
    pin.Mode = GPIO_MODE_OUTPUT_PP;
    pin.Pull = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &pin);

    // Pines de salida para los pulsos de los SCRs (PB6, PB7)
    pin.Pin = SCR_PULSE_1_PIN | SCR_PULSE_2_PIN;
    HAL_GPIO_Init(SCR_PULSE_1_PORT, &pin);
    HAL_GPIO_WritePin(SCR_PULSE_1_PORT, SCR_PULSE_1_PIN | SCR_PULSE_2_PIN, GPIO_PIN_RESET);
}

void UART1_Init(void){
	__HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    GPIO_InitTypeDef uart_pins = {0};
    uart_pins.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
    uart_pins.Mode      = GPIO_MODE_AF_PP;
    uart_pins.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &uart_pins);

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

void ADC1_Init(void){
    __HAL_RCC_ADC1_CLK_ENABLE();
    GPIO_InitTypeDef adc_pin = {0};
    adc_pin.Pin = GPIO_PIN_3; // PA0
    adc_pin.Mode = GPIO_MODE_ANALOG;
    adc_pin.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &adc_pin);

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}

// ... (resto del archivo main_init.c) ...

/**
 * @brief Configura TIM3 para generar una onda cuadrada de 60 Hz en PB4.
 */
void TIM3_Init_60Hz_Sim(void){
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef pwm_pin = {0};
    pwm_pin.Pin = ZC_SIM_OUT_PIN; // PB4
    pwm_pin.Mode = GPIO_MODE_AF_PP;
    pwm_pin.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(ZC_SIM_OUT_PORT, &pwm_pin);

    // --- CÁLCULO CORREGIDO PARA 60 Hz ---
    // Reloj del Timer (TIM3CLK) = 100 MHz
    // Frecuencia deseada = 60 Hz
    //
    // Para tener un tick de 1µs (1 MHz), necesitamos dividir por 100.
    // Prescaler = 100 - 1 = 99
    //
    // Ahora, con un tick de 1 MHz, calculamos el periodo.
    // Period = (Frecuencia de tick / Frecuencia deseada) - 1
    // Period = (1,000,000 / 60) - 1 = 16666.66 - 1 = 16665.66 --> Usamos 16666
    //
    // Frecuencia resultante = 1,000,000 / (16666 + 1) = 59.998 Hz (¡Perfecto!)
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 99;      // <--- VALOR CORREGIDO
    htim3.Init.Period = 16666;    // <--- VALOR CORREGIDO

    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) Error_Handler();

    TIM_OC_InitTypeDef oc_channel = {0};
    oc_channel.OCMode = TIM_OCMODE_PWM1;
    oc_channel.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc_channel.Pulse = htim3.Init.Period / 2; // Mantenemos el 50% de ciclo de trabajo
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &oc_channel, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void TIM2_Init(void){
    // 1. Relojes
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // 2. Configurar PIN PA0 (Entrada de pulsos)
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;      // Pull-Up necesario para tu prueba con cable a GND
    gpio.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &gpio);

    // 3. Configuración Base del Timer
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.Period = 0xFFFFFFFF;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    // IMPORTANTE: Usamos Base_Init en lugar de Encoder_Init
    if(HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();

    // 4. Configurar MODO ESCLAVO (Aquí está la magia)
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};

    // Modo: Usar señal externa para incrementar el contador
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;

    // Fuente: Usar el pin TI1 (PA0)
    sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;

    // Polaridad: Contar cuando la señal CAIGA (toque tierra)
    sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_FALLING;

    // Filtro: 15 (Maximo) para ignorar el rebote de tu mano temblorosa
    sSlaveConfig.TriggerFilter = 15;

    if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK) {
        Error_Handler();
    }

    // 5. Iniciar
    HAL_TIM_Base_Start(&htim2);
}

void TIM4_Init_Delay(void){
    __HAL_RCC_TIM4_CLK_ENABLE();
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 49; // Tick de 1 microsegundo
    htim4.Init.Period = 10000;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK) Error_Handler();

    HAL_NVIC_SetPriority(TIM4_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

void EXTI_Init_ZC_Detect(void){
    GPIO_InitTypeDef exti_pin = {0};
    exti_pin.Pin = ZC_DETECT_IN_PIN; // PB5
    exti_pin.Mode = GPIO_MODE_IT_RISING;
    exti_pin.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ZC_DETECT_IN_PORT, &exti_pin);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}
void EXTI_Init_PB8(void){
    // 1. Habilitar reloj del puerto B (por seguridad, aunque ya se habilite en GPIO_Init)
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef exti_pin = {0};
    // 2. Configurar el Pin PB8
    exti_pin.Pin = GPIO_PIN_8;
    // Replicamos el modo: Interrupción por flanco de subida y bajada
    exti_pin.Mode = GPIO_MODE_IT_RISING;
    exti_pin.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &exti_pin);
    // 3. Configurar el controlador de interrupciones (NVIC)
    // IMPORTANTE: El Pin 8 usa la línea EXTI8, que se agrupa en EXTI9_5_IRQn
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Error_Handler(void){
    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(500);
    }
}
