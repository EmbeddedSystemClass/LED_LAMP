#include "proto.h"
#include "channels.h"
 extern struct Channel  channels[];//обобщенная структура каналов

//typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;
//-----------------------------------------------------------------------------------
unsigned char   DEV_NAME[DEVICE_NAME_LENGTH_SYM] ="<<uUSO_2>>"; //имя устройства
unsigned char   NOTICE[DEVICE_DESC_MAX_LENGTH_SYM]="<-- GEOSPHERA_2011 -->";//примечание
unsigned char   VERSION[DEVICE_VER_LENGTH_SYM] ="\x30\x30\x30\x30\x31";	// версия программы ПЗУ	не больше 5 байт

volatile  unsigned char   ADRESS_DEV=0x1;

unsigned char   dev_desc_len=20;//длина описания устройства
//--------------------------------global variable------------------------------------
//volatile unsigned char  	PROTO_STATE;//счетчик состояний
volatile unsigned char   	recieve_count;//счетчик приемного буфера
volatile unsigned char  	transf_count;//счетчик передаваемых байтов
volatile unsigned char  	buf_len;//длина передаваемого буфера

//------------------------флаги ошибок--------------------------------
volatile  unsigned char   crc_n_ERR;	//ошибка сrc
volatile  unsigned char   COMMAND_ERR;//неподдерживаемая команда

//volatile unsigned char   TIMEOUT;//таймаут

volatile  unsigned char   CUT_OUT_NULL;//флаг-вырезаем 0 после 0xD7
volatile  unsigned char   frame_len=0;//длина кадра, которую вытаскиваем из шестого байта кадра
//--------------------------------------------------------------------
volatile  unsigned char    RecieveBuf[MAX_LENGTH_REC_BUF]={0} ; //буфер принимаемых данных
volatile  unsigned char    *TransferBuf;
//static unsigned char /*data*/ volatile  TransferBuf[MAX_LENGTH_TR_BUF] ; //буфер передаваемых данных
//--------------------------------------------------------------------
volatile  unsigned char    STATE_BYTE=0xC0;//байт состояния устройства


//uint8_t led=0;
volatile unsigned int fr_err=0;

volatile unsigned char  symbol=0xFF;//принятый символ
//-----------------------------------------------------------------------------------
union //объединение для конвертирования char->long
{
	float result_float;
	unsigned char result_char[4];
}
sym_8_to_float;

xSemaphoreHandle xProtoSemaphore;


