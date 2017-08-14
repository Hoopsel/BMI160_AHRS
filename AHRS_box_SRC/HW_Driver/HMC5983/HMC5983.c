
#include "math.h"
#include "HMC5983.h"
#include "eeprom.h"

//-------------------------------------------------------------------------
#define	HMC_CSH			GPIO_SetBits(GPIOC, GPIO_Pin_2)
#define	HMC_CSL			GPIO_ResetBits(GPIOC, GPIO_Pin_2)

//�����Ʊ궨ֵ
int16_t  HMC5983_maxx=0,HMC5983_maxy=0,HMC5983_maxz=0,
		 HMC5983_minx=-0,HMC5983_miny=-0,HMC5983_minz=-0;
unsigned char HMC5983_calib = 0; //��ʼ����ɱ�־

//�����Ʊ궨ֵ
static int16_t  Mag_Offset_X = 0, //ƫ��
				Mag_Offset_Y = 0,
				Mag_Offset_Z = 0;

static float  Mag_Scale_X = 1.0f, //������
	   		  Mag_Scale_Y = 1.0f,
	   		  Mag_Scale_Z = 1.0f;
int16_t  
		 lastMx,lastMy,lastMz; //���µĴ�����ADCֵ

int16_t  HMC5983_FIFO[3][11]; //�������˲�
void HMC5983_getRaw(int16_t *x,int16_t *y,int16_t *z);


