#include "vibration.h"
#include "led_effects.h"

#include "led_effects.h"


extern xTaskHandle xRainbowHandle,xLibrationHandle,xBlinkHandle,xVibrationHandle,xRandomHandle;

xSemaphoreHandle xVibro_Semaphore;

void Vibration_Init(void)
{
//-------------------------GPIO---------------------------
	   GPIO_InitTypeDef GPIO_InitStructure;

	   /* Configure PC.04 (ADC Channel14) as analog input -------------------------*/
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	   GPIO_Init(GPIOA, &GPIO_InitStructure);
//-------------------------NVIC---------------------------
	  NVIC_InitTypeDef NVIC_InitStructure;

	  /* Configure and enable ADC interrupt */
  //  #if defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
	  NVIC_InitStructure.NVIC_IRQChannel = ADC1_IRQn;
	//#else
	 // NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
   // #endif
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);
//--------------------------ADC---------------------------
		RCC_ADCCLKConfig(RCC_PCLK2_Div2);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

		ADC_InitTypeDef  ADC_InitStructure;
		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_InitStructure.ADC_ScanConvMode = DISABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
		ADC_InitStructure.ADC_NbrOfChannel = 1;
		ADC_Init(ADC1, &ADC_InitStructure);

		/* ADC1 regular channel14 configuration */
		ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_13Cycles5);

		/* Configure high and low analog watchdog thresholds */
		ADC_AnalogWatchdogThresholdsConfig(ADC1, /*0x0900*/0x0590, 0x0000);
		/* Configure channel14 as the single analog watchdog guarded channel */
		ADC_AnalogWatchdogSingleChannelConfig(ADC1, ADC_Channel_4);
		/* Enable analog watchdog on one regular channel */
		ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_SingleRegEnable);

		/* Enable AWD interrupt */
//		ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);

		/* Enable ADC1 */
		ADC_Cmd(ADC1, ENABLE);

		/* Enable ADC1 reset calibration register */
		ADC_ResetCalibration(ADC1);
		/* Check the end of ADC1 reset calibration register */
		while(ADC_GetResetCalibrationStatus(ADC1));

		/* Start ADC1 calibration */
		ADC_StartCalibration(ADC1);
		/* Check the end of ADC1 calibration */
		while(ADC_GetCalibrationStatus(ADC1));

		/* Start ADC1 Software Conversion */
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);

		xTaskCreate(Vibro_Process,(signed char*)"Vibro",64,NULL, tskIDLE_PRIORITY + 1, &xVibrationHandle);
		vTaskSuspend(xVibrationHandle);

		vSemaphoreCreateBinary(xVibro_Semaphore);
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
		ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);
//-------------------------------------------------------------------
}

//--------------------------------------------------------------------
 void ADC1_IRQHandler(void)
{
		static portBASE_TYPE xHigherPriorityTaskWoken;

		//ADC_ITConfig(ADC1, ADC_IT_AWD, DISABLE);

		if(ADC_GetITStatus(ADC1,ADC_IT_AWD)!= RESET)
		{
ADC1->CR1 &= (~(uint32_t)ADC_IT_AWD);

			xHigherPriorityTaskWoken = pdFALSE;

			xSemaphoreGiveFromISR( xVibro_Semaphore, &xHigherPriorityTaskWoken );


			 if( xHigherPriorityTaskWoken != pdFALSE )
			 {
				portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
			 }
			/* Clear ADC1 AWD pending interrupt bit */



			portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
		ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
}
//-------------------------------------------------------------------
#define LONG_TIME 0xffff

 void Vibro_Process( void *pvParameters ) //
 {
 	while(1)
 	{
 		if( xVibro_Semaphore != NULL)
 		{
 			if( xSemaphoreTake( xVibro_Semaphore, portMAX_DELAY ) == pdTRUE )
 			{
 				ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
 				vTaskDelay(500);
 				Set_Random_Color();
 				//vTaskDelay(1500);
 				ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
 				ADC_ITConfig(ADC1, ADC_IT_AWD, ENABLE);

 				//xSemaphoreGive(xVibro_Semaphore);
 				//taskYIELD();
 			}
 		}
 	}
 }
