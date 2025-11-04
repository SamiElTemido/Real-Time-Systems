#include <main.h>
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim2;
void System_Init(void){
	HAL_Init();
	Clock_Init();
//	UART1_Init();
	GPIO_Init();
//	TIM4_Init();
//	TIM2_Init();
//	MCO_OutputInit();
	TIM4_TestClock();
}
void TIM4_TestClock(void)
{
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef pin = {0};
    pin.Pin = GPIO_PIN_6;
    pin.Mode = GPIO_MODE_AF_PP;
    pin.Pull = GPIO_NOPULL;
    pin.Speed = GPIO_SPEED_FREQ_HIGH;
    pin.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &pin);

    TIM_HandleTypeDef htim4 = {0};
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;       // No prescaler
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 999;    // Arbitrary, will define with compare
    HAL_TIM_Base_Init(&htim4);
    HAL_TIM_OC_Init(&htim4);

    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode = TIM_OCMODE_TOGGLE;  // Toggle output on match
    oc.Pulse = 249	;                  // Compare value (half of Period = 50%)
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    HAL_TIM_OC_ConfigChannel(&htim4, &oc, TIM_CHANNEL_1);

    HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_1);
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

		__HAL_RCC_GPIOB_CLK_ENABLE();
		pin.Pin = GPIO_PIN_6;				// PINES DE DIRECCION POR AHORA B7
		pin.Mode= GPIO_MODE_OUTPUT_PP;
		pin.Pull =GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &pin);

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

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

void TIM2_Init(void){
   /*   Enable clock of peripheral   */
   __HAL_RCC_TIM2_CLK_ENABLE();
   __HAL_RCC_GPIOA_CLK_ENABLE();
   /*   Configure function pins   */
   GPIO_InitTypeDef qei_pin = {0};
   qei_pin.Pin = GPIO_PIN_0 | GPIO_PIN_1;	//A0 y A1
   qei_pin.Mode = GPIO_MODE_AF_PP;
   qei_pin.Pull = GPIO_PULLUP;
   qei_pin.Alternate = GPIO_AF1_TIM2;
   HAL_GPIO_Init(GPIOA, &qei_pin);

   htim2.Instance = TIM2;
   htim2.Init.Prescaler = 0;
   htim2.Init.Period = 0xFFFFFFFF;

   TIM_Encoder_InitTypeDef qei_conf = {0};
   qei_conf.EncoderMode = TIM_ENCODERMODE_TI2;
   qei_conf.IC1Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
   qei_conf.IC2Polarity = TIM_ENCODERINPUTPOLARITY_RISING;
   qei_conf.IC1Selection = TIM_ICSELECTION_DIRECTTI;
   qei_conf.IC2Selection = TIM_ICSELECTION_DIRECTTI;

   if(HAL_TIM_Encoder_Init(&htim2, &qei_conf) != HAL_OK){
      Error_Handler();
   }

   HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

}

void TIM4_Init(void){
   /* Enable peripheral clock */
   __HAL_RCC_TIM4_CLK_ENABLE();
   __HAL_RCC_GPIOB_CLK_ENABLE();
   /* Configure peripheral pins */


   /* Set peripheral high level */

   // CK_PSC = Clock speed
   // CK_CNT = CK_PSC / 1 + Pre-scale
   // INT_Freq = CK_CNT / 1 + Period

   GPIO_InitTypeDef oc_pin = {0};
   oc_pin.Pin = GPIO_PIN_6;
   oc_pin.Mode = GPIO_MODE_AF_PP;
   oc_pin.Alternate = GPIO_AF2_TIM4;
   HAL_GPIO_Init(GPIOB, &oc_pin);

   htim4.Instance = TIM4;
   htim4.Init.Prescaler = 99;
   htim4.Init.Period = 999;

   HAL_TIM_PWM_Init(&htim4);

   TIM_OC_InitTypeDef oc_channel = {0};
   oc_channel.OCMode = TIM_OCMODE_PWM1;
   oc_channel.OCPolarity = TIM_OCPOLARITY_HIGH;
   oc_channel.OCNPolarity = TIM_OCPOLARITY_LOW;
   oc_channel.Pulse = 499;      //when it reaches 1000
   if(HAL_TIM_PWM_ConfigChannel(&htim4, &oc_channel, TIM_CHANNEL_1) != HAL_OK){
      Error_Handler();
   }
   /* Start peripheral */
   HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
}

void Error_Handler(void)
{
    while (1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}

