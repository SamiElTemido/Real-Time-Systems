#ifndef __MAIN_H
#define __MAIN_H

#include <stm32f4xx_hal.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>
#include <semphr.h>

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "ssd1306.h"



#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//
#include "mpu_display.h"

#define LED_PIN             GPIO_PIN_13
#define LED_PORT            GPIOC

void System_Init(void);
void Clock_Init(void);
void I2C1_Init(void);
void GPIO_Init(void);
void UART1_Init(void);
void MPU_Init(void);
void MPU_Calibrate(void);

void oled_task(void*);



void sample_task(void *);
void transmit_task(void *);

void blinkFunction(TimerHandle_t);

#define MPU_ADDRESS (0x68 << 1)
#define G_SCALE 0.00006103515625f
#define RADTODEG 57.29577951308232f
#define DEGSXSEC 0.00762939453125f
#define TS 0.005

void Error_Handler(void);

#endif /* __MAIN_H */
