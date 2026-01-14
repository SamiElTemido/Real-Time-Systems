#include "main.h"

#define RCC_ADRR 0x40023800U
#define RCC_AHB1ENR 12

#define GPIOA_ADRR 0x40020000
#define GPIOC_ADRR 0x40020800U

#define GPIO_OFFSET     0x400U  // separación entre puertos

#define GPIO_MODER 0
#define GPIO_TYPER 1
#define GPIO_PUPDR 3
#define GPIO_SPEED 2
#define GPIO_ALTFL 8
#define GPIO_ALTFH 9

#define GPIO_BSRR 6
#define GPIO_IDR 4
#define GPIO_ODR 5

typedef struct {
    uint32_t gpioX;     // 0: A, 1: B, 2: C, D: 3, E: 4, no hay f ni g, H: 6
    uint8_t pinX;       // 0-15
    uint8_t mode;      // 0: entrada, 1: salida, 2: alternativa, 3: analogico
    uint8_t otype;     // 0: push-pull, 1: open-drain
    uint8_t speed;     // 0: low, 1: medium, 2: fast, 3: very high
    uint8_t pupdr;      // 0: sin pull, 1: pull-up, 2: pull-down
    uint8_t alt_func;  //alternativa (0-15)
} GPIO_Config_t;


void GPIO_CONFIG(GPIO_Config_t config)//structura de "tipo pin" para su configuracion
{
	if (config.gpioX == 5 || config.gpioX > 6) return;

	if (config.pinX > 15) return;

	unsigned int *gpio_ptr= (unsigned int *)(GPIOA_ADRR + GPIO_OFFSET*config.gpioX); //elegir puerto como BASE PUERTO A

	unsigned int *rcc_ptr = (unsigned int *)RCC_ADRR; //rcc addr
		*(rcc_ptr + RCC_AHB1ENR) = *(rcc_ptr + RCC_AHB1ENR ) | (0X01<<config.gpioX);//para habilitar relog

	*(gpio_ptr + GPIO_MODER)= *(gpio_ptr + GPIO_MODER) & ~(0x03 << config.pinX * 2); //moder
	*(gpio_ptr + GPIO_MODER)= *(gpio_ptr + GPIO_MODER) |  (config.mode << config.pinX * 2); //moder

	*(gpio_ptr + GPIO_TYPER)= *(gpio_ptr + GPIO_TYPER) & ~(0x01 << config.pinX ); //typer
	*(gpio_ptr + GPIO_TYPER)= *(gpio_ptr + GPIO_TYPER) |  (config.otype << config.pinX); //typer

	*(gpio_ptr + GPIO_SPEED)= *(gpio_ptr + GPIO_SPEED) & ~(0x03 << config.pinX * 2); //speed
	*(gpio_ptr + GPIO_SPEED)= *(gpio_ptr + GPIO_SPEED) |  (config.speed << config.pinX * 2); //speed

	*(gpio_ptr + GPIO_PUPDR)= *(gpio_ptr + GPIO_PUPDR) & ~(0x03 << config.pinX * 2); //pull?
	*(gpio_ptr + GPIO_PUPDR)= *(gpio_ptr + GPIO_PUPDR) |  (config.pupdr << config.pinX * 2); //pull?

	if (config.pinX < 8)
	{
		*(gpio_ptr + GPIO_ALTFL) = *(gpio_ptr + GPIO_ALTFL) & ~(0xF << (config.pinX * 4));    //ALT FL
		*(gpio_ptr + GPIO_ALTFL) = *(gpio_ptr + GPIO_ALTFL) | (config.alt_func << (config.pinX * 4));//ALT FL
	}
	else
	{
		*(gpio_ptr + GPIO_ALTFH) = *(gpio_ptr + GPIO_ALTFH) & ~(0xF << ((config.pinX - 8) * 4));	//ALT FH
		*(gpio_ptr + GPIO_ALTFH) = *(gpio_ptr + GPIO_ALTFH) | (config.alt_func << ((config.pinX - 8) * 4));//ALT FH
	}
}
