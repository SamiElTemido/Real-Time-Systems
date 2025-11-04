// SAMUEL MICHAEL GARCIA GONZALEZ
#include "main.h"
#include "PIN_Gpio_confg.h"

int main(void)
{
	unsigned int count;
	unsigned int *gpio_ptr= (unsigned int *)GPIOC_ADRR;
	//unsigned int *gpio_ptrA = (unsigned int *)GPIOA_ADRR;

	GPIO_Config_t led;
	    led.gpioX = 2;         // 2 = GPIOC
	    led.pinX = 13;         // PC13
	    led.mode = 1;          // 1 = salida
	    led.otype = 0;         // push-pull
	    led.speed = 0;         // low speed
	    led.pupdr = 0;         // no pull-up/pull-down
	    led.alt_func = 0;      // no función alternativa
	    GPIO_CONFIG(led);
	while(1)
	{
		*(gpio_ptr + GPIO_ODR) = *(gpio_ptr + GPIO_ODR)  | (0x01<<13);
		for(count=0;count <200000;count++);
		*(gpio_ptr + GPIO_ODR) = *(gpio_ptr + GPIO_ODR)  & ~(0x01<<13); //parpadeo
		for(count=0;count <200000;count++);
	}
	return 0;
}
