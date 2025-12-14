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
#include <stdbool.h>

#define TS 0.1
#define PPR 6
#define CPR 1/PPR
#define C1 60/CPR

/* === Declaración de Manejadores de Periféricos (extern) === */
// Declara que estas variables existen en algún lugar del proyecto.
// Cualquier archivo .c que incluya main.h sabrá de ellas.
extern UART_HandleTypeDef huart1;
extern ADC_HandleTypeDef  hadc1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef  htim3;
extern TIM_HandleTypeDef  htim4;

/* === Definiciones de Pines === */
#define ZC_SIM_OUT_PIN      GPIO_PIN_4  // PB4: Salida del simulador 60 Hz (TIM3_CH1)
#define ZC_SIM_OUT_PORT     GPIOB

#define ZC_DETECT_IN_PIN    GPIO_PIN_5  // PB5: Entrada para la interrupción de cruce por cero (EXTI5)
#define ZC_DETECT_IN_PORT   GPIOB

#define ZC_DETECT_IN_PIN_2  GPIO_PIN_8  // PB8: Segunda Entrada de cruce por cero (EXTI8)
#define ZC_DETECT_IN_PORT_2 GPIOB

#define SCR_PULSE_1_PIN     GPIO_PIN_6  // PB6: Pulso para el primer SCR
#define SCR_PULSE_1_PORT    GPIOB
#define SCR_PULSE_2_PIN     GPIO_PIN_7  // PB7: Pulso para el segundo SCR
#define SCR_PULSE_2_PORT    GPIOB

/* === Prototipos de Funciones de Inicialización === */
void System_Init(void);
void Clock_Init(void);
void UART1_Init(void);
void GPIO_Init(void);
void ADC1_Init(void);
void TIM3_Init_60Hz_Sim(void);
void TIM4_Init_Delay(void);
void TIM2_Init(void);
void EXTI_Init_ZC_Detect(void);
void EXTI_Init_PB8(void);
/* === Prototipos de Tareas de FreeRTOS === */
void control_task(void*);
void status_task(void*);
void blinkFunction(TimerHandle_t);

/* === Prototipo para retardo de precisión === */
void DWT_Delay_us(uint32_t microseconds);

/* === Manejador de Errores === */
void Error_Handler(void);

#endif /* __MAIN_H */
