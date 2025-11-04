#include <main.h>
//extern UART_HandleTypeDef huart1;
void System_Init(void){
	Clock_Init();
	//UART1_Init();
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
		//declaras manejador
		GPIO_InitTypeDef pin = {0};
		//configuras ese pedo
		pin.Pin = GPIO_PIN_13;			//led
		pin.Mode = GPIO_MODE_OUTPUT_PP;
		pin.Pull = GPIO_NOPULL;
		pin.Speed = GPIO_SPEED_FREQ_LOW;
		//inicializas
		HAL_GPIO_Init(GPIOC, &pin);

		__HAL_RCC_GPIOA_CLK_ENABLE();
		pin.Pin = 0x0FFF;				//el a0 al a12, pero solo se usa hasta el a11 como salidas
		pin.Mode= GPIO_MODE_OUTPUT_PP;
		pin.Pull =GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &pin);

		__HAL_RCC_GPIOB_CLK_ENABLE();
		pin.Pin = 0xF0F;					// primeros 4 pines del puerto b salidas y ahora b0-b3 y b8-b11
		pin.Mode= GPIO_MODE_OUTPUT_PP;
		pin.Pull =GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &pin);
		pin.Pin = 0x0F0;					// del b4 al b7 4 pines del puerto b entrdas
		pin.Mode= GPIO_MODE_INPUT;
		pin.Pull =GPIO_PULLDOWN;
		HAL_GPIO_Init(GPIOB, &pin);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

}

void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}
