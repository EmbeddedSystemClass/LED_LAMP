#ifndef IR_CONTROL_H
#define IR_CONTROL_H

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
/*
#define KEY_1 		64
#define KEY_2		76
#define KEY_3		4
#define KEY_4		0
#define KEY_5		10
#define KEY_6		30
#define KEY_7		14
#define KEY_8		26
#define KEY_9		28
#define KEY_0		20
#define KEY_CH_UP	31
#define KEY_CH_DOWN	22
#define KEY_V_UP	21
#define KEY_V_DOWN	27
#define KEY_MENU	17
#define KEY_POWER	9
*/

#define KEY_0 		20
#define KEY_1 		64
#define KEY_2		76

#define KEY_3		253
#define KEY_4		250
#define KEY_5		255
#define KEY_6		30
#define KEY_7		254
#define KEY_8		249
#define KEY_9		28

#define KEY_CH_UP	31
#define KEY_CH_DOWN	22

#define KEY_V_UP	247
#define KEY_V_DOWN	252

#define KEY_MENU	17
#define KEY_POWER	9

#define START_MAX	1500
#define START_MIN	1000

#define BIT0_MAX	150
void IR_Control_Init(void);//инициализация IR управления пультом
void IR_Process( void *pvParameters );
#endif
