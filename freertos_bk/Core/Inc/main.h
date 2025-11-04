#ifndef __MAIN_H
#define __MAIN_H

#include <string.h>
#include "stm32f4xx_hal.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>


void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);
void MCO_OutputInit(void);
void TIM4_TestClock();

#define DIR_PIN GPIO_PIN_7
#define DIR_GPIO GPIOB
/*Free RTOS task*/
void control_task(void*);

void blinkFunction(TimerHandle_t );

void Error_Handler(void);
void TIM4_Init(void);
void TIM2_Init(void);

#define EMAX 1000
#define UMAX 999

#endif /* __MAIN_H */
