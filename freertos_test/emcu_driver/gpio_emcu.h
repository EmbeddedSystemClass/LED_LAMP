/*******************************************************************************
    gpio_emcu.h - заголовочный файл модуля GPIO.
    Author: eugenemcu.ru. Date: 03.10.2011.
    Содержит макросы основных действий с GPIO STM32.

*******************************************************************************/
#ifndef gpio_emcu_H
#define gpio_emcu_H

#include "stm32F10x.h" // Стандартный хидер CMSIS для STM32F10X.

#define PORTA   ((u32*)(APB2PERIPH_BASE + 0x0800))
#define PORTB   ((u32*)(APB2PERIPH_BASE + 0x0C00))
#define PORTC   ((u32*)(APB2PERIPH_BASE + 0x1000))
#define PORTD   ((u32*)(APB2PERIPH_BASE + 0x1400))
#define PORTE   ((u32*)(APB2PERIPH_BASE + 0x1800))
#define PORTF   ((u32*)(APB2PERIPH_BASE + 0x1C00))
#define PORTG   ((u32*)(APB2PERIPH_BASE + 0x2000))

/*------------------------------------------------------------------------------
    Макросы для GPIO.
------------------------------------------------------------------------------*/

// Set pin - установка битов порта по маске. Например: spin_m (PORTC,(1<<4)|((1<<5)); // Установить PC4, PC5.
#define spin_m(port,mask) (*(port+4)=(mask))

// Clear pin - сброс битов порта по маске. Например: сpin_m (PORTВ,(1<<0)|((1<<1)); // Сбросить PB0, PB1.
#define cpin_m(port,mask) (*(port+4)=(mask)<<16)

// Invert pin - инверсия вывода порта. Например: ipin_m (PORTC,((1<<5)); // Изменить состояние PC5.
#define ipin_m(port,mask) (*(port+3)^=(mask)) 

// Write port - запись выходного регистра порта ODR. Например: wr_port(PORTC, 0xFFFF); // Записать 0xFFFF в GPIOC->ODR.
#define wr_port(port,word) (*(port+3)=(word))

// Read port - чтение входного регистра порта IDR. Например: tmp=rd_port(PORTA); // Считать GPIOA->IDR в tmp.
#define rd_port(port) (*(port+2))

// Check pin - проверка вывода порта. Например: if (getpin_b(PORTA, 0)) // Если вывод PA.0 установлен...
#define getpin_b(port,bit) (*(port+2)&(1<<(bit)))

// Push Pull Output - Настройка вывода b на двухтактный выход (MODE=11, CNF=00). Например: pinOutPP_b (PORTC,8); 
#define pinOutPP_b(port,bit)  {*(port+((bit)/8))|=3UL<<(((bit)-(8*((bit)/8)))<<2); *(port+((bit)/8))&=~(3UL<<((((bit)-(8*((bit)/8)))<<2)+2));}

// Open Drain Output - Вывод b - на выход c открытым стоком (MODE=11, CNF=01). Например: pinOutOD_b (PORTA,0); 
#define pinOutOD_b(port,bit)  {*(port+((bit)/8))|=3UL<<(((bit)-(8*((bit)/8)))<<2); *(port+((bit)/8))&=~(3UL<<((((bit)-(8*((bit)/8)))<<2)+2)); *(port+((bit)/8))|=(1UL<<((((bit)-(8*((bit)/8)))<<2)+2));}

// Alternative Function Output - Вывода b - Двухтактный выход c альтернативной функцией (MODE=11, CNF=10). Например: pinOutAF_b (PORTB,14); 
#define pinOutAF_b(port,bit)  {*(port+((bit)/8))|=3UL<<(((bit)-(8*((bit)/8)))<<2); *(port+((bit)/8))&=~(3UL<<((((bit)-(8*((bit)/8)))<<2)+2)); *(port+((bit)/8))|=(2UL<<((((bit)-(8*((bit)/8)))<<2)+2));}

// Alternative Function Output Open Drain - Вывода b - C открытым стоком и альтернативной функцией (MODE=11, CNF=11). Например: pinOutAFOD_b (PORTB,14); 
#define pinOutAFOD_b(port,bit)  {*(port+((bit)/8))|=3UL<<(((bit)-(8*((bit)/8)))<<2); *(port+((bit)/8))&=~(3UL<<((((bit)-(8*((bit)/8)))<<2)+2)); *(port+((bit)/8))|=(3UL<<((((bit)-(8*((bit)/8)))<<2)+2));}

// Z-state Input - Вывод b - вход в третьем Z-состоянии (MODE=00, CNF=01). pinOutZ (PORTA,0); // Вывод PA0 на вход. 
#define pinInputZ_b(port,bit)  {*(port+((bit)/8))&=~(3UL<<(((bit)-(8*((bit)/8)))<<2)); *(port+((bit)/8))&=~(3UL<<((((bit)-(8*((bit)/8)))<<2)+2)); *(port+((bit)/8))|=(1UL<<((((bit)-(8*((bit)/8)))<<2)+2)); }

// Input Pull Up - Вывод b - вход c резистором на шину питания (MODE=00, CNF=10, ODR=1). pinInputPU_b (PORTD,12); // Вывод PD12 - PullUp вход. 
#define pinInputPU_b(port,bit)  {*(port+((bit)/8))&=~(3UL<<(((bit)-(8*((bit)/8)))<<2)); *(port+((bit)/8))&=~(3UL<<((((bit)-(8*((bit)/8)))<<2)+2)); *(port+((bit)/8))|=(2UL<<((((bit)-(8*((bit)/8)))<<2)+2)); (*(port+3)|=(1<<(bit)));}

// Input Pull Down - Вывод b - вход c резистором на общий (MODE=00, CNF=10, ODR=0). pinInputPD_b (PORTD,5); // Вывод PD5 - PullDown вход. 
#define pinInputPD_b(port,bit)  {*(port+((bit)/8))&=~(3UL<<(((bit)-(8*((bit)/8)))<<2)); *(port+((bit)/8))&=~(3UL<<((((bit)-(8*((bit)/8)))<<2)+2)); *(port+((bit)/8))|=(2UL<<((((bit)-(8*((bit)/8)))<<2)+2)); (*(port+3)&=~(1<<(bit)));}

#endif