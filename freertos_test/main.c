#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

#include <misc.h>

#include <stdio.h>

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

	PWM_Init();
	IR_Control_Init();
	Vibration_Init();
	Effects_Init();

    vTaskStartScheduler();

    while(1);
}
