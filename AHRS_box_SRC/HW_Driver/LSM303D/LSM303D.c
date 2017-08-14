
#include "LSM303D.h"
#include "LED.h"

#define Buf_SIZE  20	   //������� 10������ ��ƽ���˲�

int16_t  //lastAx,lastAy,lastAz,	//���µļ��ٶ�ADCֵ
		 lastMx,lastMy,lastMz;	 //���µĴ�����ADCֵ
static int16_t  LSM303_FIFO[6][Buf_SIZE]; //ADC��������
static uint8_t	 Wr_IndexAcc = 0,Wr_IndexMag = 0;
//�����Ʊ궨ֵ
static int16_t  Mag_Offset_X = 0, //ƫ��
				Mag_Offset_Y = 0,
				Mag_Offset_Z = 0;

static float  Mag_Scale_X = 1.0f, //������
	   		  Mag_Scale_Y = 1.0f,
	   		  Mag_Scale_Z = 1.0f;


//��ǰ�ų������ֵ����Сֵ  ���ڱ궨
int16_t  Mag_maxx=0,Mag_maxy=0,Mag_maxz=0,
		 Mag_minx=-0,Mag_miny=-0,Mag_minz=-0;
unsigned char Mag_calib=0; //��ʼ����ɱ�־


//���һ���µ�ֵ�� �¶ȶ��� �����˲�
static void LSM303_NewValAcc(int16_t* buf,int16_t val) {
  	buf[Wr_IndexAcc] = val;
}

//���һ���µ�ֵ�� �¶ȶ��� �����˲�
static void LSM303_NewValMag(int16_t* buf,int16_t val) {
  	buf[Wr_IndexMag] = val;
}

//��ȡ�����ƽ��ֵ
static int16_t LSM303_GetAvg(int16_t* buf){
    int i;
	int32_t	sum = 0;
	for(i=0;i<Buf_SIZE;i++)
		sum += buf[i];
	sum = sum / Buf_SIZE;
	return (int16_t)sum;
}

//����LSM303�ļĴ�����reg�Ĵ�����ַ�� dataΪҪд���ֵ
static void writeReg(u8 reg, u8 data){
	LSM303_CSL();
	SPI1_writeReg(reg,data);
	LSM303_CSH();	
}

//��ʼ��LSM303
void LSM303_Initial(void){
	int16_t temp[3];
	int i;
	SPI1_Configuration();
	//LSM303_readID();
	// Accelerometer
    // anti-alias filter bandwidth 50Hz
    // AFS = 0 (+/- 2 g full scale)
	writeReg(CTRL2, 0xC0);

    // AODR = 1001 (800 Hz ODR); AZEN = AYEN = AXEN = 1 (all axes enabled)
    writeReg(CTRL1, 0x97);
	    
	// Magnetometer
    // M_RES = 11 (high resolution mode); M_ODR = 101 (100 Hz ODR)
    writeReg(CTRL5, 0x74);

    // 0x20 = 0b00100000
    // MFS = 01 (+/- 4 gauss full scale)
    writeReg(CTRL6, 0x20);

    // MLP = 0 (low power mode off); MD = 00 (continuous-conversion mode)
    writeReg(CTRL7, 0x00);
	for(i=0;i<Buf_SIZE;i++){  //��ʼ��������������
		LSM303_readAcc(temp);
		LSM303_readMag(temp);
		}

    LSM303_update_config();	  //װ�ر궨����
//
}

//��оƬID	���ڲ���SPIͨ���Ƿ�����
//��ȷ����0x49
u8 LSM303_readID(void){
	u8 id;
	LSM303_CSL();
	id = SPI1_readReg(0x0F); //��WHO_AM_I
	LSM303_CSH();
	return id;
}

//��ȡ���ٶ�����
void LSM303_readAcc(int16_t *acc){
	u8 buf[6];
	int16_t temp[3];
	LSM303_CSL();
	SPI1_readRegs(OUT_X_L_A|0x40,6,buf);
	LSM303_CSH();
	temp[0] = -(int16_t)(((int16_t)buf[1]) << 8 | buf[0]);  //Ax  �ᷴ��
	temp[1] = -(int16_t)(((int16_t)buf[3]) << 8 | buf[2]);  //Ay
	temp[2] = (int16_t)(((int16_t)buf[5]) << 8 | buf[4]);  //Az
	LSM303_NewValAcc(&LSM303_FIFO[0][0],temp[0]);
	LSM303_NewValAcc(&LSM303_FIFO[1][0],temp[1]);
	LSM303_NewValAcc(&LSM303_FIFO[2][0],temp[2]);
	Wr_IndexAcc = (Wr_IndexAcc + 1) % Buf_SIZE;
	acc[0] = LSM303_GetAvg(&LSM303_FIFO[0][0]);  //Ax
	acc[1] = LSM303_GetAvg(&LSM303_FIFO[1][0]);  //Ay
	acc[2] = LSM303_GetAvg(&LSM303_FIFO[2][0]);  //Az
	lastAx = acc[0];
	lastAy = acc[1];
	lastAz = acc[2];
}

