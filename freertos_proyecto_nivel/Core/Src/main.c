#include <main.h>
// --- Globales de FreeRTOS ---
QueueHandle_t xqueue;
TimerHandle_t xTimer;
SemaphoreHandle_t xSem;
SemaphoreHandle_t xI2C_Mutex;

// --- Handles de Periféricos ---
UART_HandleTypeDef huart1 = {0};
I2C_HandleTypeDef hi2c1 = {0};

// --- Variables Globales para datos ---
uint8_t buflen = 0;

// --- Variables para PITCH (Eje Y) ---
float angle_acc_pitch = 0.0;
float angle_gyro_y = 0.0;
float theta_pitch = 0.0; // Ángulo filtrado
float bias_pitch = 0.0;  // Bias del giroscopio
int16_t cal_gyro_y = 0;  // Calibración

// --- Variables para ROLL (Eje X) ---
float angle_acc_roll = 0.0;
float angle_gyro_x = 0.0;
float theta_roll = 0.0; // Ángulo filtrado
float bias_roll = 0.0;  // Bias del giroscopio
int16_t cal_gyro_x = 0; // Calibración


// =====================================================================================
//                                       MAIN
// =====================================================================================
int main(void)
{
    System_Init();

    //queue
    xqueue = xQueueCreate(128, sizeof(char));

    //semaforos
    xSem = xSemaphoreCreateMutex();
    xI2C_Mutex = xSemaphoreCreateMutex();

    //timer
    xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
    xTimerStart(xTimer, 10);

    //tasks
    xTaskCreate(oled_task, "Oled",1024, NULL, 2, NULL);
    xTaskCreate(sample_task, "Sample", configMINIMAL_STACK_SIZE*4, NULL, 4, NULL);
    xTaskCreate(transmit_task, "Transmit", 1024 , NULL, 2, NULL);

    vTaskStartScheduler();
    return 0;
}



// =====================================================================================
//                               OLED TASK
// =====================================================================================
void oled_task(void* pvParameter) {
    UNUSED(pvParameter);

    /* --- Proteger inicialización I2C --- */
    xSemaphoreTake(xI2C_Mutex, portMAX_DELAY);
    ssd1306_Init();
    xSemaphoreGive(xI2C_Mutex);

    float local_pitch;
    float local_roll;

    // Variable para cambiar entre modos de visualización
    uint8_t display_mode = 0; // 0: horizon, 1: bars, 2: bubble, 3: graph

    while(1) {
        /* --- Lectura protegida de variables --- */
//        if(xSemaphoreTake(xSem, portMAX_DELAY) == pdTRUE) {
            local_pitch = theta_pitch+2;
            local_roll = theta_roll-1;
//            xSemaphoreGive(xSem);
//        }

        /* --- Limpiar pantalla --- */
        ssd1306_Fill(Black);

        /* --- Dibujar según modo seleccionado --- */
        switch(display_mode) {
            case 0:
                // Horizonte artificial
                draw_horizon(local_pitch, local_roll);
                draw_angle_bars(local_pitch, local_roll);
                break;

            case 1:
                // Barras con valores
                draw_bar_graph(local_pitch, local_roll);
                break;

            case 2:
                // Nivel de burbuja
                draw_bubble_level(local_roll, local_pitch);
//                // Mostrar valores numéricos debajo
//                char buf[32];
//                ssd1306_SetCursor(10,56);
//                sprintf(buf, "P:%.1f R:%.1f", local_pitch, local_roll);
//                ssd1306_WriteString(buf, Font_11x18, White);
                break;

            case 3:
                // Gráfico con historial
                draw_scrolling_graph(local_pitch, local_roll);
                break;
        }

        /* --- Actualización física por I2C --- */
        xSemaphoreTake(xI2C_Mutex, portMAX_DELAY);
        ssd1306_UpdateScreen();
        xSemaphoreGive(xI2C_Mutex);

        vTaskDelay(pdMS_TO_TICKS(20)); // 20 FPS

        // Opcional: cambiar modo cada 10 segundos
        // static uint32_t mode_counter = 0;
        // if (++mode_counter >= 200) { // 200 * 50ms = 10 sec
        //     mode_counter = 0;
        //     display_mode = (display_mode + 1) % 4;
        // }
    }
}



