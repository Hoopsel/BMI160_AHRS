

#include "SPI2.h"

static uint8_t SPI2_ready = 0;

void SPI2_Configuration(void){

	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	if(SPI2_ready >0)return;
	SPI2_ready++;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	/* SCK, MISO and MOSI  PB13=CLK,PB14=MISO,PB15=MOSI*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);

	 
	/*  PB12 ��Ƭѡ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);//Ԥ��Ϊ��

	/*  PB10 �������*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	

	/* SPI2 configuration  */
	SPI_Cmd(SPI2, DISABLE);        
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //��
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;      //8λ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;       //CPOL=0 ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;       //CPHA=0 ���ݲ����1��
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //���NSS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  //4��Ƶ

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;      //��λ��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;        //CRC7
    
	SPI_Init(SPI2, &SPI_InitStructure);	 //Ӧ�����õ� SPI2
	SPI_Cmd(SPI2, ENABLE); 
}

void SPI2_SetSpeed(uint16_t SpeedSet){
	SPI2->CR1 &= 0XFF87; 
	SPI2->CR1 |= SpeedSet;	//����SPI2�ٶ�  
	SPI2->CR1 |= 1<<6; 		//SPI�豸ʹ��
}

/************************************************************************
** ��������:static u8 SPI_ReadWrite_Byte(u8 byte)
** ��������:  ���ͻ��߽���1���ֽ�
** �䡡��:    byte    ����ʱ��,byte����Ϊ���͵������ֽڣ� ���յ�ʱ����̶�Ϊ0xff
** �䡡��:    SPI1->DR  ����ʱ�򣬿��Ժ���, ���յ�ʱ����Ϊ��������
***********************************************************************/
uint8_t SPI2_ReadWrite_Byte(uint8_t byte)
{
	/*�ȴ����ͼĴ�����*/
	while((SPI2->SR & SPI_I2S_FLAG_TXE)==RESET);
	/*����һ���ֽ�*/
	SPI2->DR = byte;
	/* �ȴ����ռĴ�����Ч*/
	while((SPI2->SR & SPI_I2S_FLAG_RXNE)==RESET);
	return(SPI2->DR);
}

	
//------------------End of File----------------------------		
