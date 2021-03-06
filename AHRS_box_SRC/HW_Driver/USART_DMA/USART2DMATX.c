/*******************************************************************************
* 文件名	  	 : USART2.c
* 描述	         : USART的驱动函数
* 移植步骤		 : 中间层函数
* 输入           : 无
* 输出           : 无
* 返回           : 无
*******************************************************************************/

#include "OSQMem.h"
#include "USART2DMATX.h"

#define USART2_SEND_MAX_Q	  	(OS_MEM_USART2_BLK-4)	//发送内存块内的最大空间
#define USART2_SEND_MAX_BOX		20	   					//发送内存块的最大数量

unsigned char USART2SendQBuffer[USART2_SEND_MAX_BOX][USART2_SEND_MAX_Q];//发送内存块	
unsigned char USART2SendQBoxHost=0;			//内存块头指针							
unsigned char USART2SendQBoxTail=0;			//内存块尾指针
unsigned int  USART2SendQFree=USART2_SEND_MAX_BOX;   
unsigned char USART2SendOVF=0; 				//USART2发送任务块溢出标志
unsigned char USART2RunningFlag=0;
typedef struct{
	unsigned char Num;
	unsigned char *Index;
}USART2SendTcb;

USART2SendTcb USART2SendTCB[USART2_SEND_MAX_BOX];

#define USART2_RECV_MAX_Q	  	32		//内存块内的最大空间
unsigned char USART2QRecvBuffer[USART2_RECV_MAX_Q];//接收内存块	
unsigned char USART2RecvOVF=0; 				//USART2接收任务块溢出标志  
unsigned char USART2RecvFlag=0;

extern u8 OSUSART2MemQ[OS_MEM_USART2_MAX];  			//空白内存块
extern OSMEMTcb* OSQUSART2Index;

void USART2DMAUpdate(void);
//错误定义
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
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//这里是byte
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
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);	 // 使能发送DMA
	USART_Cmd(USART2, ENABLE);
}

/*******************************************************************************
* 文件名	  	 : USART2WriteDataToBuffer
* 描述	         : 检查发送缓冲区的大小，若空间足够，将待发送的数据放入到发送缓冲
				   区中去,并且启动发送
* 输入           : buffer待发送的数据的指针，count待发送的数据的数量
* 输出           : 无
* 返回           : 若正确放入到发送缓冲区中去了，就返回0x00	 ，否则返回0x01
*******************************************************************************/
unsigned char USART2WriteDataToBuffer(unsigned char *buffer,unsigned char count)
{
	unsigned char i=count;
	u8 err;
	/*此处可以加入信号灯或者关闭中断*/
	if(count==0) return 0;
	/*计算放入count个数据需要多少个内存块*/
	if(count%USART2_SEND_MAX_Q)count=count/USART2_SEND_MAX_Q+1;
	else count=count/USART2_SEND_MAX_Q;
	/*需要count个数据块*/
	/*如果内存不足，直接返回*/		 
	if(USART2SendQFree<count){return ERR_NO_SPACE;}
	//首先申请内存块，USART2SendQBoxHost在下一个内存申请后才加一
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
			//需要一个新的内存块存放接下来的数据，所以更新USART2SendQBoxHost
			if(++USART2SendQBoxHost>=USART2_SEND_MAX_BOX)USART2SendQBoxHost=0;
			//需要一个新的内存块存放接下来的数据	
			USART2SendTCB[USART2SendQBoxHost].Index=(u8 *)OSMemGet(OSQUSART2Index,&err);
			//空的发送任务块减一 			
			USART2SendQFree--;
			count=0;
		}
		buffer++;
		i--;
	}
	//此处是尚未整块存完的数据，它们也要存放在一个新的内存块里
	if(count!=0)
	{
		USART2SendTCB[USART2SendQBoxHost].Num=count; 
		USART2SendQFree--;
		if(++USART2SendQBoxHost>=USART2_SEND_MAX_BOX)USART2SendQBoxHost=0;
	}
	//如果是第一次，就启动发送，如果是已经启动就没有这个必要了
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
* 文件名	  	 : USART2DispFun
* 描述	         : 检查发送缓冲区的大小，若空间足够，将待发送的数据放入到发送缓冲
				   区中去,并且启动发送,与USART2WriteDataToBuffer不同的是，启动发送
				   函数世不需要指定文件大小的，这就给调用提供了方便.
* 输入           : buffer待发送的数据的指针
* 输出           : 无
* 返回           : 若正确放入到发送缓冲区中去了，就返回0x00	 ，否则返回0x01
*******************************************************************************/
unsigned char USART2DispFun(unsigned char *buffer)
{
	unsigned long count=0;
	while(buffer[count]!='\0')count++;
	return(USART2WriteDataToBuffer(buffer,count));
}

/*******************************************************************************
* 文件名	  	 : USART2DMAUpdate.c
* 描述	         : USART_DMA的驱动函数
* 移植步骤		 : 中间层函数
* 输入           : 无
* 输出           : 无
* 返回           : 无
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
