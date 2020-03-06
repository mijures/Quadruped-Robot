
/** 
* @brief        MY_Drivers����DBUS
* @details  �ο�WMDģ�黯ɢװ�⡪��Dbus��
* @author      mijures
* @date     2018.10.11
* @version  v0.1
* @par Copyright (c):      mijures
* @par 0������
* @par ʹ�÷�����1.��ͷ�ļ��и��Ķ�Ӧuart�ĺ궨�� 2.��DBUS_IT�ŵ���Ӧ�����жϺ����� 3.ʹ��DBUS_Init��ʼ��DBUS 
* @par ��������ʱ�������⻹Ҫע�Ⲩ���ʵ�����
* @par v0.1 �����˺���λ�ú�ͷ�ļ��Ĳ������ݣ�ʹ���ø��ӷ��㣬�����˱�׼��ע��
*/  


#include "DBUS.h"
#include "usart.h"
#include "cmsis_os.h"


unsigned char DBUS_rx_buffer[25]={0};//DBUS���ݻ���
RC_Ctl_t RC_Ctl; //�������ң����������
QueueHandle_t xDBUSSemaphore;
osThreadId DBUSHandle;



/** 
* @brief  DBUS���ݴ洢
* @details  �����ܵ������ݴ����洢��RC_Ctl��
* @param  NULL
* @retval  NULL
* @par log
*/
void StartDBUSTask(void const * argument)
{
	for(;;)
	{
		xSemaphoreTake(xDBUSSemaphore,portMAX_DELAY);	
		uint8_t i;
			for(i=0;i<18;i++)
			{
				if(DBUS_rx_buffer[i]==0)continue;
				break;
			}
			if(i!=18)//������ 
			{
					for(i=18;i<25;i++)//0���������
						{
							if(DBUS_rx_buffer[i]!=0)		//0��������18-24������������ ����/�������
							{
								for(i=18;i<25;i++)
								{
									DBUS_rx_buffer[i]=0;
								}
								i=0;
								break ;
							}
						}
					if(i==25)
					{
									uint8_t mouse_l_last=RC_Ctl.mouse.press_l;
									uint8_t mouse_r_last=RC_Ctl.mouse.press_r;
									uint16_t raw_last=RC_Ctl.key.raw;//�����ϴβ���
									//���ݴ洢
									RC_Ctl.rc.ch0 = (DBUS_rx_buffer[0]| (DBUS_rx_buffer[1] << 8)) & 0x07ff; //!< Channel 0
									RC_Ctl.rc.ch1 = ((DBUS_rx_buffer[1] >> 3) | (DBUS_rx_buffer[2] << 5)) & 0x07ff; //!< Channel 1
									RC_Ctl.rc.ch2 = ((DBUS_rx_buffer[2] >> 6) | (DBUS_rx_buffer[3] << 2) | //!< Channel 2
									(DBUS_rx_buffer[4] << 10)) & 0x07ff;
									RC_Ctl.rc.ch3 = ((DBUS_rx_buffer[4] >> 1) | (DBUS_rx_buffer[5] << 7)) & 0x07ff; //!< Channel 3
									RC_Ctl.rc0.ch0=RC_Ctl.rc.ch0-1024;if((RC_Ctl.rc0.ch0>-5)&&(RC_Ctl.rc0.ch0<5))RC_Ctl.rc0.ch0=0;
									RC_Ctl.rc0.ch1=RC_Ctl.rc.ch1-1024;if((RC_Ctl.rc0.ch1>-5)&&(RC_Ctl.rc0.ch1<5))RC_Ctl.rc0.ch1=0;
									RC_Ctl.rc0.ch2=RC_Ctl.rc.ch2-1024;if((RC_Ctl.rc0.ch2>-5)&&(RC_Ctl.rc0.ch2<5))RC_Ctl.rc0.ch2=0;
									RC_Ctl.rc0.ch3=RC_Ctl.rc.ch3-1024;if((RC_Ctl.rc0.ch3>-5)&&(RC_Ctl.rc0.ch3<5))RC_Ctl.rc0.ch3=0;
									
									RC_Ctl.rc.s1 = ((DBUS_rx_buffer[5] >> 4)& 0x000C) >> 2; //!< Switch left
									RC_Ctl.rc.s2 = ((DBUS_rx_buffer[5] >> 4)& 0x0003); //!< Switch right
									RC_Ctl.mouse.x = DBUS_rx_buffer[6] | (DBUS_rx_buffer[7] << 8); //!< Mouse X axis
									RC_Ctl.mouse.y = DBUS_rx_buffer[8] | (DBUS_rx_buffer[9] << 8); //!< Mouse Y axis
									RC_Ctl.mouse.z = DBUS_rx_buffer[10] | (DBUS_rx_buffer[11] << 8); //!< Mouse Z axis
									RC_Ctl.mouse.press_l = DBUS_rx_buffer[12]; //!< Mouse Left Is Press ?
									RC_Ctl.mouse.press_r = DBUS_rx_buffer[13]; //!< Mouse Right Is Press ?
									RC_Ctl.key.raw = DBUS_rx_buffer[14] | (DBUS_rx_buffer[15] << 8); //!< KeyBoard value
									
									RC_Ctl.mouse.release_r_flag= !RC_Ctl.mouse.press_r && mouse_r_last;//��¼������Ҽ�����
									RC_Ctl.mouse.release_l_flag= !RC_Ctl.mouse.press_l && mouse_l_last;
									RC_Ctl.mouse.press_l_flag= RC_Ctl.mouse.press_l && (!mouse_l_last);
									RC_Ctl.mouse.press_r_flag= RC_Ctl.mouse.press_r && (!mouse_r_last);
									
									RC_Ctl.key.press_flag = ~RC_Ctl.key.raw & (RC_Ctl.key.raw^raw_last);//��¼���²���
									RC_Ctl.key.release_flag = RC_Ctl.key.raw & (RC_Ctl.key.raw^raw_last);//��¼�ͷŲ���
					}
			}	
	}
}

