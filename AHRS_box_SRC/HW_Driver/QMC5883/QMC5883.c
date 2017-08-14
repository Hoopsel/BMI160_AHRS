
#include "math.h"
#include "QMC5883.h"
#include "eeprom.h"
#include "IMU.h"

//-------------------------------------------------------------------------

//�����Ʊ궨ֵ
int16_t  QMC5883_maxx=0,QMC5883_maxy=0,QMC5883_maxz=0,
		 QMC5883_minx=-0,QMC5883_miny=-0,QMC5883_minz=-0;
unsigned char QMC5883_calib = 0; //��ʼ����ɱ�־

//�����Ʊ궨ֵ
static int16_t  Mag_Offset_X = 0, //ƫ��
				Mag_Offset_Y = 0,
				Mag_Offset_Z = 0;

static float  Mag_Scale_X = 1.0f, //������
	   		  Mag_Scale_Y = 1.0f,
	   		  Mag_Scale_Z = 1.0f;
int16_t  
		 lastMx,lastMy,lastMz; //���µĴ�����ADCֵ
float  magic_GRAVITY;

int16_t  QMC5883_FIFO[3][11]; //�������˲�
void QMC5883_getRaw(int16_t *x,int16_t *y,int16_t *z);


/**************************ʵ�ֺ���********************************************
*����ԭ��:	   void QMC5883_FIFO_init(void)
*��������:	   ������ȡ100�����ݣ��Գ�ʼ��FIFO����
���������  ��
���������  ��
*******************************************************************************/
void QMC5883_FIFO_init(void)
{
  int16_t temp[3];
  unsigned char i;
  for(i=0;i<50;i++){
  QMC5883_getRaw(&temp[0],&temp[1],&temp[2]);
  delay_us(200);  //��ʱ�ٶ�ȡ����
  }
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	   void  QMC5883_newValues(int16_t x,int16_t y,int16_t z)
*��������:	   ����һ�����ݵ�FIFO����
���������  �������������Ӧ��ADCֵ
���������  ��
*******************************************************************************/
void  QMC5883_newValues(int16_t x,int16_t y,int16_t z)
{
	unsigned char i ;
	int32_t sum=0;

	for(i=1;i<10;i++){
		QMC5883_FIFO[0][i-1]=QMC5883_FIFO[0][i];
		QMC5883_FIFO[1][i-1]=QMC5883_FIFO[1][i];
		QMC5883_FIFO[2][i-1]=QMC5883_FIFO[2][i];
	}

	QMC5883_FIFO[0][9]=y;	//x
	QMC5883_FIFO[1][9]=-x;	  //��Ի�
	QMC5883_FIFO[2][9]=z;	  //z

	sum=0;
	for(i=0;i<10;i++){	//ȡ�����ڵ�ֵ���������ȡƽ��
   		sum+=QMC5883_FIFO[0][i];
	}
	QMC5883_FIFO[0][10]=sum/10;	//��ƽ��ֵ����

	sum=0;
	for(i=0;i<10;i++){
   		sum+=QMC5883_FIFO[1][i];
	}
	QMC5883_FIFO[1][10]=sum/10;

	sum=0;
	for(i=0;i<10;i++){
   		sum+=QMC5883_FIFO[2][i];
	}
	QMC5883_FIFO[2][10]=sum/10;

	if(QMC5883_calib){//У����Ч�Ļ� �ɼ����ֵ��Сֵ
		if(QMC5883_minx>QMC5883_FIFO[0][10])QMC5883_minx=(int16_t)QMC5883_FIFO[0][10];
		if(QMC5883_miny>QMC5883_FIFO[1][10])QMC5883_miny=(int16_t)QMC5883_FIFO[1][10];
		if(QMC5883_minz>QMC5883_FIFO[2][10])QMC5883_minz=(int16_t)QMC5883_FIFO[2][10];

		if(QMC5883_maxx<QMC5883_FIFO[0][10])QMC5883_maxx=(int16_t)QMC5883_FIFO[0][10];
		if(QMC5883_maxy<QMC5883_FIFO[1][10])QMC5883_maxy=(int16_t)QMC5883_FIFO[1][10];
		if(QMC5883_maxz<QMC5883_FIFO[2][10])QMC5883_maxz=(int16_t)QMC5883_FIFO[2][10];
	}

} //QMC5883_newValues

/**************************ʵ�ֺ���********************************************
*����ԭ��:	   void QMC5883_writeReg(unsigned char reg, unsigned char val)
*��������:	   дQMC5883L�ļĴ���
���������    reg  �Ĵ�����ַ
			  val   Ҫд���ֵ	
���������  ��
*******************************************************************************/
void QMC5883_writeReg(unsigned char reg, unsigned char val) {
  IICwriteByte(QMC5883L_ADDR,reg,val);
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void QMC5883_getRaw(int16_t *x,int16_t *y,int16_t *z)
*��������:	   дQMC5883L�ļĴ���
���������    reg  �Ĵ�����ַ
			  val   Ҫд���ֵ	
���������  ��
*******************************************************************************/
void QMC5883_getRaw(int16_t *x,int16_t *y,int16_t *z) {
  unsigned char vbuff[6] , i=100;
  vbuff[0]=vbuff[1]=vbuff[2]=vbuff[3]=vbuff[4]=vbuff[5]=0;
  if((I2C_ReadOneByte(QMC5883L_ADDR,0x06)&0x01) != 0){
     IICreadBytes(QMC5883L_ADDR,0x00,6,vbuff);
  }
  QMC5883_newValues(((int16_t)vbuff[1] << 8) | vbuff[0],((int16_t)vbuff[3] << 8) | vbuff[2],((int16_t)vbuff[5] << 8) | vbuff[4]); 
   *x = QMC5883_FIFO[0][10];
   *y = QMC5883_FIFO[1][10];
   *z = QMC5883_FIFO[2][10];
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void QMC5883_getValues(int16_t *x,int16_t *y,int16_t *z)
*��������:	   ��ȡ �����Ƶĵ�ǰADCֵ
���������    �������Ӧ�����ָ��	
���������  ��
*******************************************************************************/
void QMC5883_getlastValues(int16_t *x,int16_t *y,int16_t *z) {
  *x = QMC5883_FIFO[0][10];
  *y = QMC5883_FIFO[1][10]; 
  *z = QMC5883_FIFO[2][10]; 
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void QMC5883_mgetValues(float *arry)
*��������:	   ��ȡ У����� ������ADCֵ
���������    �������ָ��	
���������  ��
*******************************************************************************/
uint32_t HMC5883_Last_Update = 0;
void QMC5883_mgetValues(float *arry) {
  int16_t xr,yr,zr;
  uint32_t mtime = micros();  //��ȡʱ��
  //��ȡƵ�� 220hz
  if((HMC5883_Last_Update==0)||((HMC5883_Last_Update+4500)<mtime)||(HMC5883_Last_Update > mtime)){
  QMC5883_getRaw(&xr, &yr, &zr);
  HMC5883_Last_Update = mtime;
  }else {
  	QMC5883_getlastValues(&xr, &yr, &zr);
  }

  arry[0] = (xr-Mag_Offset_X)*Mag_Scale_X;  //Mx
  arry[1] = (yr-Mag_Offset_Y)*Mag_Scale_Y;  //My
  arry[2] = (zr-Mag_Offset_Z)*Mag_Scale_Z;  //Mz
  lastMx = arry[0];
  lastMy = arry[1];
  lastMz = arry[2];
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void QMC5883_init(u8 setmode)
*��������:	   ���� 5883L�Ĺ���ģʽ
���������     ģʽ
���������  ��
*******************************************************************************/
void QMC5883_init(u8 setmode) {
  
  QMC5883_writeReg(0x09, 0xD9); // 8 gauss,
  QMC5883_writeReg(0x0A, 0x00); // -+1.3Ga	  1090LSB/Gauss
  QMC5883_writeReg(0x0B, 0x01);
  QMC5883_writeReg(0x20,0x40);
  QMC5883_writeReg(0x21,0x01);
  
  QMC5883L_update_config();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void QMC5883L_SetUp(void)
*��������:	   ��ʼ�� QMC5883L ʹ֮�������״̬
���������     	
���������  ��
*******************************************************************************/
void QMC5883L_SetUp(void)
{ 
	  char id[3];
	  QMC5883_init(0); //  -+8Ga
	  QMC5883_FIFO_init();
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void QMC5883L_Start_Calib(void)
*��������:	   ��������Ʊ궨
���������     	
���������  ��
*******************************************************************************/
void QMC5883L_Start_Calib(void)
{
	QMC5883_calib = 1;//��ʼ�궨
	QMC5883_maxx = -32400;	//��ԭ���ı궨ֵ���
	QMC5883_maxy = -32400;
	QMC5883_maxz = -32400;
	QMC5883_minx = 32400;
	QMC5883_miny = 32400;
	QMC5883_minz = 32400;
	Mag_Scale_X = 1.0f;
	Mag_Scale_Y = 1.0f;
	Mag_Scale_Z = 1.0f;
	Mag_Offset_X = 0;
	Mag_Offset_Y = 0;
	Mag_Offset_Z = 0;
}

void QMC5883L_update_config(void){
  
	Mag_Offset_X = Config.dMx_offset;
	Mag_Offset_Y = Config.dMy_offset;
	Mag_Offset_Z = Config.dMz_offset;

	Mag_Scale_X = Config.dMx_scale ; 
  Mag_Scale_Y = Config.dMy_scale ;  
  Mag_Scale_Z = Config.dMz_scale ;
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void QMC5883L_Save_Calib(void)
*��������:	  ��������Ʊ궨ֵ ��Flash
���������     	
���������  ��
*******************************************************************************/
void QMC5883L_Save_Calib(void){

	if(QMC5883_maxx == QMC5883_minx)return ;
	if(QMC5883_maxy == QMC5883_miny)return ;
	if(QMC5883_maxz == QMC5883_minz)return ;  //�����������⣬���߸�����û��ת����
	//�������Ʊ궨ֵд�� Flash ����
	Config.dMx_offset = (QMC5883_maxx+QMC5883_minx)/2;
	Config.dMy_offset = (QMC5883_maxy+QMC5883_miny)/2;
	Config.dMz_offset = (QMC5883_maxz+QMC5883_minz)/2;

	Config.dMx_scale = Mag_Scale_X = 1.0f;
	Config.dMy_scale = Mag_Scale_Y = (float)(QMC5883_maxx-QMC5883_minx)/(float)(QMC5883_maxy-QMC5883_miny);
	Config.dMz_scale = Mag_Scale_Z = (float)(QMC5883_maxx-QMC5883_minx)/(float)(QMC5883_maxz-QMC5883_minz); 
    
	Write_config();	 //����ǰ����д��
	QMC5883_calib=0; //�����궨
}	//QMC5883L_Save_Calib()

//------------------End of File----------------------------
