#ifndef __QMC5883L_H
#define __QMC5883L_H

#include "stm32f4xx.h"
#include "delay.h"
#include "IOI2C.h"


#define QMC5883L_ADDR 0x1A // 7 bit address of the QMC5883L used with the Wire library
#define QMC5883L_R_XM 0
#define QMC5883L_R_XL 1

#define QMC5883L_R_YM (2)  //!< Register address for YM.
#define QMC5883L_R_YL (3)  //!< Register address for YL.
#define QMC5883L_R_ZM (4)  //!< Register address for ZM.
#define QMC5883L_R_ZL (5)  //!< Register address for ZL.

extern unsigned char QMC5883_calib;
extern int16_t  
		 		lastMx,lastMy,lastMz;
//��ǰ�ų������ֵ����Сֵ
extern int16_t  QMC5883_maxx,QMC5883_maxy,QMC5883_maxz,
		 QMC5883_minx,QMC5883_miny,QMC5883_minz;
extern unsigned char Mag_calib; //��ʼ����ɱ�־
extern float  magic_GRAVITY;

void QMC5883L_SetUp(void);	//��ʼ��
void QMC5883_getID(char id[3]);	//��оƬID
void QMC5883_getRaw(int16_t *x,int16_t *y,int16_t *z); //��ADC
void QMC5883_mgetValues(float *arry); //IMU ר�õĶ�ȡ������ֵ
void QMC5883_getlastValues(int16_t *x,int16_t *y,int16_t *z);
void QMC5883L_Save_Calib(void);
void QMC5883L_Start_Calib(void);
void QMC5883L_update_config(void);
#endif

//------------------End of File----------------------------