void USART1_IRQHandler (void)
{
 	static portBASE_TYPE xHigherPriorityTaskWoken;
 	  xHigherPriorityTaskWoken = pdFALSE;

//

 	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
   	{

 		USART_ClearITPendingBit(USART1, USART_IT_RXNE);


   		symbol=USART_ReceiveData (USART1);
   //----------------------обрабатываем возможные ошибки длины кадра-------------
   		if(recieve_count>MAX_LENGTH_REC_BUF)	//если посылка слишком длинная
   		{
   			recieve_count=0;

   			return;
   		}


   //--------------------------начало кадра...проверка до длины кадра--------
   	    if(recieve_count<6)
   		{
   				switch(recieve_count)
   				{
   					case  0:   //первый символ 0
   					{
   	 				 	 if(symbol!=0x00)
   						 {
   	 				 		recieve_count=0;

   	 				 	fr_err++;
   							//return;
   						 }
   					}
   					break;

   					case 1:	 //второй символ 0xD7
   					{
   						 if(symbol!=0xD7)
   						 {
   							recieve_count=0;

   							//return;
   						 }
   					}
   					break;

   					case 2:	 //	третий символ 0x29
   					{
   					 	 if(symbol!=0x29)
   						 {
   					 		recieve_count=0;

   							//return;
   						 }
   					}
   					break;

   					case 3:	//если адрес не совпал, то сбросим//NEW
   					{
   						if(symbol!=ADRESS_DEV)//если адрес совпал
   						{
   							recieve_count=0;

   							//return;
   						}
   					}
   					break;

   					default:  //
   					{
   					}
   					break;
   				}

   			RecieveBuf[recieve_count]=symbol;//сохраняем принятый символ в буфер
   			recieve_count++;//счетчик буфера

   			if(recieve_count==6)
   			{
   				frame_len=RecieveBuf[recieve_count-1]; //получим оставшуюся длину
   			}
   		}
   //---------------------------------------------------------
   		else  //отсюда знаем длину кадра и удаляем нули после 0xD7
   		{
   			switch(symbol)//проверим, это 0x0 ,0xD7 или другое
   			{
   				case 0xD7:
   				{
   					CUT_OUT_NULL=1;	//
   					RecieveBuf[recieve_count]=symbol;
   					recieve_count++;

   				}
   				break;

   				case 0x0:
   				{
   					if(!CUT_OUT_NULL)  //если вырезать 0x0 не надо, то не вырезаем
   					{
   						RecieveBuf[recieve_count]=symbol;
   						recieve_count++;
   					}
   					else //иначе в буфер не кладем, сбросим флаг
   					{
   						CUT_OUT_NULL=0;
   					}
   				}
   				break;

   				default:  //другие кладем в буфер
   				{
   					CUT_OUT_NULL=0;
   					RecieveBuf[recieve_count]=symbol;
   					recieve_count++;
   				}
   				break;
   			}

   			if(recieve_count>=frame_len+6)//если приняты  все байты
   			{
   				 USART_ITConfig(USART1, USART_IT_RXNE , DISABLE);
   				  /* 'Дать' семафор для разблокировки задачи. */

   				 xSemaphoreGiveFromISR( xProtoSemaphore, &xHigherPriorityTaskWoken );

   				 if( xHigherPriorityTaskWoken != pdFALSE )
   				 {
   					portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
   				 }
   			}
   		}

   	}
   //----------------------------передача----------------------------------------------------------------
   	if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
   	{

   		USART_ClearITPendingBit(USART1, USART_IT_TC);//очищаем признак прерывания

   		if(transf_count<buf_len)
   		{
   			if(transf_count<3)//передаем заголовок
   			{
   				USART_SendData(USART1,TransferBuf[transf_count]);
   				transf_count++;
   			}
   			else   //тело...   подставляем 0 после 0xD7
   			{
   					if(CUT_OUT_NULL==0)
   					{
   						if(TransferBuf[transf_count]==(unsigned char)0xD7)//проверим, это  ,0xD7 или другое
   						{
   							CUT_OUT_NULL=0x1;
   						}
   						USART_SendData(USART1,TransferBuf[transf_count]);
   						transf_count++;
   					}
   					else
   					{
   						USART_SendData(USART1,(unsigned char)0x0);
   						CUT_OUT_NULL=0;
   					}
   			}
   		}
   		else
   		{
   			transf_count=0;		//обнуляем счетчик
   			recieve_count=0;
   			//PROTO_STATE=PROTO_RESTART;
   			CUT_OUT_NULL=0;
   			 USART_ITConfig(USART1, USART_IT_RXNE , ENABLE);
   		}

   	}
   	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );

}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Proto_Init(void) //using 0
{
	//---------инициализация оборудовния----------------------------------
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA , ENABLE);//тактируем уарт

	//настраиваем контроллер прерываний
	 NVIC_InitTypeDef NVIC_InitStructure;
	 NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 14;
	  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 15;
	  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	  NVIC_Init(&NVIC_InitStructure);


	  //настраивае пины
	  GPIO_InitTypeDef GPIO_InitStructure;

	  // Tx on PD5 as alternate function push-pull
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);

	  /* Rx on PD6 as input floating */
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);


	USART_InitTypeDef USART_InitStructure;

		USART_DeInit(USART1);
		//настраиваем урат
	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ClearFlag(USART1, USART_FLAG_CTS | USART_FLAG_LBD  | USART_FLAG_TC  | USART_FLAG_RXNE );

	USART_ITConfig(USART1, USART_IT_TC, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE , ENABLE);

	USART_Cmd(USART1, ENABLE);





	//------------------------флаги ошибок--------------------------------

	crc_n_ERR=0x0;	//ошибка crc_n
	COMMAND_ERR=0x0;//неподдерживаемая команда

	TransferBuf=&RecieveBuf[0];	 //буфер ответа =буфер запроса
	ChannelsInit();


	//Restore_Dev_Address_Desc();

