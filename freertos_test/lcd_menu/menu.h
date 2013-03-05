#ifndef _MENU_H_
#define _MENU_H_




void initMenu();
void Menu_Previous(void);
void Menu_Parent(void);
void Menu_Child(void);
void Menu_Select(void);
void Menu_Next(void);
void MenuHandler( void *pvParameters );
unsigned char startMenu(void);
enum {
    MENU_CANCEL=1,
    MENU_RESET,
    MENU_MODE1,
    MENU_MODE2,
    MENU_MODE3,
    MENU_SENS1,
    MENU_SENS2,
    MENU_WARM,
    MENU_PROCESS
};

#endif
