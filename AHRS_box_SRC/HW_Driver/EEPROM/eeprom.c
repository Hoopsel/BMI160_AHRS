/* eeprom.c file
��д�ߣ�lisn3188
��ַ��www.chiplab7.com
����E-mail��lisn3188@163.com
���뻷����MDK-Lite  Version: 4.23
����ʱ��: 2012-05-05
���ԣ� ���������ڵ���ʵ���ҵ�mini IMU����ɲ���
���ܣ�
��Flash����EEPROM ���ڱ���ƫ�úͱ궨����
------------------------------------
 */			  

#include "eeprom.h"
#include "delay.h"
#include "IOI2C.h"

#define  AT24C02_ADDR	0xA0
struct data_map Config;

uint8_t  eep_buf[256];

void read_AT24C(void){
	IICreadBytes(AT24C02_ADDR,0,256,eep_buf);
}


void Write_AT24C(void){
	int i;
	u8  ADDR=0,*ptr = eep_buf;
	for(i=0;i<32;i++){
		IICwriteBytes(AT24C02_ADDR,ADDR,8,ptr);
	   	ADDR += 8;
		ptr += 8;
		delay_ms(6); //ҳд����ʱ��������ʱ5ms
	}
}


void load_define(void){
//	int i;
	Config.is_good = 0xA55A;
	Config.dGx_offset = 0;
	Config.dGy_offset = 0;
	Config.dGz_offset = 0;

	Config.dMx_offset = 0;
	Config.dMy_offset = 0;
	Config.dMz_offset = 0;

	Config.dMx_scale = 1.0f;
	Config.dMy_scale = 1.0f;
	Config.dMz_scale = 1.0f;
	Config.dAx_offset = 0;
	Config.dAy_offset = 0;
	Config.dAz_offset = 0;

	Config.dAx_scale = 1.0;
	Config.dAy_scale = 1.0;
	Config.dAz_scale = 1.0;

	Config.File_index = 1;
}

static void Read_config(void){
	int i;
	u8 *ptr;
	IICreadBytes(AT24C02_ADDR,0,256,eep_buf);
	ptr = &Config.demmy;
	for(i=0 ; i< sizeof(Config);i++){
		*ptr = eep_buf[i];
		ptr++;
	}
}

void load_config(void){
	Read_config();
	if(Config.is_good != (int16_t)0xA55A){ //������Ч
		load_define();
		Write_config();
	}
}

void Write_config(void){
	int16_t i;
	u8 *ptr = &Config.demmy;
	//�������ݵ�buf
	for(i=0 ; i< sizeof(Config);i++){
		eep_buf[i] = *ptr;
		ptr++;
	}
	Write_AT24C(); //д��
}

void New_Filed(void){
 	Config.File_index ++;
	if(Config.File_index>9999)Config.File_index = 1;
	Write_config();
}

void Get_file_name(uint8_t *name){
	int i;
	uint8_t temp[13]={"/AHRS0001.txt"};
	temp[5] = 	(Config.File_index%10000)/1000 + 0x30;
	temp[6] = 	(Config.File_index%1000)/100 + 0x30;
	temp[7] = 	(Config.File_index%100)/10 + 0x30;
	temp[8] = 	(Config.File_index%10) + 0x30;
	for(i=0;i<13;i++){
		*name = temp[i];
		name ++;
	}
	*name = 0;
	New_Filed();
}


//------------------End of File----------------------------
