#include "led_effects.h"
#include "PWM.h"
#include "stm32f10x_dac.h"
#include "vibration.h"

xTaskHandle xRainbowHandle,xLibrationHandle,xBlinkHandle,xVibrationHandle,xRandomHandle;

struct led_channel hsv_channels[LED_CHNL_NUM]=
		{
				{5,255,250},
				{10,255,250},
				{20,255,250},
				{30,255,250}
		};
const unsigned int rainbow_color_val[7]={0,7,32,100,190,240,280};

void Effects_Init(void)
{
	xTaskCreate(Libration_Process,(signed char*)"Libration",64,NULL, tskIDLE_PRIORITY + 1, &xLibrationHandle);
	vTaskSuspend(xLibrationHandle);
	xTaskCreate(Rainbow_Process,(signed char*)"Rainbow",64,NULL, tskIDLE_PRIORITY + 1, &xRainbowHandle);
	//vTaskSuspend(xRainbowHandle);
	xTaskCreate(Blink_Process,(signed char*)"Blink",64,NULL, tskIDLE_PRIORITY + 1, &xBlinkHandle);
	vTaskSuspend(xBlinkHandle);
	Random_Init();

	xTaskCreate(Random_Process,(signed char*)"RAND",64,NULL, tskIDLE_PRIORITY + 1, &xRandomHandle);
	vTaskSuspend(xRandomHandle);
}

void Rainbow_Process( void *pvParameters ) //
{
	unsigned char i=0;
	while(1)
	{
		 vTaskDelay(20);
		for(i=0;i<4;i++)
		{
			HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
			hsv_channels[i].hue++;
			if(hsv_channels[i].hue>355)
				hsv_channels[i].hue=0;
		}
	}
}



void Blink_Process( void *pvParameters ) //
{
	unsigned char i=0;
	unsigned int  j=0;
	while(1)
	{
		if(i<6)
		{
			for(j=rainbow_color_val[i];j<rainbow_color_val[i+1];j++)
			{
				vTaskDelay(30);
				if(i==0)
				{
					vTaskDelay(80);
				}
				hsv_channels[0].hue=j;
				HSVtoRGB(1,hsv_channels[0].hue,255,hsv_channels[0].volume);
				HSVtoRGB(2,hsv_channels[0].hue,255,hsv_channels[1].volume);
				HSVtoRGB(3,hsv_channels[0].hue,255,hsv_channels[2].volume);
				HSVtoRGB(4,hsv_channels[0].hue,255,hsv_channels[3].volume);
			}
		}
		else
		{
			for(j=rainbow_color_val[i];j<359;j++)
			{
				vTaskDelay(30);
				hsv_channels[0].hue=j;
				HSVtoRGB(1,hsv_channels[0].hue,255,hsv_channels[0].volume);
				HSVtoRGB(2,hsv_channels[0].hue,255,hsv_channels[1].volume);
				HSVtoRGB(3,hsv_channels[0].hue,255,hsv_channels[2].volume);
				HSVtoRGB(4,hsv_channels[0].hue,255,hsv_channels[3].volume);
			}
		}
		i++;
		if(i>=7)
			i=0;

		vTaskDelay(6000);

	}
}

void Libration_Process( void *pvParameters ) //
{
	unsigned char i=0,j=0,k=0;

	struct led_channel hsv_channels_lib[LED_CHNL_NUM]=
			{
					{100,155,149},
					{110,175,10},
					{120,200,20},
					{130,240,149}
			};

	unsigned char vol_flags[]={0,0,0,0};
	unsigned char hue_flags[]={0,0,0,0};
	unsigned char sat_flags[]={0,0,0,0};
	while(1)
	{



		 vTaskDelay(20);
		for(i=0;i<4;i++)
		{
			HSVtoRGB(i+1,hsv_channels_lib[i].hue,hsv_channels_lib[i].saturation,hsv_channels_lib[i].volume);

			if(hue_flags[i]==0)
			{
				hsv_channels_lib[i].hue++;
				if(hsv_channels_lib[i].hue==150)
				{
					hue_flags[i]=1;
				}
			}
			else
			{
				hsv_channels_lib[i].hue--;
				if(hsv_channels_lib[i].hue==0)
				{
					hue_flags[i]=0;
				}
			}


			if(k==3)
			{
				k=0;
				if(sat_flags[i]==0)
				{
					hsv_channels_lib[i].saturation++;
					if(hsv_channels_lib[i].saturation==255)
					{
						sat_flags[i]=1;
					}
				}
				else
				{
					hsv_channels_lib[i].saturation--;
					if(hsv_channels_lib[i].saturation==120)
					{
						sat_flags[i]=0;
					}
				}
			}
			k++;



			if(j==2)
			{
				j=0;
				if(vol_flags[i]==0)
				{
					hsv_channels_lib[i].volume++;
					if(hsv_channels_lib[i].volume==255)
					{
						vol_flags[i]=1;
					}
				}
				else
				{
					hsv_channels_lib[i].volume--;
					if(hsv_channels_lib[i].volume==50)
					{
						vol_flags[i]=0;
					}
				}

			}
			j++;

		}
	}
}

