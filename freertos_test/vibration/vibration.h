#ifndef VIBRATION_H
#define VIBRATION_H
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include<stm32f10x_adc.h>
#include <misc.h>

//Инклуды от FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

void Vibration_Init(void);
void Vibro_Process( void *pvParameters );
#endif
