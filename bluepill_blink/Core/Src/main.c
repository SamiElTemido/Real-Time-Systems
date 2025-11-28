// main.c (Ejemplo con FreeRTOS)
#include "main.h"

// Nota: La implementación de main_task debe estar en este o en otro archivo .c

int main(void)
{
    // 1. Inicialización de la capa HAL y del SysTick (el SysTick será usado por FreeRTOS)
    HAL_Init();

    // 2. Configuración del reloj del sistema (72 MHz) y GPIO (PC13)
    System_Init();

    // 3. Crear la tarea que parpadea el LED
    xTaskCreate(
        main_task,                  // Función de la tarea
        "LED_Blink",                // Nombre descriptivo
        configMINIMAL_STACK_SIZE,   // Tamaño mínimo del stack (definido en FreeRTOSConfig.h)
        NULL,                       // Parámetro de la tarea
        tskIDLE_PRIORITY + 1,       // Prioridad (más baja que la máxima, más alta que la Idle)
        NULL                        // Handle de la tarea (no necesario aquí)
    );

    // 4. Iniciar el planificador de FreeRTOS.
    //    El programa nunca debe salir de esta función si el planificador arranca correctamente.
    vTaskStartScheduler();

    // Si el planificador falla (ej. falta de memoria en el heap), el programa llega aquí.
    while (1)
    {
        Error_Handler();
    }
}
void main_task(void* pvParameters)
{
    // Convierte el tiempo en milisegundos a 'ticks' del RTOS (500 ms)
    const TickType_t xDelay = pdMS_TO_TICKS(500);

    // Bucle infinito de la tarea
    for(;;)
    {
        // 1. Alterna el estado del pin PC13
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

        // 2. Introduce un retardo sin bloquear a otras tareas
        vTaskDelay(xDelay);
    }
}
