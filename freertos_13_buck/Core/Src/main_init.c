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
	//FUNCION PARA CONFIGURAR EL RELOJ a 100MHZ
		RCC_OscInitTypeDef osc_config = {0};
		osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;//ACTIVAMOS EL OSCILADOR EXTERNO 25MHz
		osc_config.HSEState = RCC_HSE_ON;//ENCENDEMOS EL RELOJ
		osc_config.PLL.PLLState = RCC_PLL_ON;//ENCENDEMOS EL MULTIPLEXOR PLL
		osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;//SELECCIONAMOS EL INTERNO DE 16MHz
		osc_config.PLL.PLLM = 25;//Dividimos el PLL M por 25
		osc_config.PLL.PLLN = 200;//Multiplicamos por 100
		osc_config.PLL.PLLP = 2;//DIVIDIMOS POR 2
		if(HAL_RCC_OscConfig(&osc_config) != HAL_OK)
		{
			Error_Handler();
		}

		RCC_ClkInitTypeDef clk_config = { 0 }; //ESTRUCTURA PARA EL MANEJADOR DEL CLOCK
		clk_config.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
				| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2; //SELECCIONAMOS TODOS LOS TIPOS DE RELOJ
		clk_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; //SELECCIONAMOS PLLCLK DEL MULTIPLEXOR
		clk_config.AHBCLKDivider = RCC_SYSCLK_DIV1; //DIVIDIMOS ENTRE 1 EL PRESCALADOR AHB
		clk_config.APB1CLKDivider = RCC_HCLK_DIV2; //DIVIDIMOS ENTRE 2 EL PRESCALADOR APB1
		clk_config.APB2CLKDivider = RCC_HCLK_DIV1; //DIVIDIMOS ENTRE 1 EL PRESCALER APB2
		if (HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_4) != HAL_OK) {
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
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef uart_pins = {0};
    uart_pins.Pin       = GPIO_PIN_9 | GPIO_PIN_10;		//A9 y A10
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

void TIM1_Init(void)
{
    /* 1. Habilitar el reloj para el periférico */
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* 2. Configuración de bajo nivel (pines e interrupciones) */
    GPIO_InitTypeDef oc_pin = {0};
    oc_pin.Pin = GPIO_PIN_8 | GPIO_PIN_7;
    oc_pin.Mode = GPIO_MODE_AF_PP;
    oc_pin.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &oc_pin);

    /* 3. Configuración de alto nivel */
    htim1.Instance = TIM1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Prescaler = 0; /* CK_CNT = Periph_clock / (1 + Prescaler) */
    htim1.Init.Period = 999;  /* Timer_Freq = CK_CNT / (1 + Period) */
    HAL_TIM_Base_Init(&htim1);

    TIM_OC_InitTypeDef output_channel = {0};
    output_channel.OCMode = TIM_OCMODE_PWM1;
    output_channel.OCPolarity = TIM_OCPOLARITY_HIGH;
    output_channel.Pulse = 499;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &output_channel, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();

    TIM_BreakDeadTimeConfigTypeDef deadTimeConfig = {0};
    deadTimeConfig.DeadTime = computeDeadTime(100);
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &deadTimeConfig) != HAL_OK)
        Error_Handler();

    /* Arrancar el periférico */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
}

void ADC1_Init(void)
{
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef adc_pin = {0};
    adc_pin.Pin = GPIO_PIN_0;
    adc_pin.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &adc_pin);

    hadc1.Instance = ADC1;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
        Error_Handler();

    ADC_ChannelConfTypeDef adc_channel = {0};
    adc_channel.Channel = ADC_CHANNEL_0;
    adc_channel.Rank = 1;
    adc_channel.SamplingTime = ADC_SAMPLETIME_112CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &adc_channel) != HAL_OK)
        Error_Handler();

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
uint16_t computeDeadTime(uint16_t dead_time)
{
    uint16_t death_time;
    float clockFreq;
    clockFreq = 100000000.0 / HAL_RCC_GetSysClockFreq();

    if (dead_time >= 0 && dead_time <= 1764)
        death_time = dead_time / clockFreq;
    else if (dead_time >= 1778 && dead_time <= 3529)
        death_time = dead_time / (clockFreq * 2) + 64;
    else if (dead_time >= 3556 && dead_time <= 7001)
        death_time = dead_time / (clockFreq * 8) + 160;
    else if (dead_time >= 7112 && dead_time <= 14001)
        death_time = dead_time / (clockFreq * 16) + 192;
    else
        death_time = dead_time;
    return death_time;
}


