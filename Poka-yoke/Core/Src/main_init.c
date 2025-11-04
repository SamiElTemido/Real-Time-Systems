#include <main.h>
extern UART_HandleTypeDef huart1;
extern SPI_HandleTypeDef hspi1;
void System_Init(void)
{
	Clock_Init();
	UART1_Init();
	GPIO_Init();
	//SPI_Init();
}
void SPI_Init(void)
{
	__HAL_RCC_SPI1_CLK_ENABLE();

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_;
	// Ajusta CPOL/CPHA a tu periférico (aquí Modo 0)
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase    = SPI_PHASE_1EDGE;
	// Usaremos CS manual: NSS por software
	hspi1.Init.NSS = SPI_NSS_SOFT;
	// Prescaler según reloj SPI y periférico (prueba 8/16/32)
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	if (HAL_SPI_Init(&hspi1) != HAL_OK);
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
		pin.Pin = GPIO_PIN_13;
		pin.Mode = GPIO_MODE_OUTPUT_PP;
		pin.Pull = GPIO_NOPULL;
		pin.Speed = GPIO_SPEED_FREQ_LOW;
		//inicializas
		HAL_GPIO_Init(GPIOC, &pin);

		GPIO_InitTypeDef gal = {0};
		__HAL_RCC_GPIOA_CLK_ENABLE();
		gal.Pin = GPIO_PIN_0;
		gal.Mode= GPIO_MODE_OUTPUT_PP;
		gal.Pull =GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &gal);


		__HAL_RCC_GPIOA_CLK_ENABLE();
		gal.Pin = GPIO_PIN_0;
		gal.Mode= GPIO_MODE_INPUT;
		gal.Pull =GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &gal);
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

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}


void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}
