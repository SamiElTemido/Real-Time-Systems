
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif



#include <string.h>
#include "stm32f1xx_hal.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);
void Error_Handler();

void display_task(void*);
void main_task(void*);
//void blinkFunction(TimerHandle_t);


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