void Set_Color(unsigned char color)
{
	unsigned char i=0;
	if(color>6)
	{
		color=6;
	}
	for(i=0;i<4;i++)
	{
		hsv_channels[i].hue=rainbow_color_val[color];
		hsv_channels[i].saturation=255;
		HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
	}
}

void Set_White(void)
{
	unsigned char i=0;
	for(i=0;i<4;i++)
	{
	//	hsv_channels[i].hue=0;
	//	hsv_channels[i].saturation=0;
		//hsv_channels[i].volume=255;
		hsv_channels[i].saturation=0;
		HSVtoRGB(i+1,0,hsv_channels[i].saturation,hsv_channels[i].volume);
	}
	/*HSVtoRGB(1,0,50,255);
	HSVtoRGB(2,40,0,200);
	HSVtoRGB(3,0,50,200);
	HSVtoRGB(4,40,0,255);*/
}
//------------------------------------------------------------
void Random_Init(void)//ГСЧ, использующий ЦАП
{
/*	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

	DAC_InitTypeDef DAC_InitStructure;

    DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software;
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_Noise;
   // DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude
    //        = DAC_TriangleAmplitude_2047;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);*/

	RCC->APB1ENR |= RCC_APB1ENR_DACEN;      // вкл. тактирование ЦАП
	DAC->CR |= DAC_CR_TSEL1;
    DAC->CR |= DAC_CR_TEN1; // включение запуска от события, по умолчанию таймер 6
    DAC->CR |= DAC_CR_MAMP1;        // амплитуда на максимум
    DAC->CR |= DAC_CR_WAVE1_0;      // вкл. генератор шума

}

#define COLOR_DIF	100
void Random_Process( void *pvParameters ) //
{
	unsigned char i=0;
	unsigned char hue0[4],hue1[4];
	while(1)
	{

		for(i=0;i<4;i++)
		{
			DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
			__ASM volatile ("nop");
			__ASM volatile ("nop");
			__ASM volatile ("nop");
			__ASM volatile ("nop");
			__ASM volatile ("nop");
			hue1[i]=DAC->DOR1>>3;

			if(hue1[i]>359)
			{
				i--;
				continue;
			}

			if(i!=0)
			{
				if(hue1[i]>hue1[0]+COLOR_DIF || hue1[i]<hue1[0]-COLOR_DIF)
				{
					i--;
					continue;
				}
			}
		}



		while(hue0[0]!=hue1[0] && hue0[1]!=hue1[1] && hue0[2]!=hue1[2] && hue0[3]!=hue1[3])
		{
			vTaskDelay(30);
			for(i=0;i<4;i++)
			{

				if(hue0[i]<hue1[i])
				{
					hsv_channels[i].hue=hue0[i]++;
					HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
				}
				else
				{
					hsv_channels[i].hue=hue0[i]--;
					HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
				}

			}
		}

		vTaskDelay(1000);
	}
}

void Set_Random_Color(void)
{
	unsigned int color=1000;
	unsigned char i=0;
	while(color>359)
	{
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
		__ASM volatile ("nop");
		__ASM volatile ("nop");
		__ASM volatile ("nop");
		__ASM volatile ("nop");
		__ASM volatile ("nop");

		color=DAC->DOR1>>3;
	}

	for(i=0;i<4;i++)
	{
		hsv_channels[i].hue=color;
		HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
	}
}
