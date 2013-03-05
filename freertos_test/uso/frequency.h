#ifndef FREQUENCY_H
#define FREQUENCY_H

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include <misc.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


void Frequency_Process( void *pvParameters );//процесс обновления регистра дола-необязательный
uint16_t uint16_time_diff(uint16_t now, uint16_t before);


void Frequency_Init(void);


#endif
