#include <main.h>

// --- Globales de FreeRTOS ---
QueueHandle_t xqueue;
TimerHandle_t xTimer;
SemaphoreHandle_t xSem;

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

int main(void)
{
	System_Init();

	xqueue = xQueueCreate(64, sizeof(char));
	xSem = xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
	xTaskCreate(sample_task, "Sample", configMINIMAL_STACK_SIZE*4, NULL, 4, NULL);
	xTaskCreate(transmit_task, "Transmit", configMINIMAL_STACK_SIZE * 4, NULL, 2, NULL);
	xTimerStart(xTimer, 10);

	vTaskStartScheduler();
    return 0;
}

void sample_task(void *pvParameters)
{
	char bufIn[14]; // <-- Aumentado a 14 para leer todos los sensores
	int16_t raw_accel_x, raw_accel_y, raw_accel_z;
	int16_t raw_gyro_x, raw_gyro_y;
	float x_accel=0, y_accel=0, z_accel=0;

    // --- Parámetros del Filtro Kalman (PITCH) ---
	double P_pitch[2][2] = {{10.0, 0}, {0, 10.0}};
    float omega_pitch = 0.0;

    // --- Parámetros del Filtro Kalman (ROLL) ---
	double P_roll[2][2] = {{10.0, 0}, {0, 10.0}};
    float omega_roll = 0.0;

    // Constantes de ruido (compartidas)
	const float Q_angle = 0.001;
	const float Q_gyroBias = 0.001;
	const float R_measure = 0.1;


	MPU_Init();
	MPU_Calibrate();

    // <-- Asignar calibración inicial a los filtros
    bias_pitch = (float)cal_gyro_y * DEGSXSEC;
    bias_roll = (float)cal_gyro_x * DEGSXSEC;

    // --- Configuración de la Tarea ---
	TickType_t xLastWakeTime;
    // <-- Frecuencia basada en la constante TS (10ms)
	const TickType_t xFrequency = pdMS_TO_TICKS(TS * 1000);
	xLastWakeTime = xTaskGetTickCount();

    HAL_StatusTypeDef i2c_status; // Variable para comprobar el I2C

	while(1)
	{
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		//===================================================================
		// 1. PREDICCIÓN (ambos ejes)
		//===================================================================

		/*** PITCH (Eje Y) *******************************************/
		omega_pitch = angle_gyro_y - bias_pitch;
		theta_pitch = theta_pitch + TS * omega_pitch; // <-- CORRECCIÓN: quitado el '10*'

		// <-- CORRECCIÓN: matemática de covarianza
		P_pitch[0][0] = P_pitch[0][0] + TS * (TS * P_pitch[1][1] - P_pitch[0][1] - P_pitch[1][0] + Q_angle);
		P_pitch[0][1] = P_pitch[0][1] - TS * P_pitch[1][1];
		P_pitch[1][0] = P_pitch[1][0] - TS * P_pitch[1][1];
		P_pitch[1][1] = P_pitch[1][1] + TS * Q_gyroBias;

		/*** ROLL (Eje X) ********************************************/
		omega_roll = angle_gyro_x - bias_roll;
		theta_roll = theta_roll + TS * omega_roll;

		P_roll[0][0] = P_roll[0][0] + TS * (TS * P_roll[1][1] - P_roll[0][1] - P_roll[1][0] + Q_angle);
		P_roll[0][1] = P_roll[0][1] - TS * P_roll[1][1];
		P_roll[1][0] = P_roll[1][0] - TS * P_roll[1][1];
		P_roll[1][1] = P_roll[1][1] + TS * Q_gyroBias;

		//===================================================================
		// 2. MEDICIÓN (Lectura de I2C)
		//===================================================================
		bufIn[0] = 0x3B; // Registro inicial de datos

        // <-- CORRECCIÓN: Usar timeout y comprobar estado
		i2c_status = HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, (uint8_t *)bufIn, 1, 100);
        if (i2c_status != HAL_OK) continue; // Si falla, reintentar en el prox. ciclo

        // <-- CORRECCIÓN: Leer 14 bytes para tener Accel, Temp y Gyro
		i2c_status = HAL_I2C_Master_Receive(&hi2c1, MPU_ADDRESS, (uint8_t *)bufIn, 14, 100);
        if (i2c_status != HAL_OK) continue;

        // Procesar datos crudos
		raw_accel_x = ((int16_t)bufIn[0] << 8) | bufIn[1];
		raw_accel_y = ((int16_t)bufIn[2] << 8) | bufIn[3];
		raw_accel_z = ((int16_t)bufIn[4] << 8) | bufIn[5];
        // bufIn[6] y [7] son de Temperatura (los ignoramos)
		raw_gyro_x  = (((int16_t)bufIn[8] << 8) | bufIn[9])   - cal_gyro_x; // <-- Eje X
		raw_gyro_y  = (((int16_t)bufIn[10] << 8) | bufIn[11]) - cal_gyro_y; // <-- Eje Y

        // Convertir a unidades físicas
		x_accel = -raw_accel_x * G_SCALE; // <-- Negado para PITCH
		y_accel = raw_accel_y * G_SCALE;
		z_accel = raw_accel_z * G_SCALE;

        angle_gyro_x = raw_gyro_x * DEGSXSEC;
		angle_gyro_y = raw_gyro_y * DEGSXSEC;

        // Mediciones del Acelerómetro
		angle_acc_pitch = atan2(x_accel, z_accel) * RADTODEG;
        angle_acc_roll  = atan2(y_accel, z_accel) * RADTODEG;

		// <-- CORRECCIÓN: Eliminado el bloque 'if/else' que corrompía los datos

		//===================================================================
		// 3. ACTUALIZACIÓN (ambos ejes)
		//===================================================================

		/*** PITCH (Eje Y) *******************************************/
		double y_pitch = angle_acc_pitch - theta_pitch;
		double S_pitch = P_pitch[0][0] + R_measure;
		double K0_pitch = P_pitch[0][0] / S_pitch;
		double K1_pitch = P_pitch[1][0] / S_pitch;

		theta_pitch = theta_pitch + K0_pitch * y_pitch;
		bias_pitch = bias_pitch + K1_pitch * y_pitch;

		double p00_temp = P_pitch[0][0];
		double p01_temp = P_pitch[0][1];
		P_pitch[0][0] = P_pitch[0][0] - K0_pitch * p00_temp;
		P_pitch[0][1] = P_pitch[0][1] - K0_pitch * p01_temp;
		P_pitch[1][0] = P_pitch[1][0] - K1_pitch * p00_temp;
		P_pitch[1][1] = P_pitch[1][1] - K1_pitch * p01_temp;

        /*** ROLL (Eje X) ********************************************/
		double y_roll = angle_acc_roll - theta_roll;
		double S_roll = P_roll[0][0] + R_measure;
		double K0_roll = P_roll[0][0] / S_roll;
		double K1_roll = P_roll[1][0] / S_roll;

		theta_roll = theta_roll + K0_roll * y_roll;
		bias_roll = bias_roll + K1_roll * y_roll;

		p00_temp = P_roll[0][0];
		p01_temp = P_roll[0][1];
		P_roll[0][0] = P_roll[0][0] - K0_roll * p00_temp;
		P_roll[0][1] = P_roll[0][1] - K0_roll * p01_temp;
		P_roll[1][0] = P_roll[1][0] - K1_roll * p00_temp;
		P_roll[1][1] = P_roll[1][1] - K1_roll * p01_temp;
	}
}

