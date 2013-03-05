#include "ir_control.h"
#include "PWM.h"
#include "vibration.h"
#include "led_effects.h"

unsigned char ir_buffer[8]={0,0,0,0,0,0,0,0};
unsigned char ir_byte_counter=0;
unsigned char ir_bit_counter=0;

unsigned char ir_address=0;
unsigned char ir_code=0xF;

xSemaphoreHandle xIR_Semaphore;
extern xTaskHandle xRainbowHandle,xLibrationHandle,xBlinkHandle,xVibrationHandle,xRandomHandle;
extern struct led_channel hsv_channels[LED_CHNL_NUM];

void IR_Control_Init(void)//инициализация IR управления пультом
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_InitTypeDef  GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;/*GPIO_Mode_AF_PP GPIO_Mode_IN_FLOATING;*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	//------------------------------------------------------------------------
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);



    GPIO_PinRemapConfig( GPIO_Remap_TIM15, ENABLE);

    TIM_TimeBaseInitTypeDef timer_base;
    TIM_TimeBaseStructInit(&timer_base);
    timer_base.TIM_Prescaler = 240 - 1;
    TIM_TimeBaseInit(TIM15, &timer_base);

    TIM_ICInitTypeDef timer_ic;
    timer_ic.TIM_Channel = TIM_Channel_1;
    timer_ic.TIM_ICPolarity =TIM_ICPolarity_Rising;
    timer_ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
    timer_ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    timer_ic.TIM_ICFilter = 0;

    /* Эта функция настроит канал 1 для захвата периода,
       а канал 2 - для захвата заполнения. */
    TIM_PWMIConfig(TIM15, &timer_ic);
    /* Выбираем источник для триггера: вход 1 */
    TIM_SelectInputTrigger(TIM15, TIM_TS_TI1FP1);
    /* По событию от триггера счётчик будет сбрасываться. */
    TIM_SelectSlaveMode(TIM15, TIM_SlaveMode_Reset);
    /* Включаем события от триггера */
    TIM_SelectMasterSlaveMode(TIM15, TIM_MasterSlaveMode_Enable);


    TIM_Cmd(TIM15, ENABLE);


	xTaskCreate(IR_Process,(signed char*)"IR",64,NULL, tskIDLE_PRIORITY + 1, NULL);

	vSemaphoreCreateBinary(xIR_Semaphore);

	 TIM_ITConfig(TIM15, TIM_IT_CC1, ENABLE);
	 NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
}



