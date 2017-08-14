


#include "LED.h"
#include "math.h"
#include "AD7689.h"
#include "eeprom.h"


//��׼��ѹ+5.000V  16λADC ,��Ӧ [13.107  LBS/mV]
#define Sensitive_Accel  13107.0f        //���ٶ�������[1000mV/g]
#define Sensitive_Gyro   78.642f         //������������[6mV/��/sec] 
#define Gyro_To_32_8     (32.8f / Sensitive_Gyro) //��	������ �� �� ��ת��32.8,�Ա����λ����Ӧ��

#define Buf_SIZE    10

int16_t 
	  Accel_Xint,  //ADCֵ
	  Accel_Yint,	
	  Accel_Zint,
	  Gyro_Xint,   
	  Gyro_Yint,	
	  Gyro_Zint ;

float Accel_X,  //���ٶ�X��, ��λg [9.8m/S^2]
	  Accel_Y,	//���ٶ�Y��, ��λg [9.8m/S^2]
	  Accel_Z,	//���ٶ�Z��, ��λg [9.8m/S^2]
	  Gyro_X,   //���ٶ�X��, ��λdps [��ÿ��]
	  Gyro_Y,	//���ٶ�Y��, ��λdps [��ÿ��]
	  Gyro_Z	//���ٶ�Z��, ��λdps [��ÿ��]
	  ;
	  
//-------------------------------------------------------------------------
#define	CSH			GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define	CSL			GPIO_ResetBits(GPIOA, GPIO_Pin_8)

/*
 ����ͨ��Ϊ�����ԣ�INx��GND��Ϊ�ο�
 ʹ���ⲿ��׼��ѹԴ [5.0V]
*/
static uint16_t  AD7689_Config_buf[8]={
							(0x3c39|(0x0005<<7)), //ͨ��5��CFG����ֵ
							(0x3c39|(0x0006<<7)), //ͨ��6��CFG����ֵ
							(0x3c39|(0x0007<<7)),  //ͨ��7��CFG����ֵ
							(0x3c39|(0x0000<<7)), //ͨ��0��CFG����ֵ
							(0x3c39|(0x0001<<7)), //ͨ��1��CFG����ֵ
							(0x3c39|(0x0002<<7)), //ͨ��2��CFG����ֵ
							(0x3c39|(0x0003<<7)), //ͨ��3��CFG����ֵ
							(0x3c39|(0x0004<<7))  //ͨ��4��CFG����ֵ
							};
static uint16_t  AD7689_Result[8],AD7689_Config_index = 0;
static uint8_t   Result_Point[8] =  //��ǰ��ȡ�������ݣ���Ӧ��ADCͨ����
							{3,4,5,6,7,0,1,2};
uint8_t Gyro_Off_started = 0;
int16_t lastAx,lastAy,lastAz,
		lastGx,lastGy,lastGz;
static uint16_t Gx_offset=0,Gy_offset=0,Gz_offset=0;

//д��CFG����,����ȡ���ϴ�ADCת���Ľ��	[���AD7689�������ֲ�]
//���� uint16_t Config   14λ��������Ϣ  ��n��������Ϣ
//���� ADCת�����                       ��n-2��ת�����
static uint16_t Read_AD7689(uint16_t Config){
    uint16_t  ADC_Value = 0x00;
	Config = Config << 2;
	Config |= 0x8000;
	CSL;//ѡ��оƬ
	ADC_Value = SPI2_ReadWrite_Byte(Config >> 8);
	ADC_Value = ADC_Value << 8;	//ADC������ֽ�
	ADC_Value |= SPI2_ReadWrite_Byte(Config & 0x00ff);
	CSH;
	delay_us(1);
	return ADC_Value;
}

//ɨ��8��ͨͨ��ADCת�����
static void AD7689_Update_Result(void){
	uint8_t i;
	uint16_t temp;
	AD7689_Config_index = 0;
	for(i=0; i<8; i++){	//ɨ8��ͨ��
	temp = Read_AD7689(AD7689_Config_buf[AD7689_Config_index]);
	AD7689_Result[Result_Point[AD7689_Config_index]] = temp;
	AD7689_Config_index++;
	AD7689_Config_index = AD7689_Config_index%8;
	}

}