void transmit_task(void *pvParameters)
{
	char buffer[64];
	UNUSED(pvParameters);
	while(1)
	{
        // <-- Enviar PITCH y ROLL filtrados
		buflen = sprintf(buffer,"%.2f\t%.2f\n\r", theta_pitch, theta_roll);

		for(int i = 0; i < buflen; i++)
			xQueueSend(xqueue, &buffer[i], portMAX_DELAY);

		vTaskDelay(pdMS_TO_TICKS(20));
	}
}


void vApplicationIdleHook(void)
{
	char byteToSend;
	if(xQueueReceive(xqueue, &byteToSend, 0) == pdPASS)
	{
        // <-- CORRECCIÓN: Usar timeout corto, no HAL_MAX_DELAY
		HAL_UART_Transmit(&huart1, (uint8_t *)&byteToSend, 1, 10);
	}
}

void MPU_Calibrate(void)
{
	uint8_t bufIn[4]; // Solo necesitamos 4 bytes
	int32_t i;
    int16_t raw_gyro_x, raw_gyro_y;
	int32_t sum_x = 0;
    int32_t sum_y = 0;
    HAL_StatusTypeDef i2c_status;

	for (i = 0; i < 1000; i++)
	{
		bufIn[0] = 0x43; // Registro de Gyro X (High byte)

        // <-- CORRECCIÓN: Usar timeouts
		i2c_status = HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDRESS, (uint8_t *)bufIn, 1, 100);
        if (i2c_status != HAL_OK) continue;

        // <-- Leer 4 bytes (Gyro X y Gyro Y)
		i2c_status = HAL_I2C_Master_Receive(&hi2c1, MPU_ADDRESS, (uint8_t *)bufIn, 4, 100);
        if (i2c_status != HAL_OK) continue;

		raw_gyro_x = ((int16_t)bufIn[0] << 8) | bufIn[1];
        raw_gyro_y = ((int16_t)bufIn[2] << 8) | bufIn[3];
		sum_x += raw_gyro_x;
        sum_y += raw_gyro_y;

        vTaskDelay(pdMS_TO_TICKS(1)); // Pequeña espera
	}
    // <-- Calcular ambos promedios
	cal_gyro_x = sum_x / 1000;
	cal_gyro_y = sum_y / 1000;
}

void blinkFunction(TimerHandle_t xTimer)
{
	UNUSED(xTimer);
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
