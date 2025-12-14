#ifndef __MAIN_H
#define __MAIN_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "stm32f1xx_hal.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <semphr.h>
#include <queue.h>

void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);
void ADC1_Init(void);
void TIM1_Init(void);

#define DIR_A_PIN   GPIO_PIN_5
#define DIR_B_PIN   GPIO_PIN_4
#define DIR_GPIO GPIOB
/*Free RTOS task*/
void control_task(void*);
void sample_task(void*);
void parse_task(void*);

void blinkFunction(TimerHandle_t );

void Error_Handler(void);
int32_t GET_REFVALUE(float V_ref);


#define EMAX 1500
#define EMIN -1500

#define CMAX 1400
#define CMIN 50
#define REF_MIN 1.0
#define REF_MAX 19.0


#define TS 0.001
#define VDIV 0.124668435
#define volttoadc 4095/3.3



#endif /* __MAIN_H */
