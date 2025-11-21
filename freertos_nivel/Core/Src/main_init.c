#include <main.h>

extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c1;

void System_Init(void)
{
	Clock_Init();
	UART1_Init();
	GPIO_Init();
	I2C1_Init();

}

void Clock_Init(void)
{
	RCC_OscInitTypeDef osc_config;
	osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc_config.HSEState = RCC_HSE_ON;
	osc_config.PLL.PLLState = RCC_PLL_ON;
	osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osc_config.PLL.PLLM = 25;
	osc_config.PLL.PLLN = 200;
	osc_config.PLL.PLLP = 2;
	if( HAL_RCC_OscConfig(&osc_config) != HAL_OK)
		Error_Handler();

	RCC_ClkInitTypeDef clk_config = {0};
	clk_config.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
			RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk_config.APB1CLKDivider = RCC_HCLK_DIV2;
	clk_config.APB2CLKDivider = RCC_HCLK_DIV1;
	if(HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_4) != HAL_OK)
		Error_Handler();
}

void UART1_Init(void)
{
   //CONFIGURACION DEL RELOJ //
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

   // CONFIGURACION DE LOS PINES DEL UART
   GPIO_InitTypeDef GPIO_UART = {0};

   GPIO_UART.Pin = GPIO_PIN_9 | GPIO_PIN_10;  // TX y RX
   GPIO_UART.Mode = GPIO_MODE_AF_PP;  // FUNCION ALTERNATIVA Push-Pull
   GPIO_UART.Alternate = GPIO_AF7_USART1;  // AF7 PARA USART1 EN PA9 y PA10
   HAL_GPIO_Init(GPIOA, &GPIO_UART);

   HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
   HAL_NVIC_EnableIRQ(USART1_IRQn);

   huart1.Instance = USART1;
   huart1.Init.BaudRate = 115200;
   huart1.Init.Mode = UART_MODE_TX_RX;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
       Error_Handler();
    }
}

void GPIO_Init(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef pin = {0};// LED PIN
	pin.Pin = GPIO_PIN_13;
	pin.Mode = GPIO_MODE_OUTPUT_OD;
	pin.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOC, &pin);


	GPIO_InitTypeDef dir_pin = {0};// DIR PIN
	dir_pin.Pin = GPIO_PIN_7;
	dir_pin.Mode = GPIO_MODE_OUTPUT_PP;
	dir_pin.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &dir_pin);

	GPIO_InitTypeDef dir_pin2 = {0};// DIR PIN2
	dir_pin2.Pin = GPIO_PIN_8;
	dir_pin2.Mode = GPIO_MODE_OUTPUT_PP;
	dir_pin2.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &dir_pin2);
}


uint16_t computeDeadTime(uint16_t dead_time)
{
	uint16_t death_time;
	float clockFreq;
	clockFreq = 1000000000.0 / HAL_RCC_GetSysClockFreq();

	if(dead_time >=0 && dead_time <= 1764)
		death_time = dead_time / clockFreq;
	else if(dead_time >= 1778 && dead_time <= 3529)
		death_time = dead_time / (clockFreq * 2) + 64;
	else if(dead_time >= 3556 && dead_time <= 7001)
		death_time = dead_time / (clockFreq * 8) + 160;
	else if(dead_time >= 7112 && dead_time <= 14001)
		death_time = dead_time / (clockFreq * 16) + 192;
	else
		death_time = dead_time;
	return death_time;
}

void I2C1_Init(void)
{
   /* 1. Habilita el reloj del perifÃ©rico */
   __HAL_RCC_I2C1_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();

   /* 2. ConfiguraciÃ³n de bajo nivel (pines) */
   GPIO_InitTypeDef i2c_pin = {0};
   i2c_pin.Pin = GPIO_PIN_8 | GPIO_PIN_9;
   i2c_pin.Mode = GPIO_MODE_AF_OD;
   i2c_pin.Alternate = GPIO_AF4_I2C1;
   //i2c_pin.Pull = GPIO_PULLUP;
   HAL_GPIO_Init(GPIOB, &i2c_pin);

   /* 3. HabilitaciÃ³n de la interrupciÃ³n */
   //HAL_NVIC_SetPriority(I2C1_EV_IRQn, 15, 0);
   //HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

   /* 4. ConfiguraciÃ³n de alto nivel */
   hi2c1.Instance = I2C1;
   hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
   hi2c1.Init.ClockSpeed = 400000;
   if (HAL_I2C_Init(&hi2c1) != HAL_OK)
      Error_Handler();

   /* 5. Arrancar el perifÃ©rico */
}


void MPU_Init(void)
{
   char buffer[8];
   buffer[0] = 0x6B;buffer[1] = 0x00;
   HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, (uint8_t *)buffer, 2, HAL_MAX_DELAY);
   vTaskDelay(1);
   buffer[0] = 0x1A;buffer[1] = 0x14;
   HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, (uint8_t *)buffer, 2, HAL_MAX_DELAY);
   vTaskDelay(1);
   buffer[0] = 0x1B;buffer[1] = 0x00;
   HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, (uint8_t *)buffer, 2, HAL_MAX_DELAY);
   vTaskDelay(1);
   buffer[0] = 0x1C;buffer[1] = 0x00;
   HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, (uint8_t *)buffer, 2, HAL_MAX_DELAY);
   vTaskDelay(1);
}


void Error_Handler(void)
{
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		HAL_Delay(100);
	}
}
