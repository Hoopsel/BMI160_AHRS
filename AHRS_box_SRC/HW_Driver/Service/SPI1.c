
#include "SPI1.h"

static uint8_t SPI1_ready = 0;

/**************************ʵ�ֺ���********************************************
*����ԭ��:		void SPI1_Configuration(void)
*��������:	    ��ʼ�� SPI1 �ӿ�
*******************************************************************************/
void SPI1_Configuration(void){

	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	if(SPI1_ready >1)return;
	SPI1_ready++;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* SCK, MISO and MOSI  PA5=CLK,PA6=MISO,PA7=MOSI*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);

	/*  PC2 PA4 ��Ƭѡ*/

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOC, GPIO_Pin_2);//Ԥ��Ϊ��
	GPIO_SetBits(GPIOA, GPIO_Pin_4);//Ԥ��Ϊ��
	
	/* SPI1 configuration  */
	SPI_Cmd(SPI1, DISABLE);        
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //��
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;      //8λ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;        //CPOL=0 ʱ�����յ�
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;       //CPHA=0 ���ݲ����1��
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;        //���NSS
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;  //32��Ƶ

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;      //��λ��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;        //CRC7
    
	SPI_Init(SPI1, &SPI_InitStructure);	 //Ӧ�����õ� SPI1
	SPI_Cmd(SPI1, ENABLE); 
					   
}

void SPI1_SetSpeed(uint16_t SpeedSet){
	SPI1->CR1 &= 0XFF87; 
	SPI1->CR1 |= SpeedSet;	//����SPI1�ٶ�  
	SPI1->CR1 |= 1<<6; 		//SPI�豸ʹ��
}

/************************************************************************
** ��������:uint8_t SPI1_ReadWrite_Byte(uint8_t byte)
** ��������:  ���ͻ��߽���1���ֽ�
** �䡡��:    byte    ����ʱ��,byte����Ϊ���͵������ֽڣ� ���յ�ʱ����̶�Ϊ0xff
** �䡡��:    SPI1->DR  ����ʱ�򣬿��Ժ���, ���յ�ʱ����Ϊ��������
***********************************************************************/
uint8_t SPI1_ReadWrite_Byte(uint8_t byte)
{	
	//�ȴ����ͼĴ�����
	while((SPI1->SR & SPI_I2S_FLAG_TXE)==RESET);
	SPI1->DR = byte;  //����һ���ֽ�
	// �ȴ����ռĴ�����Ч
	while((SPI1->SR & SPI_I2S_FLAG_RXNE)==RESET);
	return(SPI1->DR);	
}


//д�Ĵ���
void SPI1_writeReg(u8 reg ,u8 data){
	SPI1_ReadWrite_Byte(reg);
	SPI1_ReadWrite_Byte(data);
}

//���Ĵ���
u8 SPI1_readReg(u8 reg){
	SPI1_ReadWrite_Byte(reg|0x80);
	return SPI1_ReadWrite_Byte(0xff);
}

//�ӼĴ�����������ֽ�[�Ĵ�����ַҪ�Զ�����]
void SPI1_readRegs(u8 reg, u8 length, u8 *data){
	u8 count = 0;
	SPI1_ReadWrite_Byte(reg|0x80);
	for(count=0;count<length;count++){
		data[count] = SPI1_ReadWrite_Byte(0xff);
	}
}

//------------------End of File----------------------------
