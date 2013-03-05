#ifndef CHANNELS_H
#define CHANNELS_H
#include <stm32f10x.h>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"
//#include "preferences.h"
#include "misc.h"
//---------------------------------------
#define CHANNEL_NUMBER	12//количество каналов
//---------------------------------------
#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)
#else
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#endif

#define DESC_BANK1_WRITE_START_ADDR  ((uint32_t)0x08020400)
#define DESC_BANK1_WRITE_END_ADDR    ((uint32_t)0x08020800)


#define BANK1_WRITE_START_ADDR  ((uint32_t)0x08020000)
#define BANK1_WRITE_END_ADDR    ((uint32_t)0x08020400)

#ifdef STM32F10X_XL
  #define BANK2_WRITE_START_ADDR   ((uint32_t)0x08088000)
  #define BANK2_WRITE_END_ADDR     ((uint32_t)0x0808C000)
#endif /* STM32F10X_XL */
//---------------------------------------
struct Channel
{
	unsigned char number ;	  	// номер канала

	union
	{
		struct
		{
			unsigned char type;		  // тип канала
			unsigned char modific;	  // модификатор канала
			unsigned char state_byte_1;	// байт состояния канала
			unsigned char state_byte_2;
		}set;

		unsigned long serialize;//сериализованная часть структуры
	} settings;

	unsigned long channel_data;		  // значение канала
	unsigned long channel_data_calibrate;//значение канала калиброванное

	union
	{
		struct
		{
			unsigned char calibrate;//флаг калиброванности канала


			float K;//
			float C;//y=kx+c


		} cal;
		unsigned long serialize[3];//сериализованная часть структуры
	} calibrate;

} ;
//---------------------------------------
void ChannelsInit(void);//инициализация структуры каналов
//void Store_Channels_Data(void);//Сохраним данные каналов в ППЗУ
//void Restore_Channels_Data(void);//Восстановим данные каналов из ППЗУ

//---------------------------------------
#endif
