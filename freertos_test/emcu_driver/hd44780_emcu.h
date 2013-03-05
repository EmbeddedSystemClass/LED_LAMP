#ifndef hd44780_emcu_H
#define hd44780_emcu_H


    #include "stm32F10x.h" // Стандартный хидер CMSIS для STM32.
    void    Delay50cyc (void);
    void    LCD_CMD         (u32 CMD);
    u8      LCD_DAT         (u8 DAT);
    void    LCD_PUTCHAR     (u32 DAT);
    void    LCD_STRING      (u8 *STRING);
    void    LCD_INIT        (void);
    void    LCD_GOTO        (unsigned char str,unsigned char n);
    u8      LCD_READ        (void);
    void    LCD_VIEW_MODE   (u32 CURSOR1, u32 CURSOR2, u32 VIEW_ON);
    void    CREATE_CHAR     (char ADR, char Y1,char Y2,char Y3,char Y4,char Y5,char Y6,char Y7,char Y8);

    #define  NUM_OF_LINE        4   // Число строк LCD.
    #define  CHAR_IN_LINE       20  // Число символов в строке строк LCD.
    #define  LCD_C_PORT         PORTB // Порт управляющих сигналов CD, RW, EN.
	#define  LCD_CD             7 // Сигнал CD команда/данные.
	#define  LCD_RW             6 // Сигнал RW чтение/запись.
	#define  LCD_EN             5 // Сигнал EN разрешения данных на шине.
    #define  LCD_D_PORT         PORTB // Порт шины данных
    #define  LCD_D_SHIFT        12 // Смещение от начала порта до шины данных.
    #define  LCD_D_MASK         0x0FUL<<LCD_D_SHIFT // Маска шины данных.
    #define  ON                 1 // Значение константы "Включено".
    #define  OFF                0 // Значение константы "Выключено".
#endif
