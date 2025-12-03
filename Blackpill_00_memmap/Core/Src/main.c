#include "main.h"
#define RCC_ADRR 0x40023800U
#define RCC_AHB1ENR 12

#define GPIOC_ADRR 0x40020800U
#define GPIO_MODER 0
#define GPIO_TYPER 1
#define GPIO_ODR 5
#define GPIO_PULL_UP 3

#define GPIOA_ADRR 0x40020000
#define GPIO_BSRR 6

#define GPIO_IDR 4

void GPIO_CONFIG(uint32_t gpioX,uint8_t pinX,uint8_t mode,uint8_t otype,
				 uint8_t speed,uint8_t pupdr, uint8_t alt_func);

int main(void)
{
	unsigned int count;
	unsigned int *gpio_ptr = (unsigned int *)GPIOC_ADRR;

	GPIO_CONFIG(2, 13, 1, NULL, NULL, NULL, NULL);
	while(1)
	{
		*(gpio_ptr + GPIO_ODR) = *(gpio_ptr + GPIO_ODR)  | (0x01<<13);
		for(count=0;count <100000;count++);
		*(gpio_ptr + GPIO_ODR) = *(gpio_ptr + GPIO_ODR)  & ~(0x01<<13);
		for(count=0;count <100000;count++);
	}
	return 0;
}
void GPIO_CONFIG(uint32_t gpioX,  // GPIO port: 0=A, 1=B, 2=C, 3=D, 4=E, 6=H
			uint8_t pinX,		  // Pin number (0-15)
			uint8_t mode,       // 0=input, 1=output, 2=alternate, 3=analog
			uint8_t otype,      // 0=push-pull, 1=open-drain
			uint8_t speed,      // 0=low, 1=medium, 2=fast, 3=very high
			uint8_t pupdr,      // 0=no pull, 1=pull-up, 2=pull-down
			uint8_t alt_func)   // Alternate function (0-15) for alternate mode
{
	if (gpioX == 5 || gpioX > 6) return;

	unsigned int *gpio_ptr = (unsigned int *)(0x40020000 + 0x400 * gpioX);
	unsigned int *rcc_ptr = (unsigned int *)0x40023800;

	// Enable GPIO clock
	*(rcc_ptr + 12) = *(rcc_ptr + 12) | (0x01 << gpioX);

	// Configure GPIO mode
	*(gpio_ptr + 0) = *(gpio_ptr + 0) & ~(0x03 << pinX * 2);
	*(gpio_ptr + 0) = *(gpio_ptr + 0) | (mode << pinX * 2);

	// Configure GPIO output type
	*(gpio_ptr + 1) = *(gpio_ptr + 1) & ~(0x01 << pinX);
	*(gpio_ptr + 1) = *(gpio_ptr + 1) | (otype << pinX);

	// Configure GPIO speed
	*(gpio_ptr + 2) = *(gpio_ptr + 2) & ~(0x03 << pinX * 2);
	*(gpio_ptr + 2) = *(gpio_ptr + 2) | (speed << pinX * 2);

	// Configure GPIO pull-up/pull-down
	*(gpio_ptr + 3) = *(gpio_ptr + 3) & ~(0x03 << pinX * 2);
	*(gpio_ptr + 3) = *(gpio_ptr + 3) | (pupdr << pinX * 2);

	if (pinX < 8)
	{
		// Configure alternate function low register
		*(gpio_ptr + 8) &= ~(0xF << (pinX * 4));
		*(gpio_ptr + 8) |= (alt_func << (pinX * 4));
	}
	else
	{
		// Configure alternate function high register
		*(gpio_ptr + 9) &= ~(0xF << ((pinX - 8) * 4));
		*(gpio_ptr + 9) |= (alt_func << ((pinX - 8) * 4));
	}
}

