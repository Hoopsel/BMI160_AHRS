/* KEY.C file
STM32-SDK �������������
��д�ߣ�lisn3188
��ַ��www.chiplab7.com
����E-mail��lisn3188@163.com
���뻷����MDK-Lite  Version: 4.23
����ʱ��: 2012-02-28
���ԣ� ���������ڵ���ʵ���ҵ�STM32-SDK����ɲ���
���ܣ�ʵ��	Captian ��LED��ʼ���Ͳ��� API

---------Ӳ���ϵ���������:----------

------------------------------------
 */

#include "LED.h"


//LED ���ȼ����
static int LightLevel[40]={0,0,0,0,0,1,1,2,4,8,16,32,50,64,80,100,100,120,140,180,180,140,120,100,100,80,64,50,32,16,8,4,2,1,1,0,0,0,0,0};
u8 lightc=0;

void Initial_PWM_LED(u32 arr,u32 psc);
/**************************ʵ�ֺ���********************************************
*����ԭ��:		void Initial_LED_GPIO(void)
*��������:		���� LED ��Ӧ�Ķ˿�Ϊ���
*******************************************************************************/
void Initial_LED_GPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  //ʹ��GPIOA ��ʱ��,
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC , ENABLE);
  //����PA8 Ϊ�������  ˢ��Ƶ��Ϊ2Mhz  
  GPIOB->AFR[0] &= 0xff000fff;	//����SWD
  GPIOB->AFR[0] |= 0x00033000;  //��ֹJTAG�ӿ�
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 ; // USB��������	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  //Ӧ�����õ�GPIOB 
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  USB_Disable();

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	   //BEEP
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	   //PC3 ��Դ��⡣�͵�ƽ�ػ���
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	   //PA0 ��������״̬����
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;	   //PB8 SD ���
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	   //PB10 ��ť ���
  GPIO_Init(GPIOB, &GPIO_InitStructure);

   //����LED �˿�����ߵ�ƽ, �ص�.
   Initial_PWM_LED(250,5);
   GPIO_SetBits(GPIOC, GPIO_Pin_6);
   BEEP_OFF();
}

uint32_t key_pressT;
uint8_t  key_staut = 0;
uint8_t  Key_IS_Press = 0;
uint32_t Beep_StartT,Beep_OnT = 0;

void Scan_Key_Routing(void){  //����ɨ��
	uint32_t temp;	
   if((GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10)==0x00)){ //�͵�ƽ
   		if(key_staut == 0){
		key_staut = 1;
		key_pressT = micros(); //��¼�ߵ�ƽ��ʱ��
		}
   }else{  //�ߵ�ƽ��
	 if(key_staut != 0){
	 	key_staut = 0;
		temp = micros();
		//����ʱ����200ms  - 1000ms֮����Ч
		if((1000000>(temp - key_pressT))&&((temp - key_pressT)>200000)){
			Key_IS_Press = 1;
		}
	 }

   }
//------------beep 
	if(Beep_OnT){
	temp = micros();	
	if((temp - Beep_StartT) > Beep_OnT){
		Beep_OnT = 0;
		BEEP_OFF();	
	}
	}
}

void Set_BeepON(uint16_t ON_ms){
	if(ON_ms == 0){
		Beep_OnT = 0;
		BEEP_OFF();
		return;
	}
	Beep_StartT = micros();
	Beep_OnT = (uint32_t)ON_ms * 1000;  // ms -> us
	BEEP_ON();
	lightc=0;
}



void SET_POWER_Down(void){
   GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  //Ӧ�����õ�GPIOC 
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_ResetBits(GPIOC, GPIO_Pin_3);	
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void LED_Reverse(void)
*��������:		LED ��ȡ��, ��,����ʱ���ö˿�ʹ֮ת����״̬,
								����ʱ���ö˿�ʹ֮ת����״̬.
*******************************************************************************/
u8 LED_ST = 0;
void LED_Reverse(void)
{
/*
	if(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_6))	
		GPIO_ResetBits(GPIOC, GPIO_Pin_6);
		else
		GPIO_SetBits(GPIOC, GPIO_Pin_6);  */
		if(LED_ST){
		   TIM8->CCR1 = 200;
		   LED_ST = 0;
		} else{
			TIM8->CCR1 = 0;
			LED_ST = 1;	
		} 
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void LED_Change(void)
*��������:		�ı�LED�����ȣ���	LightLevel ����
*******************************************************************************/
void LED_Change(void)
{
	TIM8->CCR1=LightLevel[lightc]; //����ͨ��1�ıȽ�ֵ
	if(++lightc==40)lightc=0;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void initial_Timer1(void)
*��������:		Timer1 ��ʼ�� 
*******************************************************************************/
void Initial_Timer1(void){
    
    //ϵͳ��84Mhz  84M/��5+1��=14000 000��
	//������װ����65536��
	//����һ����ʱ���Ľṹ�����
 	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); 
	//������ʱ��
 	//������װ�ص�ֵ
 	TIM_TimeBaseStructure.TIM_Period = 0xffff;  
 	//Ԥ��Ƶ��ֵ������ֵ��1
 	TIM_TimeBaseStructure.TIM_Prescaler = 5; //14M ��ʱ��	����ʱ4ms
 	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
 	//��������������
 	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
	//���жϣ������жϡ�
    //TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);     
	//ʹ�ܶ�ʱ��
   	TIM_Cmd(TIM1, ENABLE); 
}

double Get_Timer1_p(void){
	uint32_t temp=0 ;
	double tempf;
  	temp = TIM1->CNT;
	TIM1->CNT = 0;  //������ʱ��
  	tempf = ( (double)temp ) / 28000000.0f;
	return tempf;
}


void Initial_PWM_LED(u32 arr,u32 psc){

	//�˲������ֶ��޸�IO������
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);  	//TIM8ʱ��ʹ��   
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC , ENABLE);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;           //GPIOC6
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOC,&GPIO_InitStructure);              //��ʼ��PC6
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_TIM8); //GPIOC6����Ϊ��ʱ��8
  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=arr;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV4; 
	
	TIM_TimeBaseInit(TIM8,&TIM_TimeBaseStructure);//��ʼ����ʱ��8
	
	//��ʼ��TIM8 Channel1 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������
	TIM_OCInitStructure.TIM_Pulse = 20;	
	TIM_OC1Init(TIM8, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	//��ֹOC2 OC3���
	TIM_OC2Init(TIM8, &TIM_OCInitStructure);
	TIM_OC3Init(TIM8, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);  //ʹ��TIM8��CCR1�ϵ�Ԥװ�ؼĴ���
 
    TIM_ARRPreloadConfig(TIM8,ENABLE);//ARPEʹ�� 
	TIM_Cmd(TIM8, ENABLE);  //ʹ��TIM8
	TIM_CtrlPWMOutputs(TIM8, ENABLE); 
}


//------------------End of File----------------------------
