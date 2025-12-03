#include <main.h>

void System_Init(void){
	//Clock_Init();
	GPIO_Init();

}
void Clock_Init(void)
{
RCC_OscInitTypeDef osc_config = {0};
RCC_ClkInitTypeDef clk_config = {0};

// 1. Oscilador externo HSE = 8 MHz y PLL x9 = 72 MHz
osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
osc_config.HSEState       = RCC_HSE_ON;
osc_config.HSEPredivValue = RCC_HSE_PREDIV_DIV1;   // 8 MHz / 1
osc_config.HSIState       = RCC_HSI_ON;            // opcional, suele quedar ON
osc_config.PLL.PLLState   = RCC_PLL_ON;
osc_config.PLL.PLLSource  = RCC_PLLSOURCE_HSE;     // PLL entra 8 MHz
osc_config.PLL.PLLMUL     = RCC_PLL_MUL9;          // 8 MHz x 9 = 72 MHz

if (HAL_RCC_OscConfig(&osc_config) != HAL_OK)
{
    Error_Handler();
}

// 2. Árbol de relojes: HCLK = 72 MHz, PCLK1 = 36 MHz, PCLK2 = 72 MHz
clk_config.ClockType = RCC_CLOCKTYPE_SYSCLK |
                       RCC_CLOCKTYPE_HCLK   |
                       RCC_CLOCKTYPE_PCLK1  |
                       RCC_CLOCKTYPE_PCLK2;

clk_config.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK; // SYSCLK = 72 MHz
clk_config.AHBCLKDivider  = RCC_SYSCLK_DIV1;         // HCLK = 72 MHz
clk_config.APB1CLKDivider = RCC_HCLK_DIV2;           // PCLK1 = 36 MHz (máx. 36)
clk_config.APB2CLKDivider = RCC_HCLK_DIV1;           // PCLK2 = 72 MHz

if (HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_2) != HAL_OK)
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
		pin.Pin = GPIO_PIN_13;
		pin.Mode = GPIO_MODE_OUTPUT_PP;
		pin.Pull = GPIO_NOPULL;
		pin.Speed = GPIO_SPEED_FREQ_LOW;
		//inicializas
		HAL_GPIO_Init(GPIOC, &pin);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);


}
//void SysTick_Handler(void)
//{
//  HAL_IncTick(); // Incrementa la variable 'uwTick' usada por HAL_Delay()
//
//  // Si tienes otras cosas que necesitan ejecutarse cada milisegundo (por ejemplo, el planificador RTOS)
//  // Normalmente aquí se pondría: osSystickHandler();
//}

void Error_Handler(void)
{
    while (1)
    {
    	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    	        HAL_Delay(500);
    }
}
