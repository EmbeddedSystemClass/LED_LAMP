#include "dol.h"
#include "channels.h"
 extern struct Channel  channels[CHANNEL_NUMBER];//обобщенная структура каналов
uint32_t counter=0x80008000;

void TIM3_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
	  GPIOC->ODR ^= GPIO_Pin_9;
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

    if(TIM3->CR1 & TIM_CR1_DIR)
    	counter-=0x10000 ;
    else
    	counter+=0x10000;


  }
}

void DOL_Init(void)//инициализация таймера дола
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//тактируем портА
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_TIM3EN, ENABLE);//тактируем таймер

	 	//настройка таймера дола
		TIM_TimeBaseInitTypeDef timer_base;
	    TIM_TimeBaseStructInit(&timer_base);
	    timer_base.TIM_Period = 65535;
	    timer_base.TIM_CounterMode = TIM_CounterMode_Down | TIM_CounterMode_Up;
	    TIM_TimeBaseInit(TIM3, &timer_base);


	    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12,
	    TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);
	    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	    TIM_Cmd(TIM3, ENABLE);
	    //настройка прерывания дола
	    NVIC_InitTypeDef NVIC_InitStructure;
	    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 14;
	    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	    NVIC_Init(&NVIC_InitStructure);

	    //настройка пинов микроконтроллера
	    GPIO_InitTypeDef  GPIO_InitStructure;

	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);

	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOA, &GPIO_InitStructure);

	    xTaskCreate(DOL_Process,(signed char*)"DOL_PROCESS",32,
	            NULL, tskIDLE_PRIORITY + 1, NULL);

}

void DOL_Process( void *pvParameters )//процесс обновления регистра дола-необязательный
{
	while(1)
	{
		channels[11].channel_data=counter+TIM3->CNT;
		vTaskDelay(50);
	}
}
