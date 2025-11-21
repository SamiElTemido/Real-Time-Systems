#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stm32f4xx_hal.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>
#include <semphr.h>


void System_Init(void);
void Clock_Init(void);
void I2C1_Init(void);
void GPIO_Init(void);

void UART1_Init(void);
void MPU_Init(void);
void MPU_Calibrate(void);
void Error_Handler(void);

void TIM1_Init(void);
void ADC1_Init(void);

uint16_t computeDeadTime(uint16_t dead_time);

void blinkFunction(TimerHandle_t);

//FreeRTOS Tasks
void sample_task(void *);

void transmit_task(void *);
#define MPU_ADDRESS (0x68 << 1)
#define G_SCALE 0.00006103515625f
#define RADTODEG 57.29577951308232f
#define DEGSXSEC 0.00762939453125f
#define TS 0.001

#endif /* __MAIN_H */

