/*******************************************************************************
* ����	         : USART����������
* ��ֲ����		 : �м�㺯��
* ����           : ��
* ���           : ��
* ����           : ��
*******************************************************************************/
#ifndef  _USART2DMA_H
#define  _USART2DMA_H

#include "stm32f4xx.h"

#define		DMA_MODE 		1 		//�����ǲ���DMAģʽ��������ͨ���ж�ģʽ

void USART2DMAUpdate(void);
unsigned char USART2WriteDataToBuffer(unsigned char *buffer,unsigned char count);
unsigned char USART1DispFun(unsigned char *buffer);

#endif

//------------------End of File----------------------------