//	PROTO_STATE=PROTO_RESTART;//счетчик состояний
	recieve_count=0x0;//счетчик буфера приема
	transf_count=0x0;//счетчик передаваемых байтов
	buf_len=0x0;//длина передаваемого буфера
	//DE_RE=0;//линия на прием
	CUT_OUT_NULL=0;

	 xTaskCreate(ProtoProcess,(signed char*)"PROTO",64,
	            NULL, tskIDLE_PRIORITY + 1, NULL);

	vSemaphoreCreateBinary( xProtoSemaphore );
	return;
}
//-----------------------------------------------------------------------------
unsigned char Send_Info(void) //using 0    //посылка информации об устройстве
{
	    unsigned char    i=0;

	   //заголовок кадра---
	   TransferBuf[0]=0x00;
	   TransferBuf[1]=0xD7;
	   TransferBuf[2]=0x29;
	   //------------------
	   TransferBuf[3]=ADRESS_DEV;  // адрес узла
	   TransferBuf[4]=GET_DEV_INFO_RESP;  // код операции
	   TransferBuf[6]=STATE_BYTE;

	   for(i=0;i<20;i++)
	   {				  // записываем наименование изделия
			   if(i<DEVICE_NAME_LENGTH_SYM)
			   {
			     	TransferBuf[i+7]=DEV_NAME[i];
			   }
			   else
			   {
			   		TransferBuf[i+7]=0x00;
			   }
		}

	   for(i=0;i<5;i++)                   // записываем версию ПЗУ
	   {
	       if(i<DEVICE_VER_LENGTH_SYM)
		   {
		    	 TransferBuf[i+27]=VERSION[i];
		   }
	   }

	   TransferBuf[32]=CHANNEL_NUMBER;		   // количество каналов

	   for(i=0;i<CHANNEL_NUMBER;i++)				   // данные по каналу
       {
		  	TransferBuf[i*2+33]=((channels[i].settings.set.type)<<4)|channels[i].settings.set.modific; // байт данных
		  	TransferBuf[i*2+33+1]=0x00;							// резерв байт
	   }
	   for(i=0;i<dev_desc_len;i++)					// записываем примечание
	   {
			 TransferBuf[i+33+CHANNEL_NUMBER*2]=NOTICE[i];
	   }

	   TransferBuf[5]=28+CHANNEL_NUMBER*2+dev_desc_len;			// подсчет длины данных
	   TransferBuf[33+CHANNEL_NUMBER*2+dev_desc_len]=CRC_Check(&TransferBuf[1],32+CHANNEL_NUMBER*2+dev_desc_len); // подсчет контрольной суммы

	return (34+CHANNEL_NUMBER*2+dev_desc_len);
}
//-----------------------------------------------------------------------------
unsigned char Node_Full_Init(void) //using 0 //полная инициализация узла
{
	return 0;
}
//-----------------------------------------------------------------------------
unsigned char Channel_List_Init(void) //using 0 //Инициализация списка каналов узла (без потери данных);
{
	return 0;
}
//-----------------------------------------------------------------------------
unsigned char Channel_Get_Data(void) //using 0 //Выдать данные по каналам, согласно абсолютной нумерации;
{
	return 0;
}
//-----------------------------------------------------------------------------
unsigned char  Channel_Set_Parameters(void) //using 0 //Установить параметры по каналам, согласно абсолютной нумерации;
{
         unsigned char   index=0, store_data=0;//i=0;
//	LED=1;
	   while(index<RecieveBuf[5]-1)				   // данные по каналам
	      {
			  	if(RecieveBuf[6+index]<CHANNEL_NUMBER)
			    {
					switch((RecieveBuf[6+index+1]>>4)&0xF)
					{
					 		case 0x0://АЦП
							{
								if((channels[RecieveBuf[6+index]].settings.set.modific!=RecieveBuf[6+index+1])||(channels[RecieveBuf[6+index]].settings.set.state_byte_1!=RecieveBuf[6+index+2]) || (channels[RecieveBuf[6+index]].settings.set.state_byte_2!=RecieveBuf[6+index+3]))
								{
									channels[RecieveBuf[6+index]].settings.set.state_byte_1=RecieveBuf[6+index+2];
									channels[RecieveBuf[6+index]].settings.set.state_byte_2=RecieveBuf[6+index+3];
									channels[RecieveBuf[6+index]].settings.set.modific	   =RecieveBuf[6+index+1]&0xF;
									//debug----------------
									//if(channels[RecieveBuf[6+index]].settings.set.modific==3)
									{
										GPIOC->ODR |= GPIO_Pin_8;
									}
									//-----------------
									store_data=1;

								}
								index=index+1;
							}
							break;

							case 0x2://частотомер
							{
							/*   if(channels[RecieveBuf[6+index]].settings.set.state_byte_1!=RecieveBuf[6+index+2])
							   {
							   		channels[RecieveBuf[6+index]].settings.set.state_byte_1=RecieveBuf[6+index+2];
									store_data=1;
							   }*/
							}
							break;
					}
					index=index+3;
				}
				else
				{
					return Request_Error(FR_UNATTENDED_CHANNEL);
				}
		  }
	   if(store_data)
	   {
	   		Store_Channels_Data();	//сохраним настройки каналов в ППЗУ
		}
//	   LED=0;
	   return Request_Error(FR_SUCCESFUL);
}
//-----------------------------------------------------------------------------
unsigned char Channel_Set_Order_Query(void) //using 0 //Задать последовательность опроса;
{
	return 0;
}
//-----------------------------------------------------------------------------
unsigned char Channel_Get_Data_Order(void) //using 0 //Выдать данные по каналам, согласно последовательности опроса;
{
	return 0;
}
//-----------------------------------------------------------------------------
unsigned char Channel_Set_State(void) //using 0 //Установить состояния по каналам, согласно абсолютной нумерации;
{
	return 0;
}
//-----------------------------------------------------------------------------
unsigned char  Channel_Get_Data_Order_M2(void) //using 0 //Выдать данные по каналам, согласно последовательности опроса;
{
	return 0;
}
//-----------------------------------------------------------------------------
unsigned char Channel_Set_Reset_State_Flags(void) //using 0 //	Установка/Сброс флагов состояния
{
	STATE_BYTE=0x40;
	return	Request_Error(FR_SUCCESFUL);//ошибки нет, подтверждение
}
//-----------------------------------------------------------------------------
unsigned char Channel_All_Get_Data(void) //using 0 //Выдать информацию по всем каналам узла (расширенный режим);
{
	   unsigned char  index=0,i=0;


	   TransferBuf[0]=0x00;TransferBuf[1]=0xD7;TransferBuf[2]=0x29;
	   TransferBuf[3]=ADRESS_DEV;  //
	   TransferBuf[4]=CHANNEL_ALL_GET_DATA_RESP;  //
	   TransferBuf[6]=STATE_BYTE;
	    for(i=0;i<CHANNEL_NUMBER;i++)				   //
	    {
			  TransferBuf[index+7]=i;
			  index++;
			  TransferBuf[index+7]=((channels[i].settings.set.type)<<4)|channels[i].settings.set.modific; //
			  index++;
			  switch(channels[i].settings.set.type)
			    {
					 case 0:  //
					 {
						 switch(channels[i].settings.set.modific)
		                 {
							  case 0:
							  {
							  		if(channels[i].calibrate.cal.calibrate)//
									{
									//	 if(channels[i].settings.set.modific==0x00 || channels[i].settings.set.modific==0x01)
									//	 {
										 	TransferBuf[index+7]=((channels[i].channel_data_calibrate)&0x0000FF00)>>8;
										  	index++;
				    					  	TransferBuf[index+7]=((channels[i].channel_data_calibrate)&0x00FF0000)>>16;
										  	index++;
									//	 }
									}
									else
									{
										// if(channels[i].settings.set.modific==0x00 || channels[i].settings.set.modific==0x01)		   //
									//	 {
											TransferBuf[index+7]=((channels[i].channel_data)&0x0000FF00)>>8;
										  	index++;
				    					  	TransferBuf[index+7]=((channels[i].channel_data)&0x00FF0000)>>16;
										  	index++;
										// }
									}


									  TransferBuf[index+7]=channels[i].settings.set.state_byte_1;	 //
			                          index++;
			                          TransferBuf[index+7]=channels[i].settings.set.state_byte_2;	 //
				                      index++;
							  }
							  break;

							  case 1:
							  {
							  }
							  break;

				        	  case 2:
							  {
							  }
							  break;

							  case 3:
							  {
							        if(channels[i].calibrate.cal.calibrate)//
									{
							 			// if(channels[i].settings.set.modific==0x02 || channels[i].settings.set.modific==0x03)		   //
									//	 {
										  	TransferBuf[index+7]=((channels[i].channel_data_calibrate)&0x000000FF); //
								          	index++;
										  	TransferBuf[index+7]=((channels[i].channel_data_calibrate)&0x0000FF00)>>8;
										  	index++;
				    					  	TransferBuf[index+7]=((channels[i].channel_data_calibrate)&0x00FF0000)>>16;
										  	index++;
			  						  	// }
									}
									else
									{
									//	 if(channels[i].settings.set.modific==0x02 || channels[i].settings.set.modific==0x03)		   //
										// {
										  	TransferBuf[index+7]=((channels[i].channel_data)&0x000000FF); //
								          	index++;
										  	TransferBuf[index+7]=((channels[i].channel_data)&0x0000FF00)>>8;
										  	index++;
				    					  	TransferBuf[index+7]=((channels[i].channel_data)&0x00FF0000)>>16;
										  	index++;
									//	 }
									}


									  TransferBuf[index+7]=channels[i].settings.set.state_byte_1;	 //
			                          index++;
			                          TransferBuf[index+7]=channels[i].settings.set.state_byte_2;	 //
				                      index++;
							  }
							  break;
						  }
					  }
					  break;

				 	case 1:	 //ÄÎË
					{
						  switch(channels[i].settings.set.modific)
					      {
								  case 0:
								  {
								          TransferBuf[index+7]=((channels[i].channel_data)&0x000000FF); //
								          index++;

										  TransferBuf[index+7]=((channels[i].channel_data)&0x0000FF00)>>8;
										  index++;

								          TransferBuf[index+7]=((channels[i].channel_data)&0x00FF0000)>>16; //
								          index++;

										  TransferBuf[index+7]=((channels[i].channel_data)&0xFF000000)>>24;
										  index++;


										  TransferBuf[index+7]=channels[i].settings.set.state_byte_1;	 //
				                          index++;
								  }
								  break;
						   }
					}
					break;

					 case 2: //
					 {
						  switch(channels[i].settings.set.modific)
					      {
								  case 1:
								  {
								          TransferBuf[index+7]=((channels[i].channel_data)&0x000000FF); //
								          index++;
										  TransferBuf[index+7]=((channels[i].channel_data)&0x0000FF00)>>8;
										  index++;
										  TransferBuf[index+7]=channels[i].settings.set.state_byte_1;	 //
				                          index++;
								  }
								  break;
						   }
					  }
					  break;
			  }
		   }

		  TransferBuf[5]=index+2; 						 //
		  TransferBuf[index+7]=CRC_Check(&TransferBuf[1],(unsigned int)(index+7)-1); //
		  return (unsigned char)(7+index+1);
}

