#include "menu.h"
#include "hd44780_emcu.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include <stdio.h>

//Инклуды от FreeRTOS:

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef struct {
	void       *Next;
	void       *Previous;
	void       *Parent;
	void       *Child;
	uint8_t     Select;
	const char  Text[];
} menuItem;

menuItem* selectedMenuItem; // текущий пункт меню

menuItem* menuStack[10];
volatile unsigned char menuStackTop;
void MenuHandler( void *pvParameters );
extern xQueueHandle xKeyQueue;//очередь клавиатуры
extern xSemaphoreHandle xKeySemaphore;

unsigned int delay=0;

#define MAKE_MENU(Name, Next, Previous, Parent, Child, Select, Text) \
    extern menuItem Next;     \
	extern menuItem Previous; \
	extern menuItem Parent;   \
	extern menuItem Child;  \
	menuItem Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, (uint8_t)Select, { Text }}

#define PREVIOUS   selectedMenuItem->Previous
#define NEXT       selectedMenuItem->Next
#define PARENT     selectedMenuItem->Parent
#define CHILD      selectedMenuItem->Child
#define SELECT	   selectedMenuItem->Select

char strNULL[]  = "";

#define NULL_ENTRY Null_Menu
menuItem        Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, 0, {0x00}};

//                 NEXT,      PREVIOUS     PARENT,     CHILD
MAKE_MENU(m_s1i1,  m_s1i2,    NULL_ENTRY,  NULL_ENTRY, m_s2i1,       0, "Start");
MAKE_MENU(m_s1i2,  m_s1i3,    m_s1i1,      NULL_ENTRY, m_s3i1,       0, "Settings");
MAKE_MENU(m_s1i3,  NULL_ENTRY,m_s1i2,      NULL_ENTRY, NULL_ENTRY,   MENU_RESET, "Reset");

// подменю Запуск
MAKE_MENU(m_s2i1,  m_s2i2,    NULL_ENTRY,  m_s1i1,     NULL_ENTRY,   MENU_MODE1, "Mode 1");
MAKE_MENU(m_s2i2,  m_s2i3,    m_s2i1,      m_s1i1,     NULL_ENTRY,   MENU_MODE2, "Mode 2");
MAKE_MENU(m_s2i3,  NULL_ENTRY,m_s2i2,      m_s1i1,     NULL_ENTRY,   MENU_MODE3, "Mode 3");

// подменю Настройка
MAKE_MENU(m_s3i1,  m_s3i2,    NULL_ENTRY,  m_s1i2,     m_s4i1,       0, "Pressure");
MAKE_MENU(m_s3i2,  NULL_ENTRY,m_s3i1,      m_s1i2,     m_s5i1,       0, "Time");

// подменю Давление
MAKE_MENU(m_s4i1,  m_s4i2,    NULL_ENTRY,  m_s3i1,     NULL_ENTRY,   MENU_SENS1, "Sensor 1");
MAKE_MENU(m_s4i2,  NULL_ENTRY,m_s4i1,      m_s3i1,     NULL_ENTRY,   MENU_SENS2, "Sensor 2");

// подменю Время
MAKE_MENU(m_s5i1,  m_s5i2,    NULL_ENTRY,  m_s3i2,     NULL_ENTRY,   MENU_WARM, "Warm");
MAKE_MENU(m_s5i2,  NULL_ENTRY,m_s5i1,      m_s3i2,     NULL_ENTRY,   MENU_PROCESS, "Process");


void menuChange(menuItem* NewMenu)
{
	if ((void*)NewMenu == (void*)&NULL_ENTRY)
	  return;

	selectedMenuItem = NewMenu;
}

unsigned char dispMenu(void) {
	menuItem* tempMenu,*tempMenu2;

	LCD_CMD (1);//clrscr
	//delay=10000; while (delay--){}
	LCD_CMD (1);//clrscr
	LCD_CMD (1);//clrscr
	LCD_CMD (1);//clrscr
	// первая строка - заголовок. Или пункт меню верхнего уровня
	//delay=10000; while (delay--){}
	LCD_GOTO (1,0);
	tempMenu = selectedMenuItem->Parent;
	if ((void*)tempMenu == (void*)&NULL_ENTRY)
	{ // мы на верхнем уровне
		LCD_STRING ("MENU:");
	}

	else
	{
		 LCD_STRING ((char *)tempMenu->Text);
	}

	//lcd_clrscr(2);
	//
	LCD_GOTO (2,5);
	tempMenu2=selectedMenuItem->Previous;
	LCD_STRING ((char *)tempMenu2->Text);

	LCD_GOTO (3,4);
	LCD_PUTCHAR ('>');

	LCD_GOTO (3,5);
	LCD_STRING ((char *)selectedMenuItem->Text);
	LCD_GOTO (4,5);
	tempMenu2=selectedMenuItem->Next;
	LCD_STRING ((char *)tempMenu2->Text);

	return (1);
}



unsigned char startMenu(void) {
	selectedMenuItem = (menuItem*)&m_s1i1;

	dispMenu();

	return (0);
}

void initMenu() {

}

void Menu_Previous(void)
{
	//LCD_CMD (1);//clrscr
	menuChange(PREVIOUS);
	dispMenu();
	//LCD_CMD (1);//clrscr
	//LCD_CMD (1);
	//LCD_GOTO (25);
	//LCD_STRING ("PREV");

}
void Menu_Parent(void)
{
	//LCD_CMD (1);//clrscr
	menuChange(PARENT);
	dispMenu();
	//LCD_CMD (1);
	//LCD_GOTO (25);
	//LCD_STRING ("PARENT");
}
void Menu_Child(void)
{
	//LCD_CMD (1);//clrscr
	menuChange(CHILD);
	dispMenu();
	//LCD_CMD (1);
	//LCD_GOTO (25);
	//LCD_STRING ("CHILD");
}
void Menu_Select(void)
{
	menuChange(SELECT);
	dispMenu();
	//LCD_CMD (1);
	//LCD_GOTO (25);
	//LCD_STRING ("SELECT");
}
void Menu_Next(void)
{
	//LCD_CMD (1);//clrscr
	menuChange(NEXT);
	dispMenu();
	//LCD_CMD (1);
	//LCD_GOTO (25);
	//LCD_STRING ("NEXT");
}

void MenuHandler( void *pvParameters )
{
	unsigned int key;
	unsigned char str[20]="";
    LCD_INIT ();
    startMenu();
    while(1)
    {
        if( xKeySemaphore != NULL )
        {

            if( xSemaphoreTake( xKeySemaphore, ( portTickType ) 10 ) == pdTRUE )
            {
            	if( xKeyQueue != 0 )
            	 {
					 if( xQueueReceive( xKeyQueue, &( key ), ( portTickType ) 10 ) )
					 {
						 sprintf(&str,"%i",key);
						// LCD_STRING (&str);
						 if((key&GPIO_Pin_10)==0)
						 {

							 Menu_Previous();
						 }

						 if((key&GPIO_Pin_11)==0)
						 {
							 Menu_Next();
						 }

						 if((key&GPIO_Pin_12)==0)
						 {
							 Menu_Child();
						 }

						 if((key&GPIO_Pin_0)==1)
						 {
							 Menu_Parent();
						 }
					 }
            	 }
            }
            else
            {

            }
        }

    	vTaskDelay(10);
    }
}