//��ȡ�������е�ADC���
static uint16_t Read_AD7689_Result(uint8_t ch){
	if(ch > 7)return 0;
	return AD7689_Result[ch];
}

void Gyro_Initial_Offset(void){
	uint16_t i;
	uint32_t offset_sumx = 0,
			offset_sumy = 0,
			offset_sumz = 0;
	for(i = 0;i < 50 ; i++)AD7689_Update_Result();
	for(i = 0;i < 600 ; i++){
	AD7689_Update_Result();
	delay_ms(1);
	offset_sumx += Read_AD7689_Result(2); //gyro x 
	offset_sumy += Read_AD7689_Result(1); //gyro y
	offset_sumz += Read_AD7689_Result(5); //gyro z
	}
	Config.dGx_offset = (uint16_t)(offset_sumx / 600);
	Config.dGy_offset = (uint16_t)(offset_sumy / 600);
	Config.dGz_offset = (uint16_t)(offset_sumz / 600);
	Write_config();  //д��flash����
}

int16_t filter_buf[6][Buf_SIZE]; 
int Wr_Index = 0 ;
int16_t Fliter_AVG(int16_t* buf) {
  	int32_t sum = 0;
	int i;
	for(i=0;i<Buf_SIZE;i++)
		sum += buf[i];
	return sum/Buf_SIZE;
}

//����6��Ĵ��������ݡ�
void Dof6_Update(void){
	float temp , mid;
	AD7689_Update_Result();
	mid = (float)32767.0; // [0xffff/2]  
	

	temp = (float)(AD7689_Result[2]);  //Gyro X
	temp -= Config.dGx_offset;
	temp = temp;   //����任
	Gyro_X = temp / Sensitive_Gyro;	//ת�ɶ�ÿ��
	lastGx = Gyro_Xint = temp;//*Gyro_To_32_8;
	
	temp = (float)(AD7689_Result[1]);  //Gyro Y
	temp -= Config.dGy_offset;
	temp = -temp;   //����任
	Gyro_Y = temp / Sensitive_Gyro;
	lastGy = Gyro_Yint = temp;//*Gyro_To_32_8;
	
	temp = (float)(AD7689_Result[5]);  //Gyro Z
	temp -= Config.dGz_offset;
	temp = -temp;   //����任
	Gyro_Z = temp / Sensitive_Gyro;
	lastGz = Gyro_Zint = temp;//*Gyro_To_32_8;
	
	temp = (float)((float)AD7689_Result[7]-mid);  //ACC x
	temp = -temp;   //����任
	lastAx = Accel_Xint = temp;
	Accel_X = temp / Sensitive_Accel; //ת�ɵ�λΪg  
		
	temp = (float)((float)AD7689_Result[6]-mid); //ACC y
	temp = -temp;   //����任
	lastAy =Accel_Yint = temp;
	Accel_Y = temp / Sensitive_Accel;
	
	temp = (float)((float)AD7689_Result[3]-mid); //ACC z
	temp = -temp;   //����任	
	lastAz = Accel_Zint = temp;
	Accel_Z = temp / Sensitive_Accel;

	filter_buf[0][Wr_Index] = Gyro_Xint;
	filter_buf[1][Wr_Index] = Gyro_Yint;
	filter_buf[2][Wr_Index] = Gyro_Zint;
	filter_buf[3][Wr_Index] = Accel_Xint;
	filter_buf[4][Wr_Index] = Accel_Yint;
	filter_buf[5][Wr_Index] = Accel_Zint;
	Wr_Index = (Wr_Index + 1) % Buf_SIZE;

	lastGx = Fliter_AVG(&filter_buf[0][0]);
	lastGy = Fliter_AVG(&filter_buf[1][0]);
	lastGz = Fliter_AVG(&filter_buf[2][0]);
	lastAx = Fliter_AVG(&filter_buf[3][0]);
	lastAy = Fliter_AVG(&filter_buf[4][0]);
	lastAz = Fliter_AVG(&filter_buf[5][0]);

}


//------------------End of File----------------------------
