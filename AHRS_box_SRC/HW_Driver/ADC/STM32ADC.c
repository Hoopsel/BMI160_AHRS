/* STM32ADC.c file
��д�ߣ�lisn3188
��ַ��www.chiplab7.com
����E-mail��lisn3188@163.com
���뻷����MDK-Lite  Version: 4.23
����ʱ��: 2012-11-12
���ԣ� ���������ڵ���ʵ���ҵ�[Captain �ɿذ�]����ɲ���

ռ��STM32 ��Դ��
1. ʹ��ADC1����ģ��ת��
2. ʹ��DMA ͨ��1�ɼ�ADC�Ľ��������Ҫ����ĸ�Ԥ

------------------------------------
 */

#include "STM32ADC.h"

#define ADC1_DR_Address    ((uint32_t)0x4001204C)
#define adc_buf_size  10   // ADC��������������ƽ��

uint16_t AD_Value[adc_buf_size];

//ADC ��������
void ADC_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	//����GPIOB	ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA ,
                           ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    //ADC1_IN1--> PA1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; //��������Ϊ ģ������      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; 
    GPIO_Init(GPIOA, &GPIO_InitStructure); 
}

/*******************************************************************************
* Function Name  : ADCDMA_Configuration
* Description    : DMA���ã���ADCģ���Զ���ת��������ڴ�
*******************************************************************************/
void ADC_DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    //����DMAʱ��	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    //DMA_DeInit(DMA2_Stream0);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0; 
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR_Address;
    DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&AD_Value;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	//ֻ��һ��ͨ�����ɼ���adc_buf_size ������
    DMA_InitStructure.DMA_BufferSize = adc_buf_size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    //ѭ��ģʽ������Bufferд�����Զ��ص���ʼ��ַ��ʼ����
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
    //������ɺ�����DMAͨ��
    DMA_Cmd(DMA2_Stream0, ENABLE);
}

/*******************************************************************************
* Function Name  : ADC1_Configuration
* Description    : ADC1���ã�����ADCģ�����ú���У׼��
*******************************************************************************/
void ADC1_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	//����ADC1ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2 | 
                         RCC_APB2Periph_ADC3, ENABLE);
	/* ADC Common Init **********************************************************/
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;  //ADC����ģʽ
  	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;  
  	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4; //ADC��������4��Ƶ
  	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//����ת������
  	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  	ADC_InitStructure.ADC_NbrOfConversion = 1;	//����ת�����г���Ϊ1
  	ADC_Init(ADC1, &ADC_InitStructure);
    
    //����ת������1��ͨ��8  AIN8
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_480Cycles);
    //����ת������2��ͨ��9������ʱ��>2.2us,(239cycles)	AIN9
    //ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_480Cycles);
    
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
	//ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);
    // ����ADC��DMA֧�֣�Ҫʵ��DMA���ܣ������������DMAͨ���Ȳ�����
    ADC_DMACmd(ADC1, ENABLE);
    
}

/*******************************************************************************
* Function Name  : ADC_Voltage_initial
* Description    : ADC ��ʼ�������ϵ��ʱ�����һ�Ρ�
*******************************************************************************/
void ADC_Voltage_initial(void){
	ADC_GPIO_Configuration();
	ADC_DMA_Configuration();
	ADC1_Configuration();
	
	//������һ��ADת��
    /* Start ADC1 Software Conversion */ 
  	ADC_SoftwareStartConv(ADC1);
    //��Ϊ�Ѿ����ú���DMA��������AD�Զ�����ת��������Զ�������AD_Value�� 
}

/*******************************************************************************
* Function Name  : Get_Bat_Vol
* Description    : ��ȡ��ص�ѹ����λ 0.01V   
*******************************************************************************/  
int16_t lastVOL;
int16_t Get_Bat_Vol(void){
	float temp;
	int i;
	temp = 0;
	for(i=0;i<adc_buf_size;i++){
	 temp += AD_Value[i];
	}
	temp = temp / adc_buf_size;

	temp = (temp*330)/4096;	//ADCֵ ����ѹ��ת��
	temp = temp * 11; //��ѹ���� ����
	lastVOL = temp;
	return temp ;  
}

// ��ص�ѹ����3.60v  ��ʾ�ػ�
static int Bat_low_Count = 0;
uint8_t Is_BAT_LOW(void){
	
	 if(Get_Bat_Vol()<360){
		 Bat_low_Count ++;
		 if(Bat_low_Count > 10)return 1;
	 }else Bat_low_Count = 0;
	
   return 0;
}



//------------------End of File----------------------------