/** 
* @brief  �������¶������
* @details  
* @param   KeyValue ����ֵ
* @retval  1 ���¶��� 0 û�а��¶���
* @par 
*/
uint8_t IsPress(uint16_t KeyValue)
{
	if(RC_Ctl.key.press_flag & KeyValue)
	{
		RC_Ctl.key.press_flag &= ~KeyValue;//������־
		return 1;
	}else return 0;
}

/** 
* @brief  �����ɿ��������
* @details  This is the detail description. 
* @param    KeyValue ����ֵ
* @retval  1 �ɿ����� 0 û���ɿ�����
* @par log
*/
uint8_t IsRelease(uint16_t KeyValue)
{
	if(RC_Ctl.key.release_flag & KeyValue)
	{
		RC_Ctl.key.release_flag &= ~KeyValue;//������־
		return 1;
	}else return 0;
}

/** 
* @brief  ����״̬���
* @details  This is the detail description. 
* @param   KeyValue ����ֵ
* @retval  1 ���� 0 �ɿ�
* @par log
*/
uint16_t IsSet(uint16_t KeyValue)
{
	return (RC_Ctl.key.raw&KeyValue)>0;
}

/** 
* @brief  DBUS��ʼ��
* @details  This is the detail description. 
* @param   NULL
* @retval  NULL
* @par log
*/
void DBUS_Init(void)
{
	vSemaphoreCreateBinary(xDBUSSemaphore);//������ֵ�ź������״δ����ź�������ֵΪ0
	while(!xDBUSSemaphore);
	osThreadDef(DBUSTask,StartDBUSTask,osPriorityRealtime,0,256);
	DBUSHandle = osThreadCreate(osThread(DBUSTask),NULL);
	__HAL_UART_ENABLE_IT(&DBUS,UART_IT_IDLE);
	HAL_UART_Receive_DMA(&DBUS,(uint8_t *)DBUS_rx_buffer,sizeof(DBUS_rx_buffer));
}


/** 
* @brief  DBUS�жϴ���
* @details  �յ����ݻ���жϣ������ݴ洢��Ctl�ṹ����,Ҫ���˺����Ž�HDBUS_IRQHandler��
* @param   NULL
* @retval  NULL
* @par log
*/
void DBUS_IT(void) 
{
	static BaseType_t xHigherPriorityTaskWoken;  //�����ȼ������Ƿ񱻻��ѵ�״̬����                  
	if(__HAL_UART_GET_FLAG(&DBUS,UART_FLAG_IDLE))
	{
		HAL_UART_DMAStop(&DBUS);
		__HAL_UART_CLEAR_IDLEFLAG(&DBUS);
		HAL_UART_Receive_DMA(&DBUS,DBUS_rx_buffer,25);
		 xSemaphoreGiveFromISR(xDBUSSemaphore,&xHigherPriorityTaskWoken);
		if( xHigherPriorityTaskWoken == pdTRUE )portEND_SWITCHING_ISR(xHigherPriorityTaskWoken); //��� xHigherPriorityTaskWoken = pdTRUE,���л�����ǰ�������ִ�У�Ȼ���˳��ж� } 
	}
}