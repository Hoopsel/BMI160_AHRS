#ifndef __AD7689_H
#define __AD7689_H

#include "stm32f4xx.h"
#include "delay.h"
#include "SPI2.h"

extern int16_t 
	  Accel_Xint,  //ADCֵ
	  Accel_Yint,	
	  Accel_Zint,
	  Gyro_Xint,   
	  Gyro_Yint,	
	  Gyro_Zint ;

extern float Accel_X,  //���ٶ�X��, ��λg [9.8m/S^2]
			  Accel_Y,	//���ٶ�Y��, ��λg [9.8m/S^2]
			  Accel_Z,	//���ٶ�Z��, ��λg [9.8m/S^2]
			  Gyro_X,   //���ٶ�X��, ��λdps [��ÿ��]
			  Gyro_Y,	//���ٶ�Y��, ��λdps [��ÿ��]
			  Gyro_Z	//���ٶ�Z��, ��λdps [��ÿ��]
			  ;
extern int16_t lastAx,lastAy,lastAz,
		lastGx,lastGy,lastGz;
extern int16_t acc_vector_tran;
extern float Transe_ax,Transe_ay,Transe_az,
		Transe_gx,Transe_gy,Transe_gz,transe_Data[6],transe_gyro,transe_acc;

void Gyro_Initial_Offset(void);
void Dof6_Update(void);	//����6��Ĵ���������
void Reset_ACC_Offset(void);
int16_t get_ACCMAX(unsigned char aisx);
void Gyro_update_config(void);
#endif

//------------------End of File----------------------------


