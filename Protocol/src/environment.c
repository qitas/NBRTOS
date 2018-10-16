/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
#include "platform.h"
#include "bc95.h"
#include "usart1.h"
#include "usart3.h"
#include "delay.h"

#include "led.h"
#include "beep.h"
#include "BH1750.h"
#include "i2c.h"
#include "key.h"
#include "adxl345.h"
#include "BMP180.h"
#include "SHT20.h"

void Peripheral_Init(void)
{
	
	Led_Init();											//LED接口初始化
	
//	Beep_Init();                                         //蜂鸣器初始化
	
// 	Key_Init();											//按键接口初始化
	

	IIC_Init();											//I2C总线初始化
	
	IIC_1750_Init();                                    //BH1750初始化
	
	ADXL345_Init();                                       //ADX345初始化
	
}

static bool _nbiot_init_state = false;
void nbiot_init_environment()
{
    if ( !_nbiot_init_state )
    {
     nbiot_time_init();
     USART1_Init();
     USART3_Init(); 
     Peripheral_Init();	 	
     mDelay(5000);			
	   netdev_init();
     _nbiot_init_state = true;
    }
}

void nbiot_clear_environment( void )
{
    if ( _nbiot_init_state )
    {	
      SendCmd("AT+MIPLDELETE=0","OK",300,0,10); 		
       _nbiot_init_state = false;
    }
}

void nbiot_reset(void)
{

 NVIC_GenerateSystemReset();

}

