#include "main.h"
extern UART_HandleTypeDef huart1;

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);  // Limpia flags y dispara HAL_UART_RxCpltCallback
}
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(ZC_DETECT_IN_PIN);
  HAL_GPIO_EXTI_IRQHandler(ZC_DETECT_IN_PIN_2);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM4 global interrupt.
  */
void TIM4_IRQHandler(void)
{
  /* USER CODE BEGIN TIM4_IRQn 0 */

  /* USER CODE END TIM4_IRQn 0 */
  HAL_TIM_IRQHandler(&htim4);
  /* USER CODE BEGIN TIM4_IRQn 1 */

  /* USER CODE END TIM4_IRQn 1 */
}
