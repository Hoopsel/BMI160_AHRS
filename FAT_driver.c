#include "FAT_driver.h"
#include "UART1.h"
#include  "IMU.h"
#include "OSQMem.h"

FATFS fs;  		//�߼����̹�����.	 
FIL file;	  		//�ļ�1
FIL ftemp;	  		//�ļ�2.
UINT br,bw;			//��д����
FILINFO fileinfo;	//�ļ���Ϣ
DIR dir;  			//Ŀ¼

u8 fatbuf[512];			//SD�����ݻ�����
u8 res=0;
u32 FileSave_DelayC = 500; 

u8 File_SecBuf[512];
u8 File_Buf[4096];
uint16_t FBindex = 0;
void Add_To_Buf(u8* data , int16_t len){
  int16_t i;
  for(i=0;i<len;i++){
    File_Buf[FBindex++] = data[i];
  }
}

void Get_From_Buf512(u8* data){
  int16_t i;
  for(i=0;i<512;i++){
    data[i] = File_Buf[i];
  }
  for(i=0;i< FBindex - 512;i++){
    File_Buf[i] = File_Buf[512+i];
  }
  FBindex = FBindex - 512;
}


void FAT_Initial(void){
	u16 try = 0;
	while(SD_Init()){
		if(++try > 100)return;
	}
	res = 1;try = 0;
	while((res != 0)&&(try++ < 100)){
		res = f_mount(&fs,"0:",1);
		}
}

//��·���µ��ļ�
//path:·��+�ļ���
//mode:��ģʽ
//����ֵ:ִ�н��
u8 mf_open(u8 *path,u8 mode)
{
	u16 try = 0;
	res = 1;
	while((res != 0)&&(try++ < 200)){
		res=f_open(&file,(TCHAR*)path,mode);//���ļ�
		}

	return res;
} 
//�ر��ļ�
//����ֵ:ִ�н��
u8 mf_close(void)
{
  mf_write(File_Buf,FBindex);
	f_close(&file);
	return 0;
}
//��������
//len:�����ĳ���
//����ֵ:ִ�н��
u8 mf_read(u16 len)
{
	u16 i;
	u16 tlen=0;
	for(i=0;i<len/512;i++)
	{
		res=f_read(&file,fatbuf,512,&br);
		if(res)
		{
			break;
		}else
		{
			tlen+=br;
		}
	}
	if(len%512)
	{
		res=f_read(&file,fatbuf,len%512,&br);
		if(res)	//�����ݳ�����
		{  
		}else
		{
			tlen+=br; 
		}	 
	} 
	return res;
}
//д������
//dat:���ݻ�����
//len:д�볤��
//����ֵ:ִ�н��
u8 mf_write(u8*dat,u16 len)
{			    				   	 
	res = f_write(&file,dat,len,&bw);
	//if(res != 0){UART1_Putw_Dec(res);UART1_Put_String("f_write err\r\n");}
	return res;
}

//�ļ���дָ��ƫ��
//offset:����׵�ַ��ƫ����
//����ֵ:ִ�н��.
u8 mf_lseek(u32 offset)
{
	return f_lseek(&file,offset);
}

void mf_f_sync(void){
  f_sync(&file);
}

//�������ݱ����Ƶ��
void FileSave_TimerSet(u16 Speed)
{
	//ϵͳ��84Mhz  84M/��8399+1��=10000��
	//������װ����500��*10=5000/10000=0.05s
	//����һ����ʱ���Ľṹ�����
	NVIC_InitTypeDef NVIC_InitStructure;
 	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//������ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); 
 	//������װ�ص�ֵ
 	TIM_TimeBaseStructure.TIM_Period = 10000/Speed - 1;  
 	//Ԥ��Ƶ��ֵ������ֵ��1
 	TIM_TimeBaseStructure.TIM_Prescaler = 8399; 
 	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
 	//��������������
 	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	//���жϣ������жϡ�
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE); 
	
 	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; 
 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;   
 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
 	NVIC_Init(&NVIC_InitStructure); 

	TIM4->CNT = 0;
	TIM4->CR1 |= 0x01;    //ʹ�ܶ�ʱ��
}

void FileSave_Stop(void){
   TIM4->CNT = 0;
   TIM4->CR1 &= 0xFFFE;    //ֹͣ��ʱ��
}

extern float  pitch ,roll ,yaw;
//  Temperature in 0.01C
//  Pressure    in 0.01mbar = Pa
//  Altitude    in meter  / cm
extern float MS5611_Temperature,MS5611_Pressure,MS5611_Altitude;
extern int16_t lastGx,lastGy,lastGz;//���µ�������ٶ�ADCֵ��
extern int16_t  lastAx,lastAy,lastAz,  //���µļ��ٶ�ADCֵ
		 		lastMx,lastMy,lastMz;  //���µĴ�����ADCֵ
