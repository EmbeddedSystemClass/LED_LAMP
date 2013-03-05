#ifndef PWM_H
#define PWM_H

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
//------------------------------------
#define CHANNEL1_RED 		TIM3->CCR4//TIM2->CCR1
#define CHANNEL1_GREEN		TIM3->CCR3//TIM2->CCR2
#define CHANNEL1_BLUE		TIM3->CCR2//TIM2->CCR3

#define CHANNEL2_RED		TIM3->CCR1//TIM2->CCR4
#define CHANNEL2_GREEN		TIM2->CCR4//TIM3->CCR1
#define CHANNEL2_BLUE		TIM2->CCR3//TIM3->CCR2

#define CHANNEL3_RED		TIM2->CCR2//TIM3->CCR3
#define CHANNEL3_GREEN		TIM2->CCR1//TIM3->CCR4
#define CHANNEL3_BLUE		TIM4->CCR4//TIM4->CCR1

#define CHANNEL4_RED		TIM4->CCR3//
#define CHANNEL4_GREEN		TIM4->CCR2//
#define CHANNEL4_BLUE		TIM4->CCR1//
//------------------------------------
void PWM_Init(void);
void HSVtoRGB(unsigned int channel,unsigned int HUE, unsigned char SAT,unsigned char VOL);

#endif
