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
void Error_Handler();

void display_task(void*);
void main_task(void*);
void doorTask(void*);

void blinkFunction(TimerHandle_t );

int key(int,int);


#endif /* __MAIN_H */
