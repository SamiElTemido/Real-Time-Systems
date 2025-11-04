#include <main.h>

extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim1; // Ahora usamos TIM1
extern ADC_HandleTypeDef hadc1;

void System_Init(void)
{
    HAL_Init();
    Clock_Init();
    UART1_Init();
    GPIO_Init();
    TIM1_Init();
    ADC1_Init();
}

/* --------------------- RELOJ PRINCIPAL --------------------- */
void Clock_Init(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    // Ajusta estos valores según tu cristal:
    // Si tu HSE es 8 MHz:
    osc.PLL.PLLM = 8;
    osc.PLL.PLLN = 200;
    osc.PLL.PLLP = RCC_PLLP_DIV2;
    osc.PLL.PLLQ = 4;

    if (HAL_RCC_OscConfig(&osc) != HAL_OK)
        Error_Handler();

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_3) != HAL_OK)
        Error_Handler();

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

/* --------------------- GPIO --------------------- */
void GPIO_Init(void)
{
    GPIO_InitTypeDef pin = {0};

    // LED en PC13
    pin.Pin = GPIO_PIN_13;
    pin.Mode = GPIO_MODE_OUTPUT_PP;
    pin.Pull = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &pin);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

}

void UART1_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef uart_pins = {0};
    uart_pins.Pin = GPIO_PIN_9 | GPIO_PIN_10; // TX/RX
    uart_pins.Mode = GPIO_MODE_AF_PP;
    uart_pins.Pull = GPIO_NOPULL;
    uart_pins.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &uart_pins);

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.Mode = UART_MODE_TX_RX;
    if (HAL_UART_Init(&huart1) != HAL_OK)
        Error_Handler();
}

/* --------------------- PWM EN TIM2 --------------------- */
/* --------------------- PWM EN TIM1 CON COMPLEMENTARIO (PA8 y PA7) --------------------- */
void TIM1_Init(void)
{
    /* 1. Habilitar la señal del reloj del periferio */
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE(); // PA7 y PA8 están en el puerto A

    /* 2. Configurar los pines del periferico (CH1 en PA8 y CH1N en PA7) */
    GPIO_InitTypeDef OC_Pin = {0};

    // TIM1 Channel 1 (PWM): PA8
    // TIM1 Channel 1N (Complementary PWM): PA7

    // Configuración para PA7 y PA8
    OC_Pin.Pin = GPIO_PIN_7 | GPIO_PIN_8;
    OC_Pin.Mode = GPIO_MODE_AF_PP;
    OC_Pin.Alternate = GPIO_AF1_TIM1; // AF1 para ambos en este caso
    OC_Pin.Pull = GPIO_NOPULL;
    OC_Pin.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &OC_Pin);

    /* 3. Dar de alta una interrupcion (opcional) */

    /* 4. Configuracion de alto nivel */
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 1;
    htim1.Init.Period = 499; // Periodo de PWM (frecuencia ~100kHz si CLK_APB2=100MHz)
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0; // Obligatorio para TIM1/TIM8

    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
        Error_Handler();

    TIM_OC_InitTypeDef Oc_Config = {0};
    Oc_Config.OCMode = TIM_OCMODE_PWM1;
    Oc_Config.Pulse = 150; // Inicialmente 0 (Duty Cycle 0%)
    Oc_Config.OCPolarity = TIM_OCPOLARITY_HIGH;
    Oc_Config.OCNPolarity = TIM_OCNPOLARITY_HIGH; // Polaridad del canal complementario
    Oc_Config.OCFastMode = TIM_OCFAST_DISABLE;
    Oc_Config.OCIdleState = TIM_OCIDLESTATE_RESET; // Estado en IDLE (Estándar)
    Oc_Config.OCNIdleState = TIM_OCNIDLESTATE_RESET; // Estado en IDLE (Complementario)

    if (HAL_TIM_PWM_ConfigChannel(&htim1, &Oc_Config, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();

    // Configuración del Dead Time y Salida Principal
    TIM_BreakDeadTimeConfigTypeDef Dt_Config = {0};
    Dt_Config.DeadTime = 10; // Ejemplo de 10 cuentas de Dead Time (ajustar según frecuencia)
    Dt_Config.BreakState = TIM_BREAK_DISABLE;
    Dt_Config.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    Dt_Config.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE; // Habilitar la salida principal

    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &Dt_Config) != HAL_OK)
        Error_Handler();

    /* 5. Arrancar el periferico */
    // Iniciar el PWM estándar (PA8)
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    // Iniciar el PWM complementario (PA7)
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}


void ADC1_Init(void)
{
	/*1. Habilitar la señal del reloj del periferio*/
	__HAL_RCC_ADC1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*2. Cponfigurar los pines del periferico*/
	GPIO_InitTypeDef adc_pin = {0};
	adc_pin.Pin = GPIO_PIN_0;
	adc_pin.Mode = GPIO_MODE_ANALOG;
	HAL_GPIO_Init(GPIOA, &adc_pin);

	/*3. Dar de alta una interrupcion*/

	/*4. Configuracion de alto nivel*/
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
	hadc1.Init.ExternalTrigConv =  ADC_SOFTWARE_START;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = ENABLE;

	if(HAL_ADC_Init(&hadc1) != HAL_OK)
		Error_Handler();

	ADC_ChannelConfTypeDef adc_chan = {0};
	adc_chan.Channel = 0;
	adc_chan.Rank = 1;
	adc_chan.SamplingTime = ADC_SAMPLETIME_56CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &adc_chan);
	HAL_ADC_Start(&hadc1);

}

void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}
