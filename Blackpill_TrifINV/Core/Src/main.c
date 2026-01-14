// SAMUEL MICHAEL GARCIA GONZALEZ
#include "main.h"

// Definimos ON y OFF
// Timer Period = 3333, así que 3335 asegura 100% de ciclo de trabajo
#define ON  3335
#define OFF 0

// Variables globales
TIM_HandleTypeDef htim1;

// --- SECUENCIA "ESCALONADA" (STAGGERED) ---
// Basada estrictamente en tu imagen image_bb3301
// Cada paso dura 3.33ms (60 grados)
uint16_t secuencia_fase[6][3] = {
    // Canal 1 (A), Canal 2 (B), Canal 3 (C)
    // D0 (Blanco), D2 (Rojo),   D4 (Amarillo)

    {ON,  OFF, OFF}, // Paso 1: Solo A encendida
    {ON,  ON,  OFF}, // Paso 2: A y B encendidas (Escalón sube)
    {ON,  ON,  ON }, // Paso 3: A, B y C encendidas (Todos arriba)
    {OFF, ON,  ON }, // Paso 4: Se apaga A (Escalón baja)
    {OFF, OFF, ON }, // Paso 5: Se apaga B
    {OFF, OFF, OFF}  // Paso 6: Se apaga C (Todos apagados, hueco final)
};

volatile uint8_t paso = 0;

int main(void)
{
    System_Init();

    // Inicializamos Timer a 3.33ms (Ver main_init.c del mensaje anterior)
    TIM1_Init_60Deg();

    // Arrancamos Timer con interrupciones
    HAL_TIM_Base_Start_IT(&htim1);

    // Arrancamos PWMs (Canales y sus complementarios)
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

    while(1)
    {
        // Nada aquí
    }
    return 0;
}

// Callback de la interrupción (Cada 3.33ms)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        // Actualizamos salidas
        TIM1->CCR1 = secuencia_fase[paso][0];
        TIM1->CCR2 = secuencia_fase[paso][1];
        TIM1->CCR3 = secuencia_fase[paso][2];

        // Siguiente paso
        paso++;
        if (paso >= 6) paso = 0;
    }
}

void TIM1_UP_TIM10_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}
