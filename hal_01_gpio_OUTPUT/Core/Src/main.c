// SAMUEL MICHAEL GARCIA GONZALEZ
#include "main.h"
int main(void)
{
	uint8_t pre_state =1;
	// 1.- habilitar el reloj del periferico.
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	//2.- configuracion del periferico.
	GPIO_InitTypeDef pin = {0};
	pin.Pin = GPIO_PIN_13;
	pin.Mode = GPIO_MODE_OUTPUT_OD;
	pin.Pull = GPIO_PULLUP;
	//3.- inicializar el periferico

	HAL_GPIO_Init(GPIOC,&pin);
	GPIO_InitTypeDef btn = {0};
	btn.Pin = GPIO_PIN_0;
	btn.Mode =GPIO_MODE_INPUT;
	btn.Mode = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA,&btn);

	while(1)
	{
		uint8_t state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) ;

		if(state==0 && pre_state ==1){

			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

			HAL_Delay(100);
		}
		for(uint32_t i=0; i<500000;i++);
	}
	return 0;
}
