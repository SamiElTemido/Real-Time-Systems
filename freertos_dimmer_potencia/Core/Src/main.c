#include "main.h"
#define TARGET_RPM 2500
// --- Variables Globales y Handlers ---
UART_HandleTypeDef huart1;
ADC_HandleTypeDef  hadc1;
TIM_HandleTypeDef  htim3;
TIM_HandleTypeDef  htim4;
TIM_HandleTypeDef  htim2;

TimerHandle_t xTimer;
QueueHandle_t xqueue;

// Variables de control de disparo
volatile uint32_t g_firing_delay_us = 100;
volatile uint8_t g_current_half_cycle = 0;
volatile bool g_is_timing_in_progress = false;
const uint32_t max_delay_us = 8333*2; //delay fuck u
    const uint32_t min_delay_us = 400;
    int adc_value=0;
int32_t pos=0;
float 	vel=0;
volatile int32_t ref=1000;
float error=0;
float Kp=0.03;
float ki=0.04;
float Kd=0.0;
float uOut=0.0;
float dt=0.001;

// 1 = SCR1 (PB6), 2 = SCR2 (PB7)
volatile uint8_t g_target_scr = 0;

// Definición de Pines (Ajusta si tus definiciones en main.h son diferentes)
#define ZC_PIN_A       GPIO_PIN_5   // Pin para semiciclo 1
#define ZC_PIN_B       GPIO_PIN_8   // Pin para semiciclo 2 (Nuevo)
#define SCR_PULSE_1_PIN GPIO_PIN_6  // Salida SCR 1
#define SCR_PULSE_2_PIN GPIO_PIN_7  // Salida SCR 2
#define SCR_PORT       GPIOB        // Puerto de los SCRs

int main(void)
{
    HAL_Init();
    System_Init(); // Asegúrate de que System_Init inicialice PB5 y PB8 como EXTI

    xqueue = xQueueCreate(128, sizeof(char));
    xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(50), pdTRUE, NULL, blinkFunction);
    xTimerStart(xTimer, 0);

    xTaskCreate(control_task, "ControlTask", configMINIMAL_STACK_SIZE*2, NULL, 2, NULL);
    xTaskCreate(status_task, "StatusTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

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
    float local_dt = 0.1;

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xPeriod);

        // 1. LEER POSICION
        pos = (int32_t)__HAL_TIM_GET_COUNTER(&htim2);

        // Resetear cuenta para calcular velocidad relativa en este intervalo
        __HAL_TIM_SET_COUNTER(&htim2, 0);

        // Calculo de velocidad (C1 debe ser 60.0 / PPR)
        vel = ((float)pos / TS) * C1;

        // 2. CALCULO DEL ERROR
        error = ref - vel;

        // 3. ALGORITMO PI
        float P = Kp * error;

        // Acumular error en el tiempo
        integral += error * local_dt;

        // --- CAMBIO 1: Límite del acumulado integral a 1000 ---
        if (integral > 500.0f) integral = 500.0f;
        else if (integral < -500.0f) integral = -500.0f;

        float I = ki * integral;

        control_signal = P + I;

        // --- CAMBIO 2: Saturación de la salida al 20% ---
        if (control_signal > 30.0f) control_signal = 30.0f;
        if (control_signal < 0.0f)  control_signal = 0.0f;

        // 4. ACTUAR
        // Convertir el porcentaje de control (0-20%) a retardo de disparo
        // A mayor control_signal, MENOR retardo (más potencia)
        float range = max_delay_us - min_delay_us;
        float delay_float = max_delay_us - ((control_signal / 100.0f) * range);

        g_firing_delay_us = (uint32_t)delay_float;

        // 5. DEBUG
        // Uso de snprintf para evitar errores con floats y formateo manual
        buflen = snprintf(buffer, sizeof(buffer),
            "Ref:%d Vel:%d.%d Cnt:%ld Out:%d.%d Err:%d.%d Del:%lu\r\n",
            (int)ref,
            (int)vel, (int)((vel - (int)vel) * 10), // Decimal manual (1 digito)
            pos,
            (int)control_signal, (int)((control_signal - (int)control_signal) * 10),
            (int)error, (int)((error - (int)error) * 10),
            g_firing_delay_us
        );

        // Enviar a la cola
        for(int i = 0; i < buflen; i++) {
            xQueueSend(xqueue, buffer + i, 0);
        }
    }
}