/**************************ʵ�ֺ���********************************************
*����ԭ��:	   void HMC5983_FIFO_init(void)
*��������:	   ������ȡ100�����ݣ��Գ�ʼ��FIFO����
���������  ��
���������  ��
*******************************************************************************/
void HMC5983_FIFO_init(void)
{
  int16_t temp[3];
  unsigned char i;
  for(i=0;i<50;i++){
  HMC5983_getRaw(&temp[0],&temp[1],&temp[2]);
  delay_us(200);  //��ʱ�ٶ�ȡ����
  }
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	   void  HMC5983_newValues(int16_t x,int16_t y,int16_t z)
*��������:	   ����һ�����ݵ�FIFO����
���������  �������������Ӧ��ADCֵ
���������  ��
*******************************************************************************/
void  HMC5983_newValues(int16_t x,int16_t y,int16_t z)
{
	unsigned char i ;
	int32_t sum=0;

	for(i=1;i<10;i++){
		HMC5983_FIFO[0][i-1]=HMC5983_FIFO[0][i];
		HMC5983_FIFO[1][i-1]=HMC5983_FIFO[1][i];
		HMC5983_FIFO[2][i-1]=HMC5983_FIFO[2][i];
	}

	HMC5983_FIFO[0][9]=y;	//x
	HMC5983_FIFO[1][9]=-x;	  //��Ի�
	HMC5983_FIFO[2][9]=z;	  //z

	sum=0;
	for(i=0;i<10;i++){	//ȡ�����ڵ�ֵ���������ȡƽ��
   		sum+=HMC5983_FIFO[0][i];
	}
	HMC5983_FIFO[0][10]=sum/10;	//��ƽ��ֵ����

	sum=0;
	for(i=0;i<10;i++){
   		sum+=HMC5983_FIFO[1][i];
	}
	HMC5983_FIFO[1][10]=sum/10;

	sum=0;
	for(i=0;i<10;i++){
   		sum+=HMC5983_FIFO[2][i];
	}
	HMC5983_FIFO[2][10]=sum/10;

	if(HMC5983_calib){//У����Ч�Ļ� �ɼ����ֵ��Сֵ
		if(HMC5983_minx>HMC5983_FIFO[0][10])HMC5983_minx=(int16_t)HMC5983_FIFO[0][10];
		if(HMC5983_miny>HMC5983_FIFO[1][10])HMC5983_miny=(int16_t)HMC5983_FIFO[1][10];
		if(HMC5983_minz>HMC5983_FIFO[2][10])HMC5983_minz=(int16_t)HMC5983_FIFO[2][10];

		if(HMC5983_maxx<HMC5983_FIFO[0][10])HMC5983_maxx=(int16_t)HMC5983_FIFO[0][10];
		if(HMC5983_maxy<HMC5983_FIFO[1][10])HMC5983_maxy=(int16_t)HMC5983_FIFO[1][10];
		if(HMC5983_maxz<HMC5983_FIFO[2][10])HMC5983_maxz=(int16_t)HMC5983_FIFO[2][10];
	}

} //HMC5983_newValues

/**************************ʵ�ֺ���********************************************
*����ԭ��:	   void HMC5983_writeReg(unsigned char reg, unsigned char val)
*��������:	   дHMC5983L�ļĴ���
���������    reg  �Ĵ�����ַ
			  val   Ҫд���ֵ	
���������  ��
*******************************************************************************/
void HMC5983_writeReg(unsigned char reg, unsigned char val) {
  HMC_CSL;
  SPI1_ReadWrite_Byte(reg);
  SPI1_ReadWrite_Byte(val);
  HMC_CSH;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_getRaw(int16_t *x,int16_t *y,int16_t *z)
*��������:	   дHMC5983L�ļĴ���
���������    reg  �Ĵ�����ַ
			  val   Ҫд���ֵ	
���������  ��
*******************************************************************************/
void HMC5983_getRaw(int16_t *x,int16_t *y,int16_t *z) {
  unsigned char vbuff[6] , i=100;
  vbuff[0]=vbuff[1]=vbuff[2]=vbuff[3]=vbuff[4]=vbuff[5]=0;
  HMC_CSL;
  SPI1_ReadWrite_Byte(HMC5983_R_XM | 0xC0);
  for(i=0;i<6;i++)
  vbuff[i] = SPI1_ReadWrite_Byte(0xFF);
  HMC_CSH;
  HMC5983_newValues(((int16_t)vbuff[0] << 8) | vbuff[1],((int16_t)vbuff[4] << 8) | vbuff[5],((int16_t)vbuff[2] << 8) | vbuff[3]); 
   *x = HMC5983_FIFO[0][10];
   *y = HMC5983_FIFO[1][10];
   *z = HMC5983_FIFO[2][10];
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_getValues(int16_t *x,int16_t *y,int16_t *z)
*��������:	   ��ȡ �����Ƶĵ�ǰADCֵ
���������    �������Ӧ�����ָ��	
���������  ��
*******************************************************************************/
void HMC5983_getlastValues(int16_t *x,int16_t *y,int16_t *z) {
  *x = HMC5983_FIFO[0][10];
  *y = HMC5983_FIFO[1][10]; 
  *z = HMC5983_FIFO[2][10]; 
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_mgetValues(float *arry)
*��������:	   ��ȡ У����� ������ADCֵ
���������    �������ָ��	
���������  ��
*******************************************************************************/
uint32_t HMC5883_Last_Update = 0;
void HMC5983_mgetValues(float *arry) {
  int16_t xr,yr,zr;
  uint32_t mtime = micros();  //��ȡʱ��
  //��ȡƵ�� 220hz
  if((HMC5883_Last_Update==0)||((HMC5883_Last_Update+4500)<mtime)||(HMC5883_Last_Update > mtime)){
  HMC5983_getRaw(&xr, &yr, &zr);
  HMC5883_Last_Update = mtime;
  }else {
  	HMC5983_getlastValues(&xr, &yr, &zr);
  }

  arry[0] = (xr-Mag_Offset_X)*Mag_Scale_X;  //Mx
  arry[1] = (yr-Mag_Offset_Y)*Mag_Scale_Y;  //My
  arry[2] = (zr-Mag_Offset_Z)*Mag_Scale_Z;  //Mz
  lastMx = arry[0];
  lastMy = arry[1];
  lastMz = arry[2];
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_setGain(unsigned char gain)
*��������:	   ���� 5883L������
���������     Ŀ������ 0-7
���������  ��
*******************************************************************************/
void HMC5983_setGain(unsigned char gain) { 
  // 0-7, 1 default
  if (gain > 7) return;
  HMC5983_writeReg(HMC5983_R_CONFB, gain << 5);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_setMode(unsigned char mode)
*��������:	   ���� 5883L�Ĺ���ģʽ
���������     ģʽ
���������  ��
*******************************************************************************/
void HMC5983_setMode(unsigned char mode) {
  if (mode > 2) {
    return;
  }
  HMC5983_writeReg(HMC5983_R_MODE, mode);
  delay_us(100);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_init(u8 setmode)
*��������:	   ���� 5883L�Ĺ���ģʽ
���������     ģʽ
���������  ��
*******************************************************************************/
void HMC5983_init(u8 setmode) {
  if (setmode) {
    HMC5983_setMode(0);
  }

  HMC5983_writeReg(HMC5983_R_CONFA, 0x70); // 8 samples averaged, 75Hz frequency, no artificial bias.
  HMC5983_writeReg(HMC5983_R_CONFB, 0x20); // -+1.3Ga	  1090LSB/Gauss
  HMC5983_writeReg(HMC5983_R_MODE, 0x00);
  HMC5983L_update_config();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_setDOR(unsigned char DOR)
*��������:	   ���� 5983L�� �����������
���������     ����ֵ
0 -> 0.75Hz  |   1 -> 1.5Hz
2 -> 3Hz     |   3 -> 7.5Hz
4 -> 15Hz    |   5 -> 30Hz
6 -> 220Hz  
���������  ��
*******************************************************************************/
void HMC5983_setDOR(unsigned char DOR) {
	  if (DOR>6) return;
	  HMC5983_writeReg(HMC5983_R_CONFA,DOR<<2);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983_getID(char id[3])
*��������:	   ��ȡоƬ��ID
���������     	ID��ŵ�����
���������  ��
*******************************************************************************/
void HMC5983_getID(char id[3]) 
{
	HMC_CSL;
  	SPI1_ReadWrite_Byte(HMC5983_R_IDA | 0xC0);
  	id[0] = SPI1_ReadWrite_Byte(0xff);
	id[1] = SPI1_ReadWrite_Byte(0xff);
	id[2] = SPI1_ReadWrite_Byte(0xff);
  	HMC_CSH;
}   // getID().

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983L_SetUp(void)
*��������:	   ��ʼ�� HMC5983L ʹ֮�������״̬
���������     	
���������  ��
*******************************************************************************/
void HMC5983L_SetUp(void)
{ 
	  char id[3];
	  HMC5983_getID(id);
	  HMC5983_init(0); //  -+1.3Ga
	  HMC5983_setMode(0);
	  HMC5983_setDOR(6);  //220hz ������
	  HMC5983_FIFO_init();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983L_Start_Calib(void)
*��������:	   ��������Ʊ궨
���������     	
���������  ��
*******************************************************************************/
void HMC5983L_Start_Calib(void)
{

	HMC5983_calib = 1;//��ʼ�궨
	HMC5983_maxx = -4096;	//��ԭ���ı궨ֵ���
	HMC5983_maxy = -4096;
	HMC5983_maxz = -4096;
	HMC5983_minx = 4096;
	HMC5983_miny = 4096;
	HMC5983_minz = 4096;
	Mag_Scale_X = 1.0f;
	Mag_Scale_Y = 1.0f;
	Mag_Scale_Z = 1.0f;
	Mag_Offset_X = 0;
	Mag_Offset_Y = 0;
	Mag_Offset_Z = 0;
}

void HMC5983L_update_config(void){
	Mag_Offset_X = Config.dMx_offset;
	Mag_Offset_Y = Config.dMy_offset;
	Mag_Offset_Z = Config.dMz_offset;

	Mag_Scale_X = Config.dMx_scale ; 
    Mag_Scale_Y = Config.dMy_scale ;  
    Mag_Scale_Z = Config.dMz_scale ;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void HMC5983L_Save_Calib(void)
*��������:	  ��������Ʊ궨ֵ ��Flash
���������     	
���������  ��
*******************************************************************************/
void HMC5983L_Save_Calib(void){

	if(HMC5983_maxx == HMC5983_minx)return ;
	if(HMC5983_maxy == HMC5983_miny)return ;
	if(HMC5983_maxz == HMC5983_minz)return ;  //�����������⣬���߸�����û��ת����
	//�������Ʊ궨ֵд�� Flash ����
	Config.dMx_offset = (HMC5983_maxx+HMC5983_minx)/2;
	Config.dMy_offset = (HMC5983_maxy+HMC5983_miny)/2;
	Config.dMz_offset = (HMC5983_maxz+HMC5983_minz)/2;

	Config.dMx_scale = Mag_Scale_X = 1.0f;
	Config.dMy_scale = Mag_Scale_Y = (float)(HMC5983_maxx-HMC5983_minx)/(float)(HMC5983_maxy-HMC5983_miny);
	Config.dMz_scale = Mag_Scale_Z = (float)(HMC5983_maxx-HMC5983_minx)/(float)(HMC5983_maxz-HMC5983_minz); 

	Write_config();	 //����ǰ����д��
	HMC5983_calib=0; //�����궨
}	//HMC5983L_Save_Calib()

//------------------End of File----------------------------
