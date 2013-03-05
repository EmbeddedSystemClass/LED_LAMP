#include "flash.h"


#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t EraseCounter = 0x00, Address = 0x00;
uint32_t Data = 0x3210ABCD;
uint32_t NbrOfPage = 0x00;
volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
volatile TestStatus MemoryProgramStatus = PASSED;
uint8_t FlashError=0;

#ifdef STM32F10X_XL
volatile TestStatus MemoryProgramStatus2 = PASSED;
#endif /* STM32F10X_XL */

void FLASH_Write(uint32_t *data, uint16_t len)
{
/* Porgram FLASH Bank1 ********************************************************/
    /* Unlock the Flash Bank1 Program Erase controller */
    FLASH_UnlockBank1();

    /* Define the number of page to be erased */
    NbrOfPage = (BANK1_WRITE_END_ADDR - BANK1_WRITE_START_ADDR) / FLASH_PAGE_SIZE;

    /* Clear All pending flags */
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    /* Erase the FLASH pages */
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_ErasePage(BANK1_WRITE_START_ADDR + (FLASH_PAGE_SIZE * EraseCounter));
    }

    /* Program Flash Bank1 */
    Address = BANK1_WRITE_START_ADDR;

    while((Address < (BANK1_WRITE_START_ADDR+len)) && (FLASHStatus == FLASH_COMPLETE))
    {
        FLASHStatus = FLASH_ProgramWord(Address, data);
        data++;
        Address = Address + 4;
    }

    FLASH_LockBank1();

    /* Check the corectness of written data */
    Address = BANK1_WRITE_START_ADDR;

    while((Address < BANK1_WRITE_END_ADDR) && (MemoryProgramStatus != FAILED))
    {
        if((*(__IO uint32_t*) Address) != Data)
        {
            MemoryProgramStatus = FAILED;
        }
        Address += 4;
    }



    if( MemoryProgramStatus == FAILED)
    {
       // while(1);
    	FlashError=1;
    }

}

void FLASH_Read(uint32_t *data, uint16_t len)
{
    while((Address <(BANK1_WRITE_START_ADDR+len)))
    {
    	data=(*(__IO uint32_t*) Address);
    	data++;
        Address += 4;
    }
}
