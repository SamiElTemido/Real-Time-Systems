#include <main.h>
#include <string.h>   // Para strcmp, strlen

TimerHandle_t xTimer;
UART_HandleTypeDef huart1 = {0};
SPI_HandleTypeDef hspi1;

volatile int value = 0;
volatile int paso = 0;
int startDebug = 0;   // Flag para habilitar debug

int main(void)
{
    System_Init();
    xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
    xTaskCreate(read_task,  "Read",  configMINIMAL_STACK_SIZE * 2, NULL, 0, NULL);
    xTaskCreate(debug_task, "Debug", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
    xTimerStart(xTimer,10);
    vTaskStartScheduler();
    return 0;
}

void read_task(void *pvParameters)
{
    (void)pvParameters;
    uint8_t rxcv[64];
    memset(rxcv, 0, sizeof(rxcv));

    while (1)
    {
        // Leer hasta que llegue "Ok" (bloquea hasta recibir 2 bytes)
    	if (!startDebug)
        HAL_UART_Receive(&huart1, rxcv, 2, HAL_MAX_DELAY);

        if (strncmp((char*)rxcv, "Ok", 2) == 0) {
            startDebug = 1;  // Activa debug
            HAL_UART_Transmit(&huart1, (uint8_t*)"Debug habilitado\r\n", 18, HAL_MAX_DELAY);
            memset(rxcv, 0, sizeof(rxcv));
        }

        if (startDebug) {

        }
    }
}

void debug_task(void *pvParameters)
{
    (void)pvParameters;
    uint8_t txbuf[64];

    while (1) {
        if (startDebug) {
            int n = snprintf((char*)txbuf, sizeof txbuf, "Sensor: %i\tPaso: %i\r\n", value, paso);
            if (n > 0) {
                HAL_UART_Transmit(&huart1, txbuf, (uint16_t)n, HAL_MAX_DELAY);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void blinkFunction(TimerHandle_t xTimer)
{
    (void)xTimer;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
