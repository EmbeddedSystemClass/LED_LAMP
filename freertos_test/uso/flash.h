#ifndef FLASH_H
#define FLASH_H
#include "stm32f10x_flash.h"

/* Define the STM32F10x FLASH Page Size depending on the used STM32 device */
#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)
#else
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)
#endif

#define BANK1_WRITE_START_ADDR  ((uint32_t)0x08008000)
#define BANK1_WRITE_END_ADDR    ((uint32_t)0x08008400)

#ifdef STM32F10X_XL
  #define BANK2_WRITE_START_ADDR   ((uint32_t)0x08088000)
  #define BANK2_WRITE_END_ADDR     ((uint32_t)0x0808C000)
#endif /* STM32F10X_XL */

void FLASH_Write(uint32_t *data, uint16_t len);
void FLASH_Read(uint32_t *data, uint16_t len);

#endif