unsigned char Request_Error(unsigned char error_code) //using 0 //	Ошибочный запрос/ответ;
{
	TransferBuf[0]=0x00;TransferBuf[1]=0xD7;TransferBuf[2]=0x29;
    TransferBuf[3]=ADRESS_DEV;  // адрес узла
    TransferBuf[7]=RecieveBuf[4]; // код сбойной команды
    TransferBuf[4]=0xFF;  // код операции

	TransferBuf[6]=STATE_BYTE; // байт статуса узла
    TransferBuf[8]=error_code;
    TransferBuf[5]=0x04;	  // длина данных
    TransferBuf[9]=CRC_Check(TransferBuf,9);
	return 10;
}
//-----------------------------------------------------------------------------
void ProtoBufHandling(void) //using 0 //процесс обработки принятого запроса
{
  switch(RecieveBuf[4])
  {
//---------------------------------------
  	case GET_DEV_INFO_REQ:
	{
		buf_len=Send_Info();
	}
	break;
//---------------------------------------
  	case NODE_FULL_INIT_REQ:
	{
		buf_len=Node_Full_Init();
	}
	break;
//---------------------------------------
  	case CHANNEL_LIST_INIT_REQ:
	{
		buf_len=Channel_List_Init();
	}
	break;
//---------------------------------------
	case CHANNEL_GET_DATA_REQ:
	{
		buf_len=Channel_Get_Data();
	}
	break;
	//-----------------------------------
	case CHANNEL_SET_PARAMETERS_REQ:
	{
		buf_len=Channel_Set_Parameters();
	}
	break;
	//-----------------------------------
	case CHANNEL_SET_ORDER_QUERY_REQ:
	{
		buf_len=Channel_Set_Order_Query();
	}
	break;
//----------------------------------------
	case CHANNEL_GET_DATA_ORDER_REQ:
	{
		 buf_len=Channel_Get_Data_Order();
	}
	break;
//----------------------------------------
	case CHANNEL_SET_STATE_REQ:
	{
		 buf_len=Channel_Set_State();
	}
	break;
//----------------------------------------
	case CHANNEL_GET_DATA_ORDER_M2_REQ:
	{
		 buf_len=Channel_Get_Data_Order_M2();
	}
	break;
//------------------------------------------
	case CHANNEL_SET_RESET_STATE_FLAGS_REQ:
	{
		buf_len=Channel_Set_Reset_State_Flags();
	}
	break;
//------------------------------------------
	case CHANNEL_ALL_GET_DATA_REQ:
	{
		 buf_len=Channel_All_Get_Data();
	}
	break;
//------------------------------------------
/*	case CHANNEL_SET_ADDRESS_DESC:
	{
		 buf_len=Channel_Set_Address_Desc();
	}
	break;
//------------------------------------------
	case CHANNEL_SET_CALIBRATE:
	{
		 buf_len=Channel_Set_Calibrate();
	}
	break;*/
//------------------------------------------
    default:
	{
       COMMAND_ERR=0x1;//несуществующая команда
	   buf_len=Request_Error(FR_COMMAND_NOT_EXIST);
	  // PROTO_STATE=PROTO_ERR_HANDLING;//на обработчик ошибки
    }
  }

  return;
}
//-----------------------------------------------------------------------------------
void ProtoProcess( void *pvParameters ) //главный процесс
{
			unsigned char   crc_n;
	while(1)
	{
		if( xProtoSemaphore != NULL )
		{

			if( xSemaphoreTake( xProtoSemaphore, ( portTickType ) 10 ) == pdTRUE )
			{

				crc_n=RecieveBuf[recieve_count-1];
				transf_count=0;
				if(CRC_Check(&RecieveBuf,(recieve_count-CRC_LEN))==crc_n)
				{

					ProtoBufHandling();//процедура обработки сообщения

					transf_count=0;
					recieve_count=0;
					CUT_OUT_NULL=0;

					USART_SendData(USART1,TransferBuf[transf_count]);

					transf_count++;//инкрементируем счетчик переданных
				}
				else
				{
					crc_n_ERR=0x1;//несовпадение crc_n

				}

			}
		}
		//--------------------------------------------------
	}
}
//-----------------------crc_n------------------------------------------------------------
 unsigned char  CRC_Check( unsigned char  *Spool_pr,unsigned char Count_pr )  //проверить
{
	unsigned char crc_n = 0;
	unsigned char  *Spool;
	unsigned char  Count ;

	Spool=Spool_pr;
	Count=Count_pr;

  		while(Count!=0x0)
        {
	        crc_n = crc_n ^ (*Spool++);//
	        // циклический сдвиг вправо
	        crc_n = ((crc_n & 0x01) ? (unsigned char)0x80: (unsigned char)0x00) | (unsigned char)(crc_n >> 1);
	        // инверсия битов с 2 по 5, если бит 7 равен 1
	        if (crc_n & (unsigned char)0x80) crc_n = crc_n ^ (unsigned char)0x3C;
			Count--;
        }
    return crc_n;
}
 //-----------------------------------------------------------------------------------------------
