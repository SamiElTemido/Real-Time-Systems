#include "main.h"

TimerHandle_t xTimer;
SemaphoreHandle_t xSem;

int idx_to_bin(int m);

/* Variables compartidas */
unsigned volatile short int count = 0;
unsigned volatile short int k_new = 0;
unsigned volatile short int k_pre = 0;
unsigned volatile short int flag = 0;

const uint8_t NUM[16] = {
    /*0*/ 0b11111100,
    /*1*/ 0b01100000,
    /*2*/ 0b11011010,
    /*3*/ 0b11110010,
    /*4*/ 0b01100110,
    /*5*/ 0b10110110,
    /*6*/ 0b10111110,
    /*7*/ 0b11100000,
    /*8*/ 0b11111110,
    /*9*/ 0b11110110,
    /*A*/ 0b11101110,
    /*B*/ 0b00111110,
    /*C*/ 0b10011100,
    /*D*/ 0b01111010,
    /*E*/ 0b10011110,
    /*F*/ 0b10001110
};

int keyboard[4][4] = {
    {1, 2, 3,10}, // Fila 0
    {4, 5, 6, 11}, // Fila 1
    {7, 8, 9, 12}, // Fila 2
    {14, 0, 15,13}  // Fila 3
};//c1//c2//c3//c4

int main(void)
{
	System_Init();
	xSem = xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
	xTaskCreate(main_task, "Main", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(display_task, "Display", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(doorTask, "Door", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	xTimerStart(xTimer, 0);
	vTaskStartScheduler();
	return 0;
}

void main_task(void *pvParameters)
{
    UNUSED(pvParameters);

    uint8_t col, fil;
    int current_key = -1;    /* -1 = no teclea */
    int last_key_state = -1;

    while (1)
    {
        current_key = -1;
        for (col = 1; col < 16; col <<= 1)
        {
            GPIOB->ODR = (GPIOB->ODR & ~0x0F) | col;
            vTaskDelay(1);
            fil = GPIOB->IDR & 0xF0;
            if (fil != 0)
            {
                current_key = key(idx_to_bin(col), idx_to_bin(fil >> 4));
                break;
            }
        }

        if (current_key != -1 && last_key_state == -1)
        {
            k_new = current_key;
            flag = 1;
        }
        last_key_state = current_key;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void display_task(void *pvParameters)
{
    UNUSED(pvParameters);

    unsigned short int d1, d2, d3, d4, idx;
    d1 = d2 = d3 = d4 = idx = 0;

    while (1)
    {
        if (flag == 1)
        {
            d1 = d2;
            d2 = d3;
            d3 = d4;
            d4 = k_new;
            flag = 0;

            if ((d1 == 1) & (d2 == 2) & (d3 == 3) & (d4 == 4))
            {
                xSemaphoreGive(xSem);
                d1 = d2 = d3 = d4 = 0;
            }
        }

        for (idx = 1; idx < 16; idx <<= 1)
        {
            unsigned short int val = (idx == 1) ? d4 : (idx == 2) ? d3 : (idx == 4) ? d2 : d1;
            if (val > 15)
                val = 0;
            GPIOA->ODR = (NUM[val]) | (idx << 8);
            vTaskDelay(pdMS_TO_TICKS(1));
            GPIOA->ODR = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/* Callback del temporizador: parpadeo de LED (no bloquear aquí) */
void blinkFunction(TimerHandle_t xTimer)
{
    (void)xTimer;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

int key(int row, int col){
    // Comprueba que los índices estén dentro de los límites del array
    if (row < 4 && col < 4) {
        return keyboard[row][col];
    }
    return 0; // Retorna nulo si hay un error
}

void doorTask(void* pvParameters)
{
	UNUSED(pvParameters);
	while(1)
	{
		xSemaphoreTake(xSem,portMAX_DELAY);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
		vTaskDelay(pdMS_TO_TICKS(10000));
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

	}
}

int idx_to_bin(int m) {
    switch (m) {
        case 1:  return 0;
        case 2:  return 1;
        case 4:  return 2;
        case 8:  return 3;
        default: return 0;
    }
}

