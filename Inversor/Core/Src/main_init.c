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
void Clock_Init(void){
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
    /* 1. Habilitar reloj del TIM1 y GPIOs (A y B) */
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE(); // <--- IMPORTANTE: Habilitar GPIOB

    /* 2. Configurar pines como SALIDA DIGITAL (NO AF) */
    GPIO_InitTypeDef io = {0};
    io.Mode = GPIO_MODE_OUTPUT_PP; // <--- MODO SALIDA NORMAL
    io.Pull = GPIO_NOPULL;
    io.Speed = GPIO_SPEED_FREQ_LOW;

    // Pines que usa tu secuencia: PA7, PA8, PA9
    io.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    HAL_GPIO_Init(GPIOA, &io);

    // Pin que usa tu secuencia: PB14
    io.Pin = GPIO_PIN_14;
    HAL_GPIO_Init(GPIOB, &io); // <--- CONFIGURAR EL PIN EN GPIOB


    /* 3. Configurar TIM1 como temporizador base (interrupción) */
    htim1.Instance = TIM1;

    // Reloj de 100MHz / (999 + 1) = 100 kHz (tick de 0.01 ms)
    htim1.Init.Prescaler = 999;

    // Periodo: (415 + 1) * 0.01 ms = 4.16 ms
    // 4.16 ms * 4 pasos = 16.64 ms (aprox 60 Hz)
    htim1.Init.Period = 415;

    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim1); // <--- FUNCIÓN BASE

    /* 4. Configurar prioridad y habilitar NVIC */
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

    /* 5. Iniciar la interrupción */
    HAL_TIM_Base_Start_IT(&htim1);
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