/*
 void Store_Dev_Address_Desc(void)
 {

	 uint16_t i=0;
	 unsigned int blocks=0;
	 uint32_t EraseCounter = 0x00, Address = 0x00;
	 uint32_t NbrOfPage = 0x00;
	 volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
	 volatile TestStatus MemoryProgramStatus = PASSED;

	 FLASH_UnlockBank1();

	 Define the number of page to be erased
	NbrOfPage = (DESC_BANK1_WRITE_END_ADDR - DESC_BANK1_WRITE_START_ADDR) / FLASH_PAGE_SIZE;

	 Clear All pending flags
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	 Erase the FLASH pages
	for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
	{
		FLASHStatus = FLASH_ErasePage(DESC_BANK1_WRITE_START_ADDR + (FLASH_PAGE_SIZE * EraseCounter));
	}

	 Program Flash Bank1
	Address = DESC_BANK1_WRITE_START_ADDR;

	FLASH_ProgramWord(Address, (uint32_t)ADRESS_DEV);//запишем адрес
	Address = Address + 4;

	 for(i=0;i<(DEVICE_NAME_LENGTH>>2);i+=4)
	 {
	        FLASHStatus = FLASH_ProgramWord(Address,((uint32_t*)(&DEV_NAME))[i]);
	        Address = Address + 4;
	 }

	 for(i=0;i<(DEVICE_VER_LENGTH>>2);i+=4)
	 {
	        FLASHStatus = FLASH_ProgramWord(Address,((uint32_t*)(&VERSION))[i]);
	        Address = Address + 4;
	 }

	FLASH_ProgramWord(Address, (uint32_t)dev_desc_len);//запишем длину описания
	Address = Address + 4;

 	for(i=0;i<(dev_desc_len>>2+1);i++)
 	{
 		 FLASHStatus = FLASH_ProgramWord(Address,((uint32_t*)(&NOTICE))[i]);
 		Address+=4;
 	}
 	FLASH_LockBank1();
 //LED=0;
 	return;
 }
 //-----------------------------------------------------------------------------------------------
 void Restore_Dev_Address_Desc(void)//???
 {
 	unsigned int blocks=0,i=0;
 	uint32_t Address;
 	Address = DESC_BANK1_WRITE_START_ADDR;

 	ADRESS_DEV=(*(__IO uint32_t*) Address);
 	Address++;//прочитали адрес устройства, переходим на следующий

 	if(ADRESS_DEV<1 || ADRESS_DEV>15)
 		ADRESS_DEV=1;


 	for(i=0;i<(DEVICE_NAME_LENGTH>>2);i++)
 	{
 		((uint32_t*)(&DEV_NAME))[i]=(*(__IO uint32_t*) Address);
 		Address+=4;
 	}



 	for(i=0;i<(DEVICE_VER_LENGTH>>4);i++)
 	{
 		((uint32_t*)(&VERSION))[i]=(*(__IO uint32_t*) Address);
 		Address+=4;
 	}


 	dev_desc_len==(*(__IO uint32_t*) Address);
 	Address++;

 	if(dev_desc_len>DEVICE_DESC_MAX_LENGTH_SYM)
 		dev_desc_len=DEVICE_DESC_MAX_LENGTH_SYM;



 	for(i=0;i<(dev_desc_len>>2+1);i++)
 	{
 		((uint32_t*)(&NOTICE))[i]=(*(__IO uint32_t*) Address);
 		Address+=4;
 	}
 	return;
 }
*/