//��ȡ����������
void LSM303_readMag(int16_t *Mag){
	u8 buf[6];
	int16_t temp[3];
	LSM303_CSL();
	SPI1_readRegs(D_OUT_X_L_M|0x40,6,buf);
	LSM303_CSH();
	temp[0] = -(int16_t)(((int16_t)buf[1]) << 8 | buf[0]); //Mx
	temp[1] = -(int16_t)(((int16_t)buf[3]) << 8 | buf[2]); //My
	temp[2] = (int16_t)(((int16_t)buf[5]) << 8 | buf[4]);  //Mz
	LSM303_NewValMag(&LSM303_FIFO[3][0],temp[0]);
	LSM303_NewValMag(&LSM303_FIFO[4][0],temp[1]);
	LSM303_NewValMag(&LSM303_FIFO[5][0],temp[2]);
	Wr_IndexMag = (Wr_IndexMag + 1) % Buf_SIZE;
	Mag[0] = (LSM303_GetAvg(&LSM303_FIFO[3][0])-Mag_Offset_X)*Mag_Scale_X;  //Mx
	Mag[1] = (LSM303_GetAvg(&LSM303_FIFO[4][0])-Mag_Offset_Y)*Mag_Scale_Y;  //My
	Mag[2] = (LSM303_GetAvg(&LSM303_FIFO[5][0])-Mag_Offset_Z)*Mag_Scale_Z;  //Mz
	lastMx = Mag[0];
	lastMy = Mag[1];
	lastMz = Mag[2];

	if(Mag_calib){//У����Ч�Ļ� �ɼ� ���ֵ��Сֵ��
		if(Mag_minx>Mag[0])Mag_minx=(int16_t)Mag[0];
		if(Mag_miny>Mag[1])Mag_miny=(int16_t)Mag[1];
		if(Mag_minz>Mag[2])Mag_minz=(int16_t)Mag[2];

		if(Mag_maxx<Mag[0])Mag_maxx=(int16_t)Mag[0];
		if(Mag_maxy<Mag[1])Mag_maxy=(int16_t)Mag[1];
		if(Mag_maxz<Mag[2])Mag_maxz=(int16_t)Mag[2];
	}
}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void MagL_Start_Calib(void)
*��������:	   ��������Ʊ궨
���������     	
���������  ��
*******************************************************************************/
void LSM303_Start_Calib(void)
{
	Mag_calib=1;//��ʼ�궨
	Mag_maxx=-14096;	//��ԭ���ı궨ֵ���
	Mag_maxy=-14096;
	Mag_maxz=-14096;
	Mag_minx=14096;
	Mag_miny=14096;
	Mag_minz=14096;
	Mag_Scale_X = 1.0f;
	Mag_Scale_Y = 1.0f;
	Mag_Scale_Z = 1.0f;
	Mag_Offset_X = 0;
	Mag_Offset_Y = 0;
	Mag_Offset_Z = 0;
}

void LSM303_update_config(void){
	//ȡ�����Ƶı궨ֵ����
	Mag_Offset_X = Config.dMx_offset ;  
    Mag_Offset_Y = Config.dMy_offset ; 
    Mag_Offset_Z = Config.dMz_offset ; 

    Mag_Scale_X = Config.dMx_scale ; 
    Mag_Scale_Y = Config.dMy_scale ;  
    Mag_Scale_Z = Config.dMz_scale ;

}

/**************************ʵ�ֺ���********************************************
*����ԭ��:	  void MagL_Save_Calib(void)
*��������:	  ��������Ʊ궨ֵ ��Flash
���������     	
���������  ��
*******************************************************************************/
void LSM303_Save_Calib(void){
	//�������Ʊ궨ֵд�� Flash ����
	Config.dMx_offset = Mag_Offset_X = (Mag_maxx+Mag_minx)/2;
	Config.dMy_offset = Mag_Offset_Y = (Mag_maxy+Mag_miny)/2;
	Config.dMz_offset = Mag_Offset_Z = (Mag_maxz+Mag_minz)/2;

	Config.dMx_scale = Mag_Scale_X = 1.0f;
	Config.dMy_scale = Mag_Scale_Y = (float)(Mag_maxx-Mag_minx)/(float)(Mag_maxy-Mag_miny);
	Config.dMz_scale = Mag_Scale_Z = (float)(Mag_maxx-Mag_minx)/(float)(Mag_maxz-Mag_minz); 

	Write_config();//д��flash�б���
	Mag_calib=0; //�����궨
}	//Mag_Save_Calib()


//------------------End of File----------------------------
