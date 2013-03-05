#include "channels.h"
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
//-----------------------------------
struct Channel  channels[CHANNEL_NUMBER];
//-----------------------------------
void ChannelsInit(void) //using 0//инициализация структуры каналов
{

//	Restore_Channels_Data();
//	if(channels[0].settings.set.type!=0 || channels[0].settings.set.modific!=3)
	{
		channels[0].number=0;		  // номера каналов должны строго идти последовательно и начинаться с нуля
		channels[0].settings.set.type=0;
		channels[0].settings.set.modific=3;
		channels[0].settings.set.state_byte_1=0x40;
		channels[0].settings.set.state_byte_2=0x06;
		channels[0].channel_data=1;
		channels[0].channel_data_calibrate=11;
		channels[0].calibrate.cal.calibrate=0;
		// КАНАЛ 2 - фиксированый АЦП
		channels[1].number=1;
		channels[1].settings.set.type=0;
		channels[1].settings.set.modific=3;
		channels[1].settings.set.state_byte_1=0x40;
		channels[1].settings.set.state_byte_2=0x06;
		channels[1].channel_data=2;
		channels[1].channel_data_calibrate=22;
		channels[1].calibrate.cal.calibrate=0;
		// КАНАЛ 3 - фиксированый АЦП
		channels[2].number=2;
		channels[2].settings.set.type=0;
		channels[2].settings.set.modific=3;
		channels[2].settings.set.state_byte_1=0x40;
		channels[2].settings.set.state_byte_2=0x06;
		channels[2].channel_data=3;
		channels[2].channel_data_calibrate=33;
		channels[2].calibrate.cal.calibrate=0;
		// КАНАЛ 4 - фиксированый АЦП
		channels[3].number=3;
		channels[3].settings.set.type=0;
		channels[3].settings.set.modific=3;
		channels[3].settings.set.state_byte_1=0x40;
		channels[3].settings.set.state_byte_2=0x06;
		channels[3].channel_data=0;
		channels[3].channel_data_calibrate=0;
		channels[3].calibrate.cal.calibrate=0;
		// КАНАЛ 5 - фиксированый АЦП
		channels[4].number=4;
		channels[4].settings.set.type=0;
		channels[4].settings.set.modific=3;
		channels[4].settings.set.state_byte_1=0x40;
		channels[4].settings.set.state_byte_2=0x06;
		channels[4].channel_data=0;
		channels[4].channel_data_calibrate=0;
		channels[4].calibrate.cal.calibrate=0;
		// КАНАЛ 6 - фиксированый АЦП
		channels[5].number=5;
		channels[5].settings.set.type=0;
		channels[5].settings.set.modific=3;
		channels[5].settings.set.state_byte_1=0x40;
		channels[5].settings.set.state_byte_2=0x06;
		channels[5].channel_data=0;
		channels[5].channel_data_calibrate=0;
		channels[5].calibrate.cal.calibrate=0;
		// КАНАЛ 7 - фиксированый АЦП
		channels[6].number=6;
		channels[6].settings.set.type=0;
		channels[6].settings.set.modific=3;
		channels[6].settings.set.state_byte_1=0x40;
		channels[6].settings.set.state_byte_2=0x06;
		channels[6].channel_data=0;
		channels[6].channel_data_calibrate=0;
		channels[6].calibrate.cal.calibrate=0;
		// КАНАЛ 8 - фиксированый АЦП
		channels[7].number=7;
		channels[7].settings.set.type=0;
		channels[7].settings.set.modific=3;
		channels[7].settings.set.state_byte_1=0x40;
		channels[7].settings.set.state_byte_2=0x06;
		channels[7].channel_data=0;
		channels[7].channel_data_calibrate=0;
		channels[7].calibrate.cal.calibrate=0;
		// КАНАЛ 9 - Частотомер 0-256 Гц
		channels[8].number=8;
		channels[8].settings.set.type=2;
		channels[8].settings.set.modific=1;
		channels[8].settings.set.state_byte_1=0x40;
		channels[8].settings.set.state_byte_2=0x0A;
		channels[8].channel_data=0x0;
		channels[8].calibrate.cal.calibrate=0;
		// КАНАЛ 10 - Частотомер 0-256 Гц
		channels[9].number=9;
		channels[9].settings.set.type=2;
		channels[9].settings.set.modific=1;
		channels[9].settings.set.state_byte_1=0x40;
		channels[9].settings.set.state_byte_2=0x0A;
		channels[9].channel_data=0x0;
		channels[9].calibrate.cal.calibrate=0;
		// КАНАЛ 11 - Частотомер 0-256 Гц
		channels[10].number=10;
		channels[10].settings.set.type=2;
		channels[10].settings.set.modific=1;
		channels[10].settings.set.state_byte_1=0x40;
		channels[10].settings.set.state_byte_2=0x0A;
		channels[10].channel_data=0x1;
		channels[10].calibrate.cal.calibrate=0;
		// КАНАЛ 12 - ДОЛ I2C
		channels[11].number=11;
		channels[11].settings.set.type=1;
		channels[11].settings.set.modific=0;
		channels[11].settings.set.state_byte_1=0x40;
		channels[11].settings.set.state_byte_2=0x0A;
		channels[11].channel_data=0x0;
		channels[11].calibrate.cal.calibrate=0;
	}
	return;
}
//-----------------------------------
void Store_Channels_Data(void) //using 0//Сохраним данные каналов в ППЗУ
{
	unsigned char i=0;
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

	for(i=0;i<CHANNEL_NUMBER;i++)
	{
		//EEPROM_Write(&channels[i].settings.serialize,1,ADC_SETTINGS_ADDR+i);


		        FLASHStatus = FLASH_ProgramWord(Address, channels[i].settings.serialize);
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
    	//FlashError=1;
    }
	return;
}
//-----------------------------------
void Restore_Channels_Data(void) //using 0//Восстановим данные каналов из ППЗУ
{
	unsigned char i=0;
	/* Program Flash Bank1 */
	Address = BANK1_WRITE_START_ADDR;
	for(i=0;i<CHANNEL_NUMBER;i++)
	{
		//EEPROM_Read(&channels[i].settings.serialize,1,ADC_SETTINGS_ADDR+i);
		channels[i].settings.serialize=(*(__IO uint32_t*) Address);

		Address += 4;
	}
	return;
}
