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
//	unsigned int *rcc_ptr = (unsigned int *)RCC_ADRR;
//	*(rcc_ptr + RCC_AHB1ENR) = *(rcc_ptr + RCC_AHB1ENR ) | (0X01<<2);
//	*(rcc_ptr + RCC_AHB1ENR) = *(rcc_ptr + RCC_AHB1ENR ) | (0X01<<0);
//
	unsigned int *gpio_ptr= (unsigned int *)GPIOC_ADRR;
//	*(gpio_ptr + GPIO_MODER) = *(gpio_ptr + GPIO_MODER) & ~(0x03<<26);
//	*(gpio_ptr + GPIO_MODER) = *(gpio_ptr + GPIO_MODER) | (0x01<<26);
//	*(gpio_ptr + GPIO_TYPER) = *(gpio_ptr + GPIO_TYPER)  & ~(0x03<<13);
//	*(gpio_ptr + GPIO_TYPER) = *(gpio_ptr + GPIO_TYPER)  | (0x01<<13);
//
//
//	unsigned int *gpio_ptrA = (unsigned int *)GPIOA_ADRR;
//	*(gpio_ptrA + GPIO_MODER) = *(gpio_ptrA + GPIO_MODER) & ~(0x03);
//	//*(gpio_ptrA + GPIO_MODER) = *(gpio_ptrA + GPIO_MODER) | (0x00);
//
//	*(gpio_ptrA + GPIO_PULL_UP) = *(gpio_ptrA + GPIO_PULL_UP) & ~(0x03);
//	*(gpio_ptrA + GPIO_PULL_UP) = *(gpio_ptrA + GPIO_PULL_UP) | (0x01);
//
//
//	//*(gpio_ptr + GPIO_ODR) = *(gpio_ptr + GPIO_ODR)  | (0x00<<13);
//	//		for(count=0;count <100000;count++);
//	uint8_t preState=1;
//	//uint8_t led_on = 0;
	GPIO_CONFIG(2,13,1,NULL, NULL, NULL, NULL);
	while(1)
	{
		*(gpio_ptr + GPIO_ODR) = *(gpio_ptr + GPIO_ODR)  | (0x01<<13);
		for(count=0;count <100000;count++);
		*(gpio_ptr + GPIO_ODR) = *(gpio_ptr + GPIO_ODR)  & ~(0x01<<13);
		for(count=0;count <100000;count++);
	}
	return 0;
}
void GPIO_CONFIG(uint32_t gpioX,// 0: A, 1: B, 2: C, 3: D, 4: E, 6:H
			uint8_t pinX,		// 0-15
			uint8_t mode,       // 0: entrada, 1: salida, 2: alterno, 3: análogo
			uint8_t otype,      // 0: push-pull, 1: open-drain
			uint8_t speed,      // 0: low, 1: medium, 2: fast, 3: very high
			uint8_t pupdr,       // 0: sin pull, 1: pull-up, 2: pull-down
			uint8_t alt_func) // alternativa (0-15), para modo alterno
{
	if (gpioX == 5 || gpioX > 6) return;

	unsigned int *gpio_ptr= (unsigned int *)(0x40020000 + 0x400*gpioX); //elegir puerto.  DE BASE PUERTO A

	unsigned int *rcc_ptr = (unsigned int *)0x40023800; //rcc addr
		*(rcc_ptr + 12) = *(rcc_ptr + 12 ) | (0X01<<gpioX);

	*(gpio_ptr + 0)= *(gpio_ptr + 0) & ~(0x03 << pinX * 2); //moder
	*(gpio_ptr + 0)= *(gpio_ptr + 0) |  (mode << pinX * 2); //moder

	*(gpio_ptr + 1)= *(gpio_ptr + 1) & ~(0x01 << pinX ); //typer
	*(gpio_ptr + 1)= *(gpio_ptr + 1) |  (otype << pinX); //typer

	*(gpio_ptr + 2)= *(gpio_ptr + 2) & ~(0x03 << pinX * 2); //speed
	*(gpio_ptr + 2)= *(gpio_ptr + 2) |  (speed << pinX * 2); //speed

	*(gpio_ptr + 3)= *(gpio_ptr + 3) & ~(0x03 << pinX * 2); //pull?
	*(gpio_ptr + 3)= *(gpio_ptr + 3) |  (pupdr << pinX * 2); //pull?

	if (pinX < 8)
	{
		*(gpio_ptr + 8) &= ~(0xF << (pinX * 4));    //ALT FL
		*(gpio_ptr + 8) |=  (alt_func << (pinX * 4));
	}
	else
	{
		*(gpio_ptr + 9) &= ~(0xF << ((pinX - 8) * 4));	//ALT FH
		*(gpio_ptr + 9) |=  (alt_func << ((pinX - 8) * 4));
	}
}

