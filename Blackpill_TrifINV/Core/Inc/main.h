#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"

// Definiciones globales
void System_Init(void);
void Clock_Init(void);
void GPIO_Init(void);
void TIM1_Init_60Deg(void); // Función específica para tu requerimiento
void Error_Handler(void);

#endif /* __MAIN_H */
