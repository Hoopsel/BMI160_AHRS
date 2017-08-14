#ifndef __ATM32ADC_H
#define __ATM32ADC_H

#include "stm32f4xx.h"


extern int16_t lastVOL;
// ģ��ת������ ��API ����
extern void ADC_Voltage_initial(void); //��ʼ�������ϵ��ʱ�����һ�Ρ�֮�� ADC���Զ��ɼ�����
extern int16_t Get_Bat_Vol(void);  //��ȡ��ǰ�ĵ�ص�ѹֵ�� ��λ 0.01V
extern uint8_t Is_BAT_LOW(void);
#endif

//------------------End of File----------------------------