// =====================================================================================
//                               SAMPLE TASK (Kalman)
// =====================================================================================
void sample_task(void *pvParameters)
{
    char bufIn[14];
    int16_t raw_accel_x, raw_accel_y, raw_accel_z;
    int16_t raw_gyro_x, raw_gyro_y;
    float x_accel=0, y_accel=0, z_accel=0;

    // --- Parámetros del Filtro Kalman (PITCH) ---
    double P_pitch[2][2] = {{10.0, 0}, {0, 10.0}};
    float omega_pitch = 0.0;

    // --- Parámetros del Filtro Kalman (ROLL) ---
    double P_roll[2][2] = {{10.0, 0}, {0, 10.0}};
    float omega_roll = 0.0;

    const float Q_angle = 0.001;
    const float Q_gyroBias = 0.001;
    const float R_measure = 0.1;

    // --- Proteger inicialización I2C ---
    xSemaphoreTake(xI2C_Mutex, portMAX_DELAY);
    MPU_Init();
    MPU_Calibrate();
    xSemaphoreGive(xI2C_Mutex);

    bias_pitch = (float)cal_gyro_y * DEGSXSEC;
    bias_roll = (float)cal_gyro_x * DEGSXSEC;

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TS * 1000); // TS = 0.001
    xLastWakeTime = xTaskGetTickCount();
    HAL_StatusTypeDef i2c_status;

    while(1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        //===================================================================
        // 1. PREDICCIÓN
        //===================================================================
        omega_pitch = angle_gyro_y - bias_pitch;
        theta_pitch += TS * omega_pitch;

        P_pitch[0][0] += TS * (TS * P_pitch[1][1] - P_pitch[0][1] - P_pitch[1][0] + Q_angle);
        P_pitch[0][1] -= TS * P_pitch[1][1];
        P_pitch[1][0] -= TS * P_pitch[1][1];
        P_pitch[1][1] += TS * Q_gyroBias;

        omega_roll = angle_gyro_x - bias_roll;
        theta_roll += TS * omega_roll;

        P_roll[0][0] += TS * (TS * P_roll[1][1] - P_roll[0][1] - P_roll[1][0] + Q_angle);
        P_roll[0][1] -= TS * P_roll[1][1];
        P_roll[1][0] -= TS * P_roll[1][1];
        P_roll[1][1] += TS * Q_gyroBias;

        //===================================================================
        // 2. MEDICIÓN (Lectura de I2C)
        //===================================================================
        bufIn[0] = 0x3B;

        xSemaphoreTake(xI2C_Mutex, portMAX_DELAY);
        i2c_status = HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, (uint8_t *)bufIn, 1, 100);

        if (i2c_status != HAL_OK) {
            xSemaphoreGive(xI2C_Mutex);
            continue;
        }

        i2c_status = HAL_I2C_Master_Receive(&hi2c1, MPU_ADDRESS, (uint8_t *)bufIn, 14, 100);
        xSemaphoreGive(xI2C_Mutex);

        if (i2c_status != HAL_OK) continue;

        // Procesar datos crudos
        raw_accel_x = ((int16_t)bufIn[0] << 8) | bufIn[1];
        raw_accel_y = ((int16_t)bufIn[2] << 8) | bufIn[3];
        raw_accel_z = ((int16_t)bufIn[4] << 8) | bufIn[5];

        raw_gyro_x  = (((int16_t)bufIn[8]  << 8) | bufIn[9])  - cal_gyro_x;
        raw_gyro_y  = (((int16_t)bufIn[10] << 8) | bufIn[11]) - cal_gyro_y;

        x_accel = -raw_accel_x * G_SCALE;
        y_accel =  raw_accel_y * G_SCALE;
        z_accel =  raw_accel_z * G_SCALE;

        angle_gyro_x = raw_gyro_x * DEGSXSEC;
        angle_gyro_y = raw_gyro_y * DEGSXSEC;

        angle_acc_pitch = atan2(x_accel, z_accel) * RADTODEG;
        angle_acc_roll  = atan2(y_accel, z_accel) * RADTODEG;

        //===================================================================
        // 3. ACTUALIZACIÓN (Escritura de variables)
        //===================================================================

        //if(xSemaphoreTake(xSem, (TickType_t)0) == pdTRUE)
        //{
            // --- PITCH ---
            double y_pitch = angle_acc_pitch - theta_pitch;
            double S_pitch = P_pitch[0][0] + R_measure;

            double K0_pitch = P_pitch[0][0] / S_pitch;
            double K1_pitch = P_pitch[1][0] / S_pitch;

            theta_pitch += K0_pitch * y_pitch;
            bias_pitch  += K1_pitch * y_pitch;

            double p00_temp = P_pitch[0][0];
            double p01_temp = P_pitch[0][1];

            P_pitch[0][0] -= K0_pitch * p00_temp;
            P_pitch[0][1] -= K0_pitch * p01_temp;
            P_pitch[1][0] -= K1_pitch * p00_temp;
            P_pitch[1][1] -= K1_pitch * p01_temp;

            // --- ROLL ---
            double y_roll = angle_acc_roll - theta_roll;
            double S_roll = P_roll[0][0] + R_measure;

            double K0_roll = P_roll[0][0] / S_roll;
            double K1_roll = P_roll[1][0] / S_roll;

            theta_roll += K0_roll * y_roll;
            bias_roll  += K1_roll * y_roll;

            p00_temp = P_roll[0][0];
            p01_temp = P_roll[0][1];

            P_roll[0][0] -= K0_roll * p00_temp;
            P_roll[0][1] -= K0_roll * p01_temp;
            P_roll[1][0] -= K1_roll * p00_temp;
            P_roll[1][1] -= K1_roll * p01_temp;

           // xSemaphoreGive(xSem);
        //}
    }
}



