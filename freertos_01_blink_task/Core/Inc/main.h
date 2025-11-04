#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"
#include <FreeRTOS.h>
#include <task.h>

void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);

#endif /* __MAIN_H */
