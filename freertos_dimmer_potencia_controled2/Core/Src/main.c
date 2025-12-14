#include "main.h"
#define TARGET_RPM 3000.0f

UART_HandleTypeDef huart1;
ADC_HandleTypeDef  hadc1;
TIM_HandleTypeDef  htim3;
TIM_HandleTypeDef  htim4;
TIM_HandleTypeDef htim2;
TimerHandle_t xTimer;
QueueHandle_t xqueue;

/* ... (Variables globales sin cambios) ... */
volatile uint32_t g_firing_delay_us = 100;
volatile uint8_t g_current_half_cycle = 0;
volatile bool g_is_timing_in_progress = false;
const uint32_t max_delay_us = 8333*2; //delay fuck u
    const uint32_t min_delay_us = 400;
int32_t pos=0;
float 	vel=0;
int32_t ref=0;
float error=0;
float Kp=0.05;
float ki=0.001;
float Kd=0.0;
float uOut=0.0;
float dt=0.001;

int main(void)
{
    // ... (main sin cambios) ...
    System_Init();
    xqueue = xQueueCreate(128, sizeof(char));
    xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
    xTimerStart(xTimer, 0);

    xTaskCreate(control_task, "ControlTask", configMINIMAL_STACK_SIZE*2, NULL, 2, NULL);
  //xTaskCreate(status_task, "StatusTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();
    return 0;
}
void control_task(void *pvParameter)
{
    char buffer[64];
    unsigned char buflen;

    // Variables PID
    float integral = 0.0;
    float control_signal = 0.0; // porcentual de todo la capacidad

    // Sincronización (100ms = 0.1 segundos)
    const TickType_t xPeriod = pdMS_TO_TICKS(100);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // Definir TS correcto para el cálculo de velocidad
    // Si la tarea corre cada 100ms, TS debe ser 0.1
    float local_dt = 0.1;

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // 1. LEER POSICION
        pos = (int32_t)__HAL_TIM_GET_COUNTER(&htim2);

        // IMPORTANTE: Para calcular velocidad (RPM), necesitamos resetear la cuenta
        // o calcular la diferencia (pos_actual - pos_anterior).
        // Si no reseteas, 'pos' crecerá infinitamente.
        __HAL_TIM_SET_COUNTER(&htim2, 0);

        // Calculo de velocidad
        // Asegúrate que C1 = 60.0 / PPR
        vel = ((float)pos / TS) * C1;

        // 2. CALCULO DEL ERROR
        error = TARGET_RPM - vel;

        // 3. ALGORITMO PI
        float P = Kp * error;
        integral += error * local_dt;

        // Anti-Windup
        float max_integral = 100.0f / (ki > 0.0001f ? ki : 1.0f);
        if (integral > max_integral) integral = max_integral;
        else if (integral < -max_integral) integral = -max_integral;

        float I = ki * integral;

        control_signal = P + I;

        // Saturación
        if (control_signal > 20.0f) control_signal = 20.0f;
        if (control_signal < 0.0f)   control_signal = 0.0f;

        // 4. ACTUAR (CORREGIDO)
        // Restamos el MINIMO para obtener el rango (aprox 7800 us)
        float range = max_delay_us - min_delay_us;
        float delay_float = max_delay_us - ((control_signal / 100.0f) * range);

        //g_firing_delay_us = (uint32_t)delay_float;
        g_firing_delay_us=min_delay_us*1;
        // 5. DEBUG (CORREGIDO)
        // Imprimimos la velocidad calculada (float) y la cuenta cruda (long)
        // Si mueves el eje con la mano, deberías ver cambiar "Cnt"
        buflen = sprintf(buffer, "Ref:%.0f Vel:%.1f Cnt:%ld Out:%.1f Err:%.1f\r\n", TARGET_RPM, vel, pos, control_signal,error);

        for(int i = 0; i < buflen; i++) {
            xQueueSend(xqueue, buffer + i, 0); // Timeout 0 para no bloquear
        }
    }
}

// ... (El resto de las funciones: HAL_GPIO_EXTI_Callback, HAL_TIM_PeriodElapsedCallback, etc., se mantienen exactamente igual que en la versión anterior) ...

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ZC_DETECT_IN_PIN)
    {
        if (g_is_timing_in_progress) {
            return;
        }
        g_is_timing_in_progress = true;

        if (HAL_GPIO_ReadPin(ZC_DETECT_IN_PORT, ZC_DETECT_IN_PIN) == GPIO_PIN_SET) {
            g_current_half_cycle = 0;
        } else {
            g_current_half_cycle = 1;
        }

        __HAL_TIM_SET_AUTORELOAD(&htim4, g_firing_delay_us);
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        HAL_TIM_Base_Start_IT(&htim4);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4)
    {
        HAL_TIM_Base_Stop_IT(&htim4);

        if (g_current_half_cycle == 0) {
            HAL_GPIO_WritePin(SCR_PULSE_1_PORT, SCR_PULSE_1_PIN, GPIO_PIN_SET);
            DWT_Delay_us(50);
            HAL_GPIO_WritePin(SCR_PULSE_1_PORT, SCR_PULSE_1_PIN, GPIO_PIN_RESET);
        } else {
            HAL_GPIO_WritePin(SCR_PULSE_2_PORT, SCR_PULSE_2_PIN, GPIO_PIN_SET);
            DWT_Delay_us(50);
            HAL_GPIO_WritePin(SCR_PULSE_2_PORT, SCR_PULSE_2_PIN, GPIO_PIN_RESET);
        }

        g_is_timing_in_progress = false;
    }
}

void DWT_Delay_us(uint32_t microseconds)
{
    uint32_t clk_cycle_start = DWT->CYCCNT;
    uint32_t cycles_per_us = (HAL_RCC_GetHCLKFreq() / 1000000);
    uint32_t num_cycles = microseconds * cycles_per_us;
    while ((DWT->CYCCNT - clk_cycle_start) < num_cycles);
}

void status_task(void *pvParameter)
{
    char buffer[64];
    uint8_t buflen;
    for(;;)
    {
        buflen = sprintf(buffer, "Angulo de disparo (delay): %lu us\r\n", g_firing_delay_us);
        for(int i = 0; i < buflen; i++) {
            xQueueSend(xqueue, buffer + i, portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void blinkFunction(TimerHandle_t xTimer)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void vApplicationIdleHook(void)
{
    unsigned char byteToSend;
    if(xQueueReceive(xqueue, &byteToSend, 0) == pdPASS)
    {
        HAL_UART_Transmit(&huart1, &byteToSend, 1, HAL_MAX_DELAY);
    }
}