void TIM1_BRK_TIM15_IRQHandler (void)
{
  unsigned int period_capture;
  static portBASE_TYPE xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;

  if (TIM_GetITStatus(TIM15, TIM_IT_CC1) != RESET)
  {
    TIM_ClearITPendingBit(TIM15, TIM_IT_CC1);
    NVIC_DisableIRQ(TIM1_BRK_TIM15_IRQn);

    /* параметры ШИМ, захваченные с канала 1 */
    period_capture = TIM_GetCapture1(TIM15);

    if(period_capture<START_MAX)
    {

    	if(period_capture>START_MIN)
    	{
    	    ir_buffer[0]=0;
    	    ir_buffer[1]=0;
    	    ir_buffer[2]=0;
    	    ir_buffer[3]=0;

    		ir_byte_counter=0;
    		ir_bit_counter=0;
    		NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
    	}
    	else
    	{
    		if(period_capture>BIT0_MAX)//bit1
    		{
    			ir_buffer[ir_byte_counter]|=(1<<ir_bit_counter);
    		}

    		ir_bit_counter++;

    		if(ir_bit_counter==8)
    		{
        		ir_byte_counter++;
        		ir_bit_counter=0;
    		}

    		if(ir_byte_counter==4)
    		{
    			ir_address=ir_buffer[0];
    			ir_code=ir_buffer[3];
        		ir_byte_counter=0;
        		ir_bit_counter=0;

    			xSemaphoreGiveFromISR( xIR_Semaphore, &xHigherPriorityTaskWoken );

  				 if( xHigherPriorityTaskWoken != pdFALSE )
  				 {
  					portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  				 }
    		}
    		else
    		{
    			NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
    		}
    	}
    }
    else
    {
    	NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
    }

   // NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);//!!!!
    if (TIM_GetFlagStatus(TIM15, TIM_FLAG_CC1OF) != RESET)
    {
      TIM_ClearFlag(TIM15, TIM_FLAG_CC1OF);
      // ...
    }
  }
  portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

//-------------------------------------------------------------------------
#define LONG_TIME 0xffff
void IR_Process( void *pvParameters ) //
{
	unsigned char i=0;
	unsigned char color=0;
	while(1)
	{
		if( xIR_Semaphore != NULL )
		{
			if( xSemaphoreTake( xIR_Semaphore, LONG_TIME ) == pdTRUE )
			{
				switch(ir_code)
				{
					case KEY_V_UP://ручная смена цвета
					{
						vTaskSuspend(xLibrationHandle);//остановим все задачи
						vTaskSuspend(xRainbowHandle);
						vTaskSuspend(xBlinkHandle);
						vTaskSuspend(xVibrationHandle);
						vTaskSuspend(xRandomHandle);

						color++;
						if(color==7)
						{
							color=0;
						}
						Set_Color(color);
						//выберем цвет из 7
					}
					break;

					case KEY_V_DOWN://режим вибросмены цвета
					{
						vTaskSuspend(xLibrationHandle);//остановим все задачи
						vTaskSuspend(xRainbowHandle);
						vTaskSuspend(xBlinkHandle);

						vTaskSuspend(xRandomHandle);

						vTaskResume(xVibrationHandle);

					}
					break;

					case KEY_3://Радуга
					{
						vTaskSuspend(xLibrationHandle);//остановим все задачи
						vTaskSuspend(xBlinkHandle);
						vTaskSuspend(xVibrationHandle);
						vTaskSuspend(xRandomHandle);

						vTaskResume(xRainbowHandle);
					}
					break;

					case KEY_4://случайная
					{
						vTaskSuspend(xLibrationHandle);//остановим все задачи
						vTaskSuspend(xRainbowHandle);
						vTaskSuspend(xBlinkHandle);
						vTaskSuspend(xVibrationHandle);

						vTaskResume(xRandomHandle);
					}
					break;

					case KEY_5://многопараметровая
					{
						//остановим все задачи
						vTaskSuspend(xRainbowHandle);
						vTaskSuspend(xBlinkHandle);
						vTaskSuspend(xVibrationHandle);
						vTaskSuspend(xRandomHandle);

						vTaskResume(xLibrationHandle);
					}
					break;

					case KEY_6://семицветная
					{
						vTaskSuspend(xLibrationHandle);//остановим все задачи
						vTaskSuspend(xRainbowHandle);
						vTaskSuspend(xVibrationHandle);
						vTaskSuspend(xRandomHandle);

						vTaskResume(xBlinkHandle);
					}
					break;

					case KEY_7://яркость выше
					{
						for(i=0;i<4;i++)
						{
							if((uint16_t)hsv_channels[i].volume+0xF<0xFF)
							{
								hsv_channels[i].volume+=0xF;
							}
							else
							{
								hsv_channels[i].volume=0xFF;
							}

							if(xTaskIsTaskSuspended(xLibrationHandle)==pdTRUE)
							{
								/*if((xTaskIsTaskSuspended(xRainbowHandle)==pdTRUE )&& (xTaskIsTaskSuspended(xBlinkHandle)==pdTRUE) && (xTaskIsTaskSuspended(xVibrationHandle)==pdTRUE) && (xTaskIsTaskSuspended(xRandomHandle)==pdTRUE))
								{
									Set_White();
								}
								else*/
								{
									HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
								}
							}
						}
					}
					break;

					case KEY_8://яркость ниже
					{
						for(i=0;i<4;i++)
						{
							if(hsv_channels[i].volume>=0xF)
							{
								hsv_channels[i].volume-=0xF;
							}
							else
							{
								hsv_channels[i].volume=0x0;
							}

							if(xTaskIsTaskSuspended(xLibrationHandle)==pdTRUE)
							{
								/*if((xTaskIsTaskSuspended(xRainbowHandle)==pdTRUE )&& (xTaskIsTaskSuspended(xBlinkHandle)==pdTRUE) && (xTaskIsTaskSuspended(xVibrationHandle)==pdTRUE) && (xTaskIsTaskSuspended(xRandomHandle)==pdTRUE))
								{
									Set_White();
								}
								else*/
								{
									HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
								}
							}
						}
					}
					break;

					case KEY_9://белый
					{
						vTaskSuspend(xLibrationHandle);//остановим все задачи
						vTaskSuspend(xRainbowHandle);
						vTaskSuspend(xBlinkHandle);
						vTaskSuspend(xVibrationHandle);
						vTaskSuspend(xRandomHandle);
						Set_White();
					}
					break;

					case KEY_0://выключить
					{
						/*for(i=0;i<4;i++)
						{
							HSVtoRGB(i+1,hsv_channels[i].hue,hsv_channels[i].saturation,hsv_channels[i].volume);
						}*/
					}
					break;
				}
				TIM_ClearITPendingBit(TIM15, TIM_IT_CC1);
				ir_code=0xF;
				NVIC_EnableIRQ(TIM1_BRK_TIM15_IRQn);
				 //xSemaphoreGive(xIR_Semaphore);
				//taskYIELD();
			}
		}
	}
}


