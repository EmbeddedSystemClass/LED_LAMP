#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

#include <misc.h>


//Инклуды от FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"



#include "PWM.h"
#include "ir_control.h"
#include "vibration.h"
#include "led_effects.h"


int main(void)
{
	SystemInit();

	PWM_Init();//инициализация шим
	IR_Control_Init();//инициализация ик управления
	Vibration_Init();//инициализация вибродатчика
	Effects_Init();//инициализация световых эффектов

    vTaskStartScheduler();

    while(1);
}
