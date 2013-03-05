#include "PWM.h"
#include <math.h>
//-------------------------------------
#define CHANNELS_NUM 4
struct
{
	unsigned int hue;//оттенок
	unsigned char sat;//насыщенность
	unsigned char vol;//€ркость
}LED_CHANNELS[CHANNELS_NUM];
//-------------------------------------

void PWM_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//тактируем портј
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//тактируем портB

    //настройка пинов микроконтроллера
    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
//--------------------------------------------------------------------------------------
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//тактируем таймер
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);//тактируем таймер
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);//тактируем таймер
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);//тактируем таймер


	 	//настройка таймера
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseStructInit( &TIM_TimeBaseInitStruct );
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV4;
    TIM_TimeBaseInitStruct.TIM_Period = 256;
    TIM_TimeBaseInitStruct.TIM_Prescaler = 60;
    TIM_TimeBaseInit( TIM1, &TIM_TimeBaseInitStruct );
    TIM_TimeBaseInit( TIM2, &TIM_TimeBaseInitStruct );
    TIM_TimeBaseInit( TIM3, &TIM_TimeBaseInitStruct );
    TIM_TimeBaseInit( TIM4, &TIM_TimeBaseInitStruct );

    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCStructInit( &TIM_OCInitStruct );
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    // Initial duty cycle equals 0%. Value can range from zero to 1000.
    TIM_OCInitStruct.TIM_Pulse = 0;

/*    TIM_OC1Init( TIM1, &TIM_OCInitStruct );
    TIM_OC2Init( TIM1, &TIM_OCInitStruct );
    TIM_OC3Init( TIM1, &TIM_OCInitStruct );
    TIM_OC4Init( TIM1, &TIM_OCInitStruct );

    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);*/

/*    TIM_ARRPreloadConfig(TIM1, ENABLE);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);*/

    TIM_OC1Init( TIM2, &TIM_OCInitStruct );
    TIM_OC2Init( TIM2, &TIM_OCInitStruct );
    TIM_OC3Init( TIM2, &TIM_OCInitStruct );
    TIM_OC4Init( TIM2, &TIM_OCInitStruct );

    TIM_OC1Init( TIM3, &TIM_OCInitStruct );
    TIM_OC2Init( TIM3, &TIM_OCInitStruct );
    TIM_OC3Init( TIM3, &TIM_OCInitStruct );
    TIM_OC4Init( TIM3, &TIM_OCInitStruct );

    TIM_OC1Init( TIM4, &TIM_OCInitStruct );
    TIM_OC2Init( TIM4, &TIM_OCInitStruct );
    TIM_OC3Init( TIM4, &TIM_OCInitStruct );
    TIM_OC4Init( TIM4, &TIM_OCInitStruct );

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM4, ENABLE);

	//--------------------------------------------------------------
    //TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM4, ENABLE);

/*    TIM1->CCR1=20;
    TIM1->CCR2=40;
    TIM1->CCR3=60;
    TIM1->CCR4=80;*/

    TIM2->CCR1=10;
    TIM2->CCR2=10;
    TIM2->CCR3=10;
    TIM2->CCR4=10;

    TIM3->CCR1=10;
    TIM3->CCR2=10;
    TIM3->CCR3=10;
    TIM3->CCR4=10;

    TIM4->CCR1=10;
    TIM4->CCR2=10;
    TIM4->CCR3=10;
    TIM4->CCR4=10;

}
//----------------------------------------------------------------------------
void HSVtoRGB(unsigned int channel,unsigned int HUE, unsigned char SAT,unsigned char VOL) //функци€ преобразовани€ цветовой модели HSV в RGB.
    {
    int Hi, Vmin, Vdec, Vinc, alpha;
    float SL;
    unsigned char RED,GREEN,BLUE;

        if (HUE > 359) HUE = 359;
        if (SAT > 255) SAT = 255; //проерка переданных значений
        if (VOL > 255) VOL = 255;

        //переход от цветовой модели HSV к модели RGB. алгоритм описан в википедии
        SL=(float)SAT/255;                         //
    /*    Hi = (int)(HUE /= 60.0);            //
        alpha = (HUE-Hi);                   //
        Vmin = VOL*(1-SL);                  //
        Vdec = VOL*(1-(alpha*SL));          //
        Vinc = VOL*(1-(1-alpha)*SL);        //*/

        Hi = (int)(HUE /60.0);
        Vmin = VOL*(1-SL);
        alpha=(VOL-Vmin)*(HUE%60)/60;
        Vinc=Vmin+alpha;
        Vdec=VOL-alpha;

        switch (Hi)
            {
            case 0:                            //
                {
                RED=floor(VOL);
                GREEN=floor(Vinc);
                BLUE=floor(Vmin);
                }
            break;
            case 1:                            //
                {
                RED=floor(Vdec);
                GREEN=floor(VOL);
                BLUE=floor(Vmin);
                }
            break;
            case 2:                            //
                {
                RED=floor(Vmin);
                GREEN=floor(VOL);
                BLUE=floor(Vinc);
                }
            break;
            case 3:                            //
                {
                RED=floor(Vmin);
                GREEN=floor(Vdec);
                BLUE=floor(VOL);
                }
            break;
            case 4:                            //
                {
                RED=floor(Vinc);
                GREEN=floor(Vmin);
                BLUE=floor(VOL);
                }
            break;
            case 5:                            //
                {
                RED=floor(VOL);
                GREEN=floor(Vmin);
                BLUE=floor(Vdec);
                }
            }

       // RED=RED*1.8;
       // GREEN=GREEN*1.2;
        BLUE=BLUE*0.6;
        GREEN=GREEN*0.8;

        switch(channel)
        	{
				case 1:
				{
					CHANNEL1_RED=RED;
					CHANNEL1_GREEN=GREEN;
					CHANNEL1_BLUE=BLUE;
				}
				break;

				case 2:
				{
					CHANNEL2_RED=RED;
					CHANNEL2_GREEN=GREEN;
					CHANNEL2_BLUE=BLUE;
				}
				break;

				case 3:
				{
					CHANNEL3_RED=RED;
					CHANNEL3_GREEN=GREEN;
					CHANNEL3_BLUE=BLUE;
				}
				break;

				case 4:
				{
					CHANNEL4_RED=RED;
					CHANNEL4_GREEN=GREEN;
					CHANNEL4_BLUE=BLUE;
				}
				break;
		 }

    }

