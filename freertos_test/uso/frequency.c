#include "frequency.h"
#include "channels.h"
 extern struct Channel  channels[CHANNEL_NUMBER];//обобщенная структура каналов
//uint32_t counter=0x80008000;
 volatile uint16_t capture1 = 0, capture2 = 0;
 volatile uint16_t capture11 = 0, capture12 = 0;
uint8_t capture_is_ready=0;
uint8_t capture_is_first = 0;

uint8_t capture_is_ready2=0;
uint8_t capture_is_first2 = 0;

void TIM4_IRQHandler(void)
{
/*	if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)
	  {
	     Даём знать, что обработали прерывание
	    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);

	     Запоминаем предыдущее измерение и считываем текущее
	    capture1 = capture2;
	    capture2 = TIM_GetCapture1(TIM4);

	     Для корректной обработки нужно минимум два измерения
	    if (!capture_is_first)
	      capture_is_ready = 1;

	    capture_is_first = 0;

	     Тут как-нибудь обрабатываем событие over-capture, если провороним
	    if (TIM_GetFlagStatus(TIM4, TIM_FLAG_CC1OF) != RESET)
	    {
	      TIM_ClearFlag(TIM4, TIM_FLAG_CC1OF);
	      // ...
	    }
	  }*/

	if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)
	  {
	    /* Даём знать, что обработали прерывание */
	    TIM_ClearITPendingBit(TIM4, TIM_IT_CC2);

	    /* Запоминаем предыдущее измерение и считываем текущее */
	    capture11 = capture12;
	    capture12 = TIM_GetCapture2(TIM4);

	    /* Для корректной обработки нужно минимум два измерения */
	    if (!capture_is_first2)
	      capture_is_ready2 = 1;

	    capture_is_first2 = 0;

	    /* Тут как-нибудь обрабатываем событие over-capture, если провороним */
	    if (TIM_GetFlagStatus(TIM4, TIM_FLAG_CC2OF) != RESET)
	    {
	      TIM_ClearFlag(TIM4, TIM_FLAG_CC2OF);
	      // ...
	    }
	  }
}

void Frequency_Init(void)//инициализация таймера дола
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//тактируем порт B
	  /* Подаём такты на TIM3 */
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	  /* Настраиваем предделитель так, чтобы таймер считал миллисекунды.
	     На бо́льших частотах следите, чтобы предделитель не превысил
	     максимальное значение uint16_t - 0xFFFF (65535) */
	  TIM_TimeBaseInitTypeDef timer_base;
	  TIM_TimeBaseStructInit(&timer_base);
	  timer_base.TIM_Prescaler = 2400 - 1;
	  TIM_TimeBaseInit(TIM4, &timer_base);

	  /* Настраиваем захват сигнала:
	   - канал: 1
	   - счёт: по нарастанию
	   - источник: напрямую со входа
	   - делитель: отключен
	   - фильтр: отключен */
	  TIM_ICInitTypeDef timer_ic;
/*	  timer_ic.TIM_Channel = TIM_Channel_1;
	  timer_ic.TIM_ICPolarity = TIM_ICPolarity_Rising;
	  timer_ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
	  timer_ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	  timer_ic.TIM_ICFilter = 0;
	  TIM_ICInit(TIM4, &timer_ic);

	   Разрешаем таймеру генерировать прерывание по захвату
	  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);*/

	  //TIM_ICInitTypeDef timer_ic;
	  timer_ic.TIM_Channel = TIM_Channel_2;
	  timer_ic.TIM_ICPolarity = TIM_ICPolarity_Rising;
	  timer_ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
	  timer_ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	  timer_ic.TIM_ICFilter = 0;
	  TIM_ICInit(TIM4, &timer_ic);

	  /* Разрешаем таймеру генерировать прерывание по захвату */
	  TIM_ITConfig(TIM4, TIM_IT_CC2, ENABLE);
	  /* Включаем таймер */
	  TIM_Cmd(TIM4, ENABLE);
	  /* Разрешаем прерывания таймера TIM4 */
	  NVIC_EnableIRQ(TIM4_IRQn);


	    //настройка прерывания
/*	    NVIC_InitTypeDef NVIC_InitStructure;
	    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 14;
	    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	    NVIC_Init(&NVIC_InitStructure);*/

	    //настройка пинов микроконтроллера
	    GPIO_InitTypeDef  GPIO_InitStructure;

	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOB, &GPIO_InitStructure);

	    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_Init(GPIOB, &GPIO_InitStructure);

	   xTaskCreate(Frequency_Process,(signed char*)"FREQUENCY_PROCESS",32,
	           NULL, tskIDLE_PRIORITY + 1, NULL);

}

void Frequency_Process( void *pvParameters )//процесс обновления регистра дола-необязательный
{
	while(1)
	{
	    if (capture_is_ready)
	    {
	      NVIC_DisableIRQ(TIM4_IRQn);
	      capture_is_ready = 0;

		channels[8].channel_data=10000/uint16_time_diff(capture2, capture1);

		//channels[9].channel_data=duty_cycle_capture;
		 NVIC_EnableIRQ(TIM4_IRQn);
	    }

	    if (capture_is_ready2)
	    {
	      NVIC_DisableIRQ(TIM4_IRQn);
	      capture_is_ready2 = 0;

		channels[9].channel_data=10000/uint16_time_diff(capture12, capture11);

		//channels[9].channel_data=duty_cycle_capture;
		 NVIC_EnableIRQ(TIM4_IRQn);
	    }
		vTaskDelay(50);
	}
}

uint16_t uint16_time_diff(uint16_t now, uint16_t before)
{
  return (now >= before) ? (now - before) : (UINT16_MAX - before + now);
}
