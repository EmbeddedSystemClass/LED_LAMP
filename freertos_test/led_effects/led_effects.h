#ifndef LED_EFFECTS_H
#define LED_EFFECTS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define LED_CHNL_NUM 4

//--------------------7 ������ ������-------------
enum rainbow_color { RED,ORANGE,YELLOW,GREEN,BLUE,STRONG_BLUE,PURPLE};

//------------------------------------------------
struct led_channel//�������� ��������� ������
{
	unsigned int hue;//����
	unsigned char saturation;//�������
	unsigned char volume;//�������
};
//--------------------------------------------
void Effects_Init(void);
void Rainbow_Process( void *pvParameters );
void Blink_Process( void *pvParameters );
void Libration_Process( void *pvParameters );
void Set_White(void);
void Set_Color(unsigned char color);

void Random_Init(void);
void Random_Process( void *pvParameters );
void Set_Random_Color(void);
#endif
