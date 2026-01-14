 #include <main.h>
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;
void System_Init(void){
	HAL_Init();
	Clock_Init();
	UART1_Init();
	GPIO_Init();
	TIM1_Init();
	ADC1_Init();

}
void Clock_Init(void)
{
    RCC_OscInitTypeDef oscConfig = {0};
    RCC_ClkInitTypeDef clkConfig = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0}; // <--- Estructura nueva

    // 1. Configurar Oscilador y PLL (72 MHz)
    oscConfig.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    oscConfig.HSEState = RCC_HSE_ON;
    oscConfig.PLL.PLLState = RCC_PLL_ON;
    oscConfig.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    oscConfig.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&oscConfig) != HAL_OK)
        Error_Handler();

    // 2. Configurar Buses (AHB, APB1, APB2)
    clkConfig.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clkConfig.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                          RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clkConfig.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clkConfig.APB1CLKDivider = RCC_HCLK_DIV2;
    clkConfig.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clkConfig, FLASH_LATENCY_2) != HAL_OK)
        Error_Handler();

    // 3. CONFIGURACIÓN CRÍTICA DEL ADC  <--- ESTO ES LO QUE FALTABA
    // PCLK2 está en 72MHz. Necesitamos < 14 MHz.
    // Divisor 6: 72MHz / 6 = 12 MHz (Perfecto)
    // Divisor 8: 72MHz / 8 = 9 MHz (También seguro)
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}


void GPIO_Init(void){
	//habilitas reloj
		__HAL_RCC_GPIOC_CLK_ENABLE();
		//declaras manejador
		GPIO_InitTypeDef pin = {0};
		//configuras ese pedo
		pin.Pin = GPIO_PIN_13;			//led
		pin.Mode = GPIO_MODE_OUTPUT_PP;
		pin.Pull = GPIO_NOPULL;
		pin.Speed = GPIO_SPEED_FREQ_LOW;
		//inicializas
		HAL_GPIO_Init(GPIOC, &pin);
}

void UART1_Init(void)
{
	/* 1. Enable the peripheral clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();

	/* 2. Configure the peripheral pinout */
	GPIO_InitTypeDef uartPin = {0};
	uartPin.Pin = GPIO_PIN_9;
	uartPin.Mode = GPIO_MODE_AF_PP;
	uartPin.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &uartPin);

	uartPin.Pin = GPIO_PIN_10;
	uartPin.Mode = GPIO_MODE_AF_INPUT;
	HAL_GPIO_Init(GPIOA, &uartPin);

	/* 3. Enable the peripheral interrupt */
	HAL_NVIC_SetPriority(USART1_IRQn, 15, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	/* 4. Configure the high-level initialization */
	huart1.Instance = USART1;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.BaudRate = 115200;
	huart1.Init.StopBits = UART_STOPBITS_1;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
}
void TIM1_Init(void)
{
    /* Enable clock of peripheral */
	__HAL_RCC_TIM1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();  // <--- ¡IMPORTANTE!

	/* Aplicar Full Remap (TIM1_REMAP = 11) para usar PA8 y PA7 */
	__HAL_AFIO_REMAP_TIM1_PARTIAL();
    /* =======================
       CONFIG GPIO PA7 - PA8
       ======================= */
    GPIO_InitTypeDef oc_pin = {0};

    oc_pin.Pin = GPIO_PIN_7 | GPIO_PIN_8;   // PA7 = CH1N, PA8 = CH1
    oc_pin.Mode = GPIO_MODE_AF_PP;
    oc_pin.Pull = GPIO_NOPULL;              // No pull-up
    oc_pin.Speed = GPIO_SPEED_FREQ_HIGH;    // Recomendado
    HAL_GPIO_Init(GPIOA, &oc_pin);


    /* =======================
       CONFIG TIM1
       ======================= */
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;               // PSC = 0
    htim1.Init.Period = 1439;               // ARR = 1439 → PWM = 50 kHz
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;

    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
        Error_Handler();


    /* =======================
       CONFIG CH1 + CH1N
       ======================= */
    TIM_OC_InitTypeDef sConfigOC = {0};

    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = 720;          // ≈ 50% duty (la mitad de 1440)
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();


    /* =======================
       DEAD-TIME + BREAK
       ======================= */
    TIM_BreakDeadTimeConfigTypeDef dt_config = {0};

    dt_config.OffStateRunMode  = TIM_OSSR_DISABLE;
    dt_config.OffStateIDLEMode = TIM_OSSI_DISABLE;
    dt_config.LockLevel        = TIM_LOCKLEVEL_OFF;
    dt_config.DeadTime         = 25;             // Ajustable
    dt_config.BreakState       = TIM_BREAK_DISABLE;
    dt_config.BreakPolarity    = TIM_BREAKPOLARITY_HIGH;
    dt_config.AutomaticOutput  = TIM_AUTOMATICOUTPUT_ENABLE;

    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &dt_config) != HAL_OK)
        Error_Handler();


    /* =======================
       INICIAR PWM
       ======================= */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    __HAL_TIM_MOE_ENABLE(&htim1);   // Asegura que MOE = 1
}

void ADC1_Init(void)
{
    /* 1. Relojes */
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 2. Configurar PA0 como entrada analógica (ADC1_IN0) */
    GPIO_InitTypeDef adc_pin = {0};
    adc_pin.Pin = GPIO_PIN_0;
    adc_pin.Mode = GPIO_MODE_ANALOG;
    adc_pin.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &adc_pin);

    /* 3. Inicializar ADC */
    hadc1.Instance = ADC1;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    //hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;  // Solo 1 canal

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
        Error_Handler();

    /* 4. Configurar canal */
    ADC_ChannelConfTypeDef adc_chan = {0};
    adc_chan.Channel = ADC_CHANNEL_0;
    adc_chan.Rank = 1;
    adc_chan.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    if (HAL_ADC_ConfigChannel(&hadc1, &adc_chan) != HAL_OK)
        Error_Handler();

    /* 5. Esperar tSTAB ~2us */
    //HAL_Delay(1);

    /* 6. Calibración del ADC (muy importante en F1) */
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
        Error_Handler();

    /* 7. Arrancar conversión continua */
    if (HAL_ADC_Start(&hadc1) != HAL_OK)
        Error_Handler();
}



void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

