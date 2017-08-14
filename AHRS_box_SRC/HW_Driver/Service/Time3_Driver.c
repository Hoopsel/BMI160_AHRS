/* Time3_Driver.c file
��д�ߣ�lisn3188
��ַ��www.chiplab7.com
����E-mail��lisn3188@163.com
���뻷����MDK-Lite  Version: 4.23
����ʱ��: 2012-04-25
���ԣ� ���������ڵ���ʵ���ҵ�mini IMU����ɲ���
���ܣ�
��ʱ�������жϵ����á��ͳ�ʼ������
------------------------------------
 */

#include "Time3_Driver.h"


/**************************ʵ�ֺ���********************************************
*����ԭ��:		void Tim3_NVIC_Init(void)
*��������:		 ��ʼ��������ʱ��4 	�ж� ����Ϊ������ȼ�����ж�
*******************************************************************************/
void Tim3_NVIC_Init(void) 
{ 
 	NVIC_InitTypeDef NVIC_InitStructure;
 	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;   
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
 	NVIC_Init(&NVIC_InitStructure); 
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void Time3_Inttrup_init(void)
*��������:		 ��ʼ����ʱ��3  ���ö�ʱ����ʱ��Ƶ��Ϊ 10K Hz
				������Tim3������жϣ����жϳ����и�������
*******************************************************************************/
void Time3_Inttrup_init(void)
{
	//ϵͳ��84Mhz  84M/��8399+1��=10000��
	//������װ����500��*10=5000/10000=0.05s
	//����һ����ʱ���Ľṹ�����
 	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//������ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
 	//������װ�ص�ֵ
 	TIM_TimeBaseStructure.TIM_Period = 10000;  
 	//Ԥ��Ƶ��ֵ������ֵ��1
 	TIM_TimeBaseStructure.TIM_Prescaler = 8399; 
 	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
 	//��������������
 	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	//���жϣ������жϡ�
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 

	TIM3->CR1 &=~0x01;    //�ض�ʱ��4 
	Tim3_NVIC_Init();  //�����ж����ȼ�
}							

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void Tim4_Set_Speed(u16 Speed)
*��������:		�����������Ƶ�ʣ�����Tim4�жϵ�Ƶ��
����   Ҫ���µ��ٶȣ���λ Hz
*******************************************************************************/
void Tim3_Set_Speed(u16 Speed)
{
	u16 New_arr;
	TIM3->CR1 &=~0x01;    //�ض�ʱ��4
	New_arr	= 10000/Speed - 1;
	TIM3->ARR = New_arr;  //д��������Զ���װֵ
	//TIM4->CR1 |= 0x01;    //ʹ�ܶ�ʱ��4
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void Tim4_Stop(void)
*��������:		�رն�ʱ��4��ʱ��
*******************************************************************************/
void Tim3_Stop(void)
{
	TIM3->CR1 &=~0x01;    //�ض�ʱ��4
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void Tim4_Restart(void)
*��������:		������ʱ��4��ʱ��
*******************************************************************************/
void Tim3_Restart(void)
{
	TIM3->CNT = 0;
	TIM3->CR1 |= 0x01;    //ʹ�ܶ�ʱ��4
}

//------------------End of File----------------------------
