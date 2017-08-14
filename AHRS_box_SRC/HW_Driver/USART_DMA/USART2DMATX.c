/*******************************************************************************
* �ļ���	  	 : USART2.c
* ����	         : USART����������
* ��ֲ����		 : �м�㺯��
* ����           : ��
* ���           : ��
* ����           : ��
*******************************************************************************/

#include "OSQMem.h"
#include "USART2DMATX.h"

#define USART2_SEND_MAX_Q	  	(OS_MEM_USART2_BLK-4)	//�����ڴ���ڵ����ռ�
#define USART2_SEND_MAX_BOX		20	   					//�����ڴ����������

unsigned char USART2SendQBuffer[USART2_SEND_MAX_BOX][USART2_SEND_MAX_Q];//�����ڴ��	
unsigned char USART2SendQBoxHost=0;			//�ڴ��ͷָ��							
unsigned char USART2SendQBoxTail=0;			//�ڴ��βָ��
unsigned int  USART2SendQFree=USART2_SEND_MAX_BOX;   
unsigned char USART2SendOVF=0; 				//USART2��������������־
unsigned char USART2RunningFlag=0;
typedef struct{
	unsigned char Num;
	unsigned char *Index;
}USART2SendTcb;

USART2SendTcb USART2SendTCB[USART2_SEND_MAX_BOX];

#define USART2_RECV_MAX_Q	  	32		//�ڴ���ڵ����ռ�
unsigned char USART2QRecvBuffer[USART2_RECV_MAX_Q];//�����ڴ��	
unsigned char USART2RecvOVF=0; 				//USART2��������������־  
unsigned char USART2RecvFlag=0;

extern u8 OSUSART2MemQ[OS_MEM_USART2_MAX];  			//�հ��ڴ��
extern OSMEMTcb* OSQUSART2Index;

void USART2DMAUpdate(void);
//������
#define ERR_NO_SPACE	0xff


void USART2DMAConfig(u8* TxBuffer1,u16 num)
{
    DMA_InitTypeDef     DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    /* DMA1 Channel4 (triggered by USART2 Tx event) Config */
    DMA_DeInit(DMA1_Stream6);
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(USART2->DR));
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)TxBuffer1;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = num;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//������byte
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);
	
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
	
	/* Enable the DMA1_Stream6 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_Cmd(DMA1_Stream6, ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);	 // ʹ�ܷ���DMA
	USART_Cmd(USART2, ENABLE);
}

/*******************************************************************************
* �ļ���	  	 : USART2WriteDataToBuffer
* ����	         : ��鷢�ͻ������Ĵ�С�����ռ��㹻���������͵����ݷ��뵽���ͻ���
				   ����ȥ,������������
* ����           : buffer�����͵����ݵ�ָ�룬count�����͵����ݵ�����
* ���           : ��
* ����           : ����ȷ���뵽���ͻ�������ȥ�ˣ��ͷ���0x00	 �����򷵻�0x01
*******************************************************************************/
unsigned char USART2WriteDataToBuffer(unsigned char *buffer,unsigned char count)
{
	unsigned char i=count;
	u8 err;
	/*�˴����Լ����źŵƻ��߹ر��ж�*/
	if(count==0) return 0;
	/*�������count��������Ҫ���ٸ��ڴ��*/
	if(count%USART2_SEND_MAX_Q)count=count/USART2_SEND_MAX_Q+1;
	else count=count/USART2_SEND_MAX_Q;
	/*��Ҫcount�����ݿ�*/
	/*����ڴ治�㣬ֱ�ӷ���*/		 
	if(USART2SendQFree<count){return ERR_NO_SPACE;}
	//���������ڴ�飬USART2SendQBoxHost����һ���ڴ������ż�һ
	USART2SendTCB[USART2SendQBoxHost].Index=(u8 *)OSMemGet(OSQUSART2Index,&err);
	if(USART2SendQBoxHost>=USART2_SEND_MAX_BOX)USART2SendQBoxHost=0;	
	count=0;
	while(i!='\0')										 
	{
		*(USART2SendTCB[USART2SendQBoxHost].Index+count)=*buffer;
		count++;
		if(count>=USART2_SEND_MAX_Q)
		{
			USART2SendTCB[USART2SendQBoxHost].Num=USART2_SEND_MAX_Q;
			//��Ҫһ���µ��ڴ���Ž����������ݣ����Ը���USART2SendQBoxHost
			if(++USART2SendQBoxHost>=USART2_SEND_MAX_BOX)USART2SendQBoxHost=0;
			//��Ҫһ���µ��ڴ���Ž�����������	
			USART2SendTCB[USART2SendQBoxHost].Index=(u8 *)OSMemGet(OSQUSART2Index,&err);
			//�յķ���������һ 			
			USART2SendQFree--;
			count=0;
		}
		buffer++;
		i--;
	}
	//�˴�����δ�����������ݣ�����ҲҪ�����һ���µ��ڴ����
	if(count!=0)
	{
		USART2SendTCB[USART2SendQBoxHost].Num=count; 
		USART2SendQFree--;
		if(++USART2SendQBoxHost>=USART2_SEND_MAX_BOX)USART2SendQBoxHost=0;
	}
	//����ǵ�һ�Σ����������ͣ�������Ѿ�������û�������Ҫ��
	if(USART2RunningFlag==0)
	{
#if	  	DMA_MODE
		USART2DMAConfig(USART2SendTCB[USART2SendQBoxTail].Index,USART2SendTCB[USART2SendQBoxTail].Num);
#else	
		USART2SendUpdate();
#endif		
		USART2RunningFlag=1;
	}

	return 0x00;
}

