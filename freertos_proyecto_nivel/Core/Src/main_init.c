#include <main.h>
extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef hi2c1;

void System_Init(void){
	HAL_Init();
	Clock_Init();
	UART1_Init();
	I2C1_Init();
	GPIO_Init();
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
	if(HAL_RCC_OscConfig(&osc_config) != HAL_OK)
	{
		Error_Handler();
	}

	RCC_ClkInitTypeDef clk_config = { 0 };
	clk_config.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk_config.APB1CLKDivider = RCC_HCLK_DIV2;
	clk_config.APB2CLKDivider = RCC_HCLK_DIV1;
	if (HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_3) != HAL_OK) {
		Error_Handler();
	}
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

void I2C1_Init(void)
{
    // GPIO PB6/PB7 AF4 Open-Drain con pull-up
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gp = {0};
    gp.Pin = GPIO_PIN_8 | GPIO_PIN_9;   // PB8=SCL, PB9=SDA
    gp.Mode = GPIO_MODE_AF_OD;
    //gp.Pull = GPIO_PULLUP;              // mejor con pull-ups externos 4.7k
    gp.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gp.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &gp);

    // Reloj I2C1
    __HAL_RCC_I2C1_CLK_ENABLE();

    // Configura hi2c1 y llama HAL_I2C_Init
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;               // prueba 100000 si hay cables largos
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }

    // Opcional: habilitar filtro analógico y configurar filtro digital si procede
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
}

void GPIO_Init(void){
    GPIO_InitTypeDef pin = {0};

	__HAL_RCC_GPIOC_CLK_ENABLE();

	pin.Pin = LED_PIN;
	pin.Mode = GPIO_MODE_OUTPUT_PP;
	pin.Pull = GPIO_NOPULL;
	pin.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_PORT, &pin);
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
    while (1)
    {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Parpadeo LENTO para indicar error
    }
}
