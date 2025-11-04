#include <main.h>
extern UART_HandleTypeDef huart1;

void System_Init(void){
	Clock_Init();
	UART1_Init();
	GPIO_Init();

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
		__HAL_RCC_GPIOB_CLK_ENABLE();
		//declaras manejador
		GPIO_InitTypeDef pin = {0};
		//configuras ese pedo
		pin.Pin = GPIO_PIN_13;
		pin.Mode = GPIO_MODE_OUTPUT_PP;
		pin.Pull = GPIO_NOPULL;
		pin.Speed = GPIO_SPEED_FREQ_LOW;
		//inicializas
		HAL_GPIO_Init(GPIOC, &pin);

		pin.Pin = GPIO_PIN_0;
		pin.Mode= GPIO_MODE_OUTPUT_PP;
		pin.Pull =GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &pin);

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
    huart1.Init.Mode         = UART_MODE_TX;
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
