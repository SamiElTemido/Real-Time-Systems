#ifndef __MAIN_H
#define __MAIN_H

#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>
#include <queue.h>

void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);
uint16_t computeDeadTime(uint16_t dead_time);

/*Free RTOS task*/
void control_task(void*);
void debug_task(void*);
void parse_task(void*);

void blinkFunction(TimerHandle_t );

void Error_Handler(void);

void TIM1_Init(void);
void ADC1_Init(void);


#endif /* __MAIN_H */