/**
 * @brief Interrupción de GPIO (Zero Crossing)
 * Aquí detectamos qué pin disparó y configuramos el Timer.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // Si ya estamos contando tiempo para un disparo, ignoramos ruido extra
    // (A menos que quieras permitir disparos superpuestos, lo cual es peligroso con un solo Timer)
    if (g_is_timing_in_progress) {
        return;
    }

    // Caso 1: Detección de Cruce por Cero A (Pin 5) -> Prepara SCR 1
    if (GPIO_Pin == ZC_PIN_A)
    {
        g_is_timing_in_progress = true;
        g_target_scr = 1; // Marcar que dispararemos el SCR 1

        // Configurar Timer 4 One-Pulse
        __HAL_TIM_SET_AUTORELOAD(&htim4, g_firing_delay_us);
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        HAL_TIM_Base_Start_IT(&htim4);
    }
    // Caso 2: Detección de Cruce por Cero B (Pin 8) -> Prepara SCR 2
    else if (GPIO_Pin == ZC_PIN_B)
    {
        g_is_timing_in_progress = true;
        g_target_scr = 2; // Marcar que dispararemos el SCR 2

        // Configurar Timer 4 One-Pulse
        __HAL_TIM_SET_AUTORELOAD(&htim4, g_firing_delay_us);
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        HAL_TIM_Base_Start_IT(&htim4);
    }
}

/**
 * @brief Interrupción del Timer (Disparo)
 * Se ejecuta cuando pasan los microsegundos calculados (g_firing_delay_us).
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4)
    {
        // Detenemos el timer para que no se repita
        HAL_TIM_Base_Stop_IT(&htim4);

        // Disparamos el SCR correspondiente según quién inició la cuenta
        if (g_target_scr == 1)
        {
            HAL_GPIO_WritePin(SCR_PORT, SCR_PULSE_1_PIN, GPIO_PIN_SET);
            DWT_Delay_us(50); // Ancho del pulso de disparo
            HAL_GPIO_WritePin(SCR_PORT, SCR_PULSE_1_PIN, GPIO_PIN_RESET);
        }
        else if (g_target_scr == 2)
        {
            HAL_GPIO_WritePin(SCR_PORT, SCR_PULSE_2_PIN, GPIO_PIN_SET);
            DWT_Delay_us(50); // Ancho del pulso de disparo
            HAL_GPIO_WritePin(SCR_PORT, SCR_PULSE_2_PIN, GPIO_PIN_RESET);
        }

        // Liberamos la bandera para permitir la siguiente detección
        g_is_timing_in_progress = false;
        g_target_scr = 0;
    }
}

// Retardo preciso usando DWT (bloqueante, usar solo para tiempos muy cortos < 100us)
void DWT_Delay_us(uint32_t microseconds)
{
    uint32_t clk_cycle_start = DWT->CYCCNT;
    uint32_t cycles_per_us = (HAL_RCC_GetHCLKFreq() / 1000000);
    uint32_t num_cycles = microseconds * cycles_per_us;
    while ((DWT->CYCCNT - clk_cycle_start) < num_cycles);
}

void status_task(void *pvParameter)
{
    // Eliminamos buffer y buflen porque no se usan aquí (se usan en control_task)

    for(;;)
    {
        // 1. Iniciar la conversión ADC
        HAL_ADC_Start(&hadc1);

        // 2. Esperar a que termine (Timeout de 10ms es suficiente)
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            // 3. Leer el valor convertido
            adc_value = HAL_ADC_GetValue(&hadc1);

            // 4. Calcular la referencia (Mapeo 0-4095 a 0-3000 RPM)
            // Usamos 4095.0f para asegurar división flotante
            ref = (int32_t)((adc_value / 4095.0f) * 3000.0f);

            // Opcional: Zona muerta para que en 0V sea exactamente 0 RPM
            if(ref < 50) ref = 0;
        }

        // Detener el ADC para ahorrar energía (opcional pero recomendado)
        HAL_ADC_Stop(&hadc1);

        // Frecuencia de actualización de la perilla (100ms es buena respuesta humana)
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