/*******************************************************************************
* �ļ���	  	 : USART2DispFun
* ����	         : ��鷢�ͻ������Ĵ�С�����ռ��㹻���������͵����ݷ��뵽���ͻ���
				   ����ȥ,������������,��USART2WriteDataToBuffer��ͬ���ǣ���������
				   ����������Ҫָ���ļ���С�ģ���͸������ṩ�˷���.
* ����           : buffer�����͵����ݵ�ָ��
* ���           : ��
* ����           : ����ȷ���뵽���ͻ�������ȥ�ˣ��ͷ���0x00	 �����򷵻�0x01
*******************************************************************************/
unsigned char USART2DispFun(unsigned char *buffer)
{
	unsigned long count=0;
	while(buffer[count]!='\0')count++;
	return(USART2WriteDataToBuffer(buffer,count));
}

/*******************************************************************************
* �ļ���	  	 : USART2DMAUpdate.c
* ����	         : USART_DMA����������
* ��ֲ����		 : �м�㺯��
* ����           : ��
* ���           : ��
* ����           : ��
*******************************************************************************/
void USART2DMAUpdate(void)
{
	if(USART2SendQBoxTail!=USART2SendQBoxHost)
	{
		OSMemDelete(OSQUSART2Index,USART2SendTCB[USART2SendQBoxTail].Index);
		if(++USART2SendQBoxTail>=USART2_SEND_MAX_BOX)USART2SendQBoxTail=0;
		if(++USART2SendQFree>=USART2_SEND_MAX_BOX)USART2SendQFree=USART2_SEND_MAX_BOX;
		if(USART2SendQBoxTail!=USART2SendQBoxHost)
		{
			USART2DMAConfig(USART2SendTCB[USART2SendQBoxTail].Index,USART2SendTCB[USART2SendQBoxTail].Num);	
		}
		else USART2RunningFlag=0;	
	}
	else 
	{		
		OSMemDelete(OSQUSART2Index,USART2SendTCB[USART2SendQBoxTail].Index);
		if(++USART2SendQBoxTail>=USART2_SEND_MAX_BOX)USART2SendQBoxTail=0;
		if(++USART2SendQFree>=USART2_SEND_MAX_BOX)USART2SendQFree=USART2_SEND_MAX_BOX;
		USART2RunningFlag=0;
	}	
}

//------------------End of File----------------------------