extern int16_t lastVOL;  //��ȡ��ǰ�ĵ�ص�ѹֵ�� ��λ 0.01V
uint32_t dtime,lasttime=0;
int16_t fdata_t;
uint32_t fdata_32t;
unsigned char File_Data[45];
unsigned char File_Data_buf[45];
unsigned char Data_Ready = 0;
uint16_t Save_Err_count = 0;
extern void Updata_PC_Route(void);
void TIM4_IRQHandler(void)
{
static int i,sum;
if (TIM4->SR&0X0001)//����ж�
	{
        if(FileSave_DelayC != 0) {
            FileSave_DelayC -- ;
            TIM4->SR&=~(1<<0);
            return;
        }
	dtime = micros();
    
	File_Data[0] = 0xA5;
	File_Data[1] = 0x5A;
	File_Data[2] = sizeof(File_Data)-2;
	File_Data[3] = 0xDF;  //����ʶ���ֽ�
	fdata_t = (int16_t)(yaw*10.0f);
	File_Data[4] = fdata_t>>8;
	File_Data[5] = fdata_t;

	fdata_t = (int16_t)(pitch*10.0f);
	File_Data[6] = fdata_t>>8;
	File_Data[7] = fdata_t;

	fdata_t = (int16_t)(roll*10.0f);
	File_Data[8] = fdata_t>>8;
	File_Data[9] = fdata_t;
	//0.01C
	fdata_t = (int16_t)MS5611_Temperature;
	File_Data[10] = fdata_t>>8;
	File_Data[11] = fdata_t;
	//Pa
	fdata_32t = (int32_t)MS5611_Pressure;
	File_Data[12] = fdata_32t>>24;
	File_Data[13] = fdata_32t>>16;
	File_Data[14] = fdata_32t>>8;
	File_Data[15] = fdata_32t;
	//0.01m
	fdata_32t = (int32_t)MS5611_Altitude;
	File_Data[16] = fdata_32t>>24;
	File_Data[17] = fdata_32t>>16;
	File_Data[18] = fdata_32t>>8;
	File_Data[19] = fdata_32t;
	//0.01V
	fdata_t = (int16_t)lastVOL;
	File_Data[20] = fdata_t>>8;
	File_Data[21] = fdata_t;

	fdata_t = (int16_t)lastAx;
	File_Data[22] = fdata_t>>8;
	File_Data[23] = fdata_t;

	fdata_t = (int16_t)lastAy;
	File_Data[24] = fdata_t>>8;
	File_Data[25] = fdata_t;

	fdata_t = (int16_t)lastAz;
	File_Data[26] = fdata_t>>8;
	File_Data[27] = fdata_t;

	fdata_t = (int16_t)lastGx;
	File_Data[28] = fdata_t>>8;
	File_Data[29] = fdata_t;

	fdata_t = (int16_t)lastGy;
	File_Data[30] = fdata_t>>8;
	File_Data[31] = fdata_t;

	fdata_t = (int16_t)lastGz;
	File_Data[32] = fdata_t>>8;
	File_Data[33] = fdata_t;

	fdata_t = (int16_t)lastMx;
	File_Data[34] = fdata_t>>8;
	File_Data[35] = fdata_t;

	fdata_t = (int16_t)lastMy;
	File_Data[36] = fdata_t>>8;
	File_Data[37] = fdata_t;

	fdata_t = (int16_t)lastMz;
	File_Data[38] = fdata_t>>8;
	File_Data[39] = fdata_t;
	//Time  us
	fdata_32t = dtime;
	File_Data[40] = fdata_32t>>24;
	File_Data[41] = fdata_32t>>16;
	File_Data[42] = fdata_32t>>8;
	File_Data[43] = fdata_32t;

	sum = 0;
	for(i=2;i<sizeof(File_Data)-1;i++)
		sum += File_Data[i]; 
		
	File_Data[44] = sum;
  Updata_PC_Route();
  /*
	if(Data_Ready == 0x0){ //ȷ�������ѱ���
	for(i=0;i<sizeof(File_Data);i++) //�������ݵ�������
		File_Data_buf[i] = File_Data[i];
	}
	Data_Ready = 1;
  */
  Add_To_Buf(File_Data,sizeof(File_Data));
  if((FBindex>512)&&(Data_Ready == 0)){
    Get_From_Buf512(File_SecBuf);
    Data_Ready = 1;
  }
  
	}  
	TIM4->SR&=~(1<<0);//����жϱ�־λ  	
}

void File_head(void){
	uint16_t  temp,sum=0,i;
    unsigned char mtemp[7];
	temp = Gyro_Resolution;
	mtemp[0] = temp>>8;
	mtemp[1] = temp;

	temp = Acc_Resolution;
	mtemp[2] = temp>>8;
	mtemp[3] = temp;

	temp = Mag_Resolution;
	mtemp[4] = temp>>8;
	mtemp[5] = temp;
	sum = 0;
	for(i=0;i<6;i++)sum += mtemp[i];
	mtemp[6] = sum;
	res = mf_write(mtemp,7);
    f_sync(&file);
    if(FileSave_DelayC == 0)
        FileSave_DelayC=20;
    Data_Ready = 0;
}

//��ʱ���ã���ȷ���Ƿ���������Ҫд���ļ�
uint32_t ftime,flasttime=0;
uint16_t File_W_Errcount = 0;
uint16_t FileSaveCount = 0;
void File_Save_Routing(void){
	uint32_t temp1,temp2;
	 if(Data_Ready){
     temp1 = micros();
     if(mf_write(File_SecBuf,512)==0);
//	 	if(mf_write(File_Data_buf,sizeof(File_Data))==0);
//     else UART1_Put_String("f_write err\r\n");
		Data_Ready = 0;
	}

}