// =====================================================================================
//                               TRANSMIT TASK
// =====================================================================================
void transmit_task(void *pvParameters)
{
    char buffer[64];

    while(1)
    {
        float local_pitch;
        float local_roll;

        // Proteger lectura
        if(xSemaphoreTake(xSem, portMAX_DELAY) == pdTRUE)
        {
            local_pitch = theta_pitch;
            local_roll  = theta_roll;
            xSemaphoreGive(xSem);
        }

        // Formato
        buflen = sprintf(buffer,"%.2f\t%.2f\n\r", local_pitch, local_roll);

        // Enviar a la cola
        for(int i = 0; i < buflen; i++)
            xQueueSend(xqueue, &buffer[i], portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}



// =====================================================================================
//                               IDLE HOOK (UART)
// =====================================================================================
void vApplicationIdleHook(void)
{
    char byteToSend;

    // No bloquea nunca
    while(xQueueReceive(xqueue, &byteToSend, 0) == pdPASS)
    {
        // Timeout corto → NO bloquea el Idle
        HAL_UART_Transmit(&huart1, (uint8_t *)&byteToSend, 1, 1);
    }

    __WFI(); // bajo consumo
}



// =====================================================================================
//                               CALIBRACIÓN MPU
// =====================================================================================
void MPU_Calibrate(void) {
    uint8_t bufIn[14];
    int32_t sum_gyro_x = 0, sum_gyro_y = 0, sum_gyro_z = 0;
    int32_t sum_accel_x = 0, sum_accel_y = 0, sum_accel_z = 0;

    const int NUM_SAMPLES = 1000;
    int valid_samples = 0;

    // Esperar a que el MPU se estabilice
    vTaskDelay(pdMS_TO_TICKS(100));

    // Opcional: Indicar al usuario que la calibración está en progreso
    // (puedes encender un LED o mostrar en el OLED)

    for (int i = 0; i < NUM_SAMPLES; i++) {
        bufIn[0] = 0x3B;  // ACCEL_XOUT_H (leer desde acelerómetro)

        // Leer todos los sensores de una vez (accel + temp + gyro)
        if(HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, bufIn, 1, 100) != HAL_OK) {
            continue;
        }
        if(HAL_I2C_Master_Receive(&hi2c1, MPU_ADDRESS, bufIn, 14, 100) != HAL_OK) {
            continue;
        }

        // Acumular valores del acelerómetro
        sum_accel_x += (int16_t)(bufIn[0] << 8 | bufIn[1]);
        sum_accel_y += (int16_t)(bufIn[2] << 8 | bufIn[3]);
        sum_accel_z += (int16_t)(bufIn[4] << 8 | bufIn[5]);

        // Acumular valores del giroscopio
        sum_gyro_x += (int16_t)(bufIn[8] << 8 | bufIn[9]);
        sum_gyro_y += (int16_t)(bufIn[10] << 8 | bufIn[11]);
        sum_gyro_z += (int16_t)(bufIn[12] << 8 | bufIn[13]);

        valid_samples++;

        vTaskDelay(pdMS_TO_TICKS(2));  // 2ms entre muestras
    }

    // Calcular promedios del giroscopio
    if (valid_samples > 0) {
        cal_gyro_x = sum_gyro_x / valid_samples;
        cal_gyro_y = sum_gyro_y / valid_samples;
        // cal_gyro_z = sum_gyro_z / valid_samples;  // Si lo necesitas

        // Para el acelerómetro:
        // X e Y deberían ser ~0 si está horizontal
        // Z debería ser ~16384 (1g) si está boca arriba, o ~-16384 si está boca abajo
        // Por ahora no los usamos, pero podrías guardarlos si es necesario
    }
}




// =====================================================================================
//                               BLINK TIMER
// =====================================================================================
void blinkFunction(TimerHandle_t xTimer)
{
    UNUSED(xTimer);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
