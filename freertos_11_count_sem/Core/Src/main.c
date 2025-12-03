#include "main.h"

TimerHandle_t xTimer;
SemaphoreHandle_t xSem;

unsigned volatile short int count = 0;
unsigned volatile short int k_new = 0;
unsigned volatile short int digits[4] = {17, 17, 17, 17}; // Inicializado apagado
unsigned volatile short int psw[4] = {0, 0, 0, 0};
unsigned volatile short int idx;
unsigned volatile short int flag_key_pressed = 0;
unsigned volatile short int check_password_flag = 0;
unsigned volatile short int psw_set_flag = 0; // 0 si no, 1 si si

const uint8_t NUM[21] = {
    /*0*/0xFC, /*1*/0x60, /*2*/0xDA, /*3*/0xF2, /*4*/0x66, /*5*/0xB6, /*6*/0xBE, /*7*/0xE0,
    /*8*/0xFE, /*9*/0xF6, /*A*/0xEE, /*B*/0x3E, /*C*/0x9C, /*D*/0x7A, /*E*/0x9E, /*F*/0x8E,
    /*P*/0xCE, /*off*/0x00, /*L*/0x1C, /*n*/0x2A, /*o*/0x3A
};

int keyboard[4][4] = {
    {1, 2, 3, 10},  // Fila 0
    {4, 5, 6, 11},  // Fila 1
    {7, 8, 9, 12},  // Fila 2
    {14, 0, 15, 13} // Fila 3
};//c1	c2 c3 c4

int main(void)
{
	System_Init();
	xSem = xSemaphoreCreateBinary();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
	xTaskCreate(main_task, "Main", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(display_task, "Display", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTaskCreate(doorTask, "Door", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	xTaskCreate(read_keyboard, "ReadKey", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	xTimerStart(xTimer, 0);
	vTaskStartScheduler();
	return 0;
}

void main_task(void *pvParameters) {
    UNUSED(pvParameters);
    while(1)
    {
        if(check_password_flag == 1) {
            if(psw_set_flag == 1) {
                if((digits[0] == psw[0]) && (digits[1] == psw[1]) && (digits[2] == psw[2]) && (digits[3] == psw[3])) {
                    display_open_blink();
                    xSemaphoreGive(xSem); // semaforo para abrir la puerta
                    psw_set_flag = 0;
                } else {
                    for(int i = 0; i < 3; i++) {
                        digits[0] = 15; // F
                        digits[1] = 10; // A
                        digits[2] = 1;  // 1
                        digits[3] = 18; // L
                        vTaskDelay(pdMS_TO_TICKS(100));
                        for(int j = 0; j < 4; j++) digits[j] = 17;//apaga los digitos
                        vTaskDelay(pdMS_TO_TICKS(100));
                    }
                }
            } else {
                for(int j = 0; j < 4; j++) psw[j] = digits[j]; // Guarda la contraseña
                psw_set_flag = 1;
                display_close_scroll();
                xSemaphoreGive(xSem);
            }

            check_password_flag = 0;
            count = 0;
            for(int j = 0; j < 4; j++) digits[j] = 17;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void read_keyboard(void *pvParameters)
{
    UNUSED(pvParameters);
    uint8_t col, fil;
    int current_key = -1;
    int last_key_state = -1;

    while (1) {
        current_key = -1;
        for (col = 1; col < 16; col <<= 1) {
            GPIOB->ODR = (GPIOB->ODR & ~0x0F) | col;
            vTaskDelay(1);
            fil = GPIOB->IDR & 0xF0;
            if (fil != 0) {
                current_key = key(idx_to_bin(col), idx_to_bin(fil >> 4));
                break;
            }
        }

        if (current_key != -1 && last_key_state == -1) {
            if (current_key == 15) { // Si la tecla es '#' (Enter)
                check_password_flag = 1;
            } else {
                k_new = current_key;
                flag_key_pressed = 1;
            }
        }
        last_key_state = current_key;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void doorTask(void* pvParameters)
{
	UNUSED(pvParameters);
	while(1)
	{
		if(xSemaphoreTake(xSem,portMAX_DELAY) == pdTRUE){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); // Abre cerrojo
			vTaskDelay(pdMS_TO_TICKS(10000)); // Mantiene abierto por 10 seg
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);   // Cierra cerrojo
		}
	}
}

void display_task(void *pvParameters){
	UNUSED(pvParameters);
	while(1){
		if(flag_key_pressed == 1){
			for(int i = 0; i < 3; i++) digits[i] = digits[i+1];
			digits[3] = k_new;
			flag_key_pressed = 0;
			count++;
		}

		for(idx = 1; idx < 16; idx <<= 1) {
			unsigned short int val;
			switch(idx){
				case 1: val = digits[3]; break;
				case 2: val = digits[2]; break;
				case 4: val = digits[1]; break;
				case 8: val = digits[0]; break;
				default: val = 17;
			}
			if (val > 20) val = 17;
			GPIOA->ODR = (NUM[val]) | (idx << 8);
			vTaskDelay(pdMS_TO_TICKS(1));
			GPIOA->ODR = 0;
		}
		vTaskDelay(pdMS_TO_TICKS(1));
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

int key(int row, int col){
    if (row < 4 && col < 4) {
        return keyboard[row][col];
    }
    return -1; // regresa -1 si hay un error
}

/* Callback del temporizador: parpadeo de LED (no bloquear aquí) */
void blinkFunction(TimerHandle_t xTimer)
{
    (void)xTimer;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void display_open_blink(void) {
    for(int i = 0; i < 5; i++) {
        digits[0] = 20; // o
        digits[1] = 16; // P
        digits[2] = 14; // E
        digits[3] = 19; // n
        vTaskDelay(pdMS_TO_TICKS(150));
        for(int j = 0; j < 4; j++) digits[j] = 17; // off
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void display_close_scroll(void) {
    const int C = 12, L = 18, O = 20, S = 5, E = 14, OFF = 17;
    int word[] = {C, L, O, S, E};

    for(int j = 0; j < 4; j++) digits[j] = OFF;
    vTaskDelay(pdMS_TO_TICKS(250));
    // Animación de scroll
    digits[3] = word[0]; // ...C
    vTaskDelay(pdMS_TO_TICKS(250));
    digits[2] = word[0]; digits[3] = word[1]; // ..CL
    vTaskDelay(pdMS_TO_TICKS(250));
    digits[1] = word[0]; digits[2] = word[1]; digits[3] = word[2]; // .CLO
    vTaskDelay(pdMS_TO_TICKS(250));
    digits[0] = word[0]; digits[1] = word[1]; digits[2] = word[2]; digits[3] = word[3]; // CLOS
    vTaskDelay(pdMS_TO_TICKS(250));
    digits[0] = word[1]; digits[1] = word[2]; digits[2] = word[3]; digits[3] = word[4]; // LOSE
    vTaskDelay(pdMS_TO_TICKS(250));
    digits[0] = word[2]; digits[1] = word[3]; digits[2] = word[4]; digits[3] = OFF; // OSE.
    vTaskDelay(pdMS_TO_TICKS(250));

    for(int j = 0; j < 4; j++) digits[j] = OFF;
}
