#include "main.h"
TimerHandle_t xTimer;
//freertos 6 de hecho
unsigned volatile short int count =0;

const uint8_t NUM[16] = {   /*0*/ 0b0111111,    /*1*/ 0b0000110,    /*2*/ 0b1011011,
							/*3*/ 0b1001111,    /*4*/ 0b1100110,    /*5*/ 0b1101101,
							/*6*/ 0b1111101,    /*7*/ 0b0000111,    /*8*/ 0b1111111,	//sin nuto decimal
							/*9*/ 0b1101111,    /*A*/ 0b1110111,    /*B*/ 0b1111100,
							/*C*/ 0b0111001,    /*D*/ 0b1011110,    /*E*/ 0b1111001,
							/*F*/ 0b1110001 };

int main(void)
{
	System_Init();
	xTimer = xTimerCreate("Blink", pdMS_TO_TICKS(500), pdTRUE, NULL, blinkFunction);
	xTaskCreate(main_task, "Main", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(display_task, "Display", configMINIMAL_STACK_SIZE, NULL, 0, NULL);
	xTimerStart(xTimer,10);
	vTaskStartScheduler();
	return 0;
}

void main_task(void *pvParameters){
	UNUSED(pvParameters);

	while(1){
		count = (count+1)%10000;
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void display_task(void *pvParameters){
	UNUSED(pvParameters);
	unsigned short int d1,d2,d3,d4,idx;
	d1=d2=d3=d4=idx=0;

	while(1){

		d1=(count/1000)%10;
		d2=(count/100)%10;
		d3=(count/10)%10;
		d4=(count)%10;

		for(idx=1;idx<16;idx<<=1)
		{

			unsigned short int val =(idx==1)? d1 : (idx==2)? d2 :(idx==4)? d3 : d4;
			GPIOA->ODR = idx | (NUM[val]<<4);
			vTaskDelay(pdMS_TO_TICKS(1));
			GPIOA->ODR = idx | (0x00<<4);
		}

	}
}

void blinkFunction(TimerHandle_t xTimer)	//timers no deben bloquear al procesador //no ciclos infinitos
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}


