#ifndef DOL_H
#define DOL_H

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

void DOL_Init(void);//инициализация таймера дола
void DOL_Process( void *pvParameters );//процесс обновления регистра дола-необязательный


#endif
