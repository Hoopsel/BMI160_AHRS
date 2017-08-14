#ifndef __MAX21100_H
#define __MAX21100_H

#include "SPI2.h"
#include "eeprom.h"
#include "delay.h"



//MAX21100Ƭѡ�źſ���
#define MAX21100_CSH()  ; 
#define MAX21100_CSL()  ; 

extern int16_t lastGx,lastGy,lastGz;//���µ�������ٶ�ADCֵ��
extern int16_t  lastAx,lastAy,lastAz;//���µļ��ٶ�ADCֵ

void MAX21100_Initial(void);  //��ʼ��MAX21100
u8 MAX21100_readID(void);	  //��id,��ȷ����0xB1[1011 0001]
void MAX21100_InitGyro_Offset(void);   //�ɼ���ƫ����
void MAX21100_readAccGyro(int16_t *data);
int16_t MAX21100_get_ACCMAX(unsigned char ais);
void MAX21100_Reset_ACC_Offset(void);
void ACC_Cal(unsigned char ch);
void ACC_Save_cal(void);

#endif

//------------------End of File----------------------------
