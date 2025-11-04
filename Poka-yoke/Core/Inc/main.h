#ifndef __MAIN_H
#define __MAIN_H

#include <string.h>
#include "stm32f4xx_hal.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <HX711.h>

void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);
void SPI_Init(void);
void Error_Handler();


#define HX711_DOUT_GPIO_Port   GPIOC
#define HX711_DOUT_Pin         GPIO_PIN_12
#define HX711_SCK_GPIO_Port    GPIOC
#define HX711_SCK_Pin          GPIO_PIN_10
void hx711_task(void *pvParameters);

void read_task(void*);
void blinkFunction(TimerHandle_t );
void debug_task(void*);


#endif /* __MAIN_H */
