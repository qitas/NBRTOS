/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
#include <nbiot.h>
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "delay.h"
#include "key.h"
#include "led.h"
#include "platform.h"
#include "SHT20.h"
#include "adxl345.h"
#include "BH1750.h"
#include "BMP180.h"

nbiot_value_t temp;   /* 温度 */
nbiot_value_t humi;   /* 湿度 */
nbiot_value_t illumi; //光照
nbiot_value_t press;  //压力
nbiot_value_t altitude; //海拔
nbiot_value_t acceleX;//三轴加速x
nbiot_value_t acceleY;//三轴加速y
nbiot_value_t acceleZ;//三轴加速z
nbiot_value_t LED;

extern float result_lx;//光照
extern _bmp180 bmp180;//压力/海拔
extern  ADXL345_INFO adxlInfo;//三轴加速

void write_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{
    printf( "write /%d/%d/%d：%d\r\n",
                  objid,
                  instid,
                  resid,data->value.as_bool );
    if(objid==3311&&instid==0&&resid==5850)
		{  
			 Led1_Set(data->value.as_bool);
		}
		/*
		else if(objid==3301&&instid==0&&resid==5700){		    
		      illumi.flag |= NBIOT_UPDATED;
          BH1750_test();				 
				  illumi.value.as_float=result_lx;
		}else if(objid==3303&&instid==0&&resid==5700){
			      SHT20_INFO sht20={0};
		        temp.flag |= NBIOT_UPDATED;						 
            sht20=SHT20_GetValue();
				    temp.value.as_float=(int64_t)sht20.tempreture;
		
		}else if(objid==3304&&instid==0&&resid==5700){
			      SHT20_INFO sht20={0};
			      humi.flag |= NBIOT_UPDATED;					 
            sht20=SHT20_GetValue();
				    humi.value.as_float=(int64_t)sht20.humidity;
    }
		*/
}

void read_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{

		   
        if(objid==3301&&instid==0&&resid==5700){		    
                  BH1750_test();				 
				  illumi.value.as_float=result_lx;
		}else if(objid==3303&&instid==0&&resid==5700){
			      SHT20_INFO sht20={0};					 
                  sht20=SHT20_GetValue();
				  temp.value.as_float=(int64_t)sht20.tempreture;
		
		}else if(objid==3304&&instid==0&&resid==5700){
			      SHT20_INFO sht20={0};				 
                  sht20=SHT20_GetValue();
				  humi.value.as_float=(int64_t)sht20.humidity;
			
    }
	
}

void execute_callback( uint16_t       objid,
                       uint16_t       instid,
                       uint16_t       resid,
                       nbiot_value_t *data,
                       const void    *buff,
                       size_t         size )
{
    printf( "execute /%d/%d/%d\r\n",
                  objid,
                  instid,
                  resid );
}

char uri[] = "coap://183.230.40.40:5683";
char *serv_addr="183.230.40.40";
const char endpoint_name[] = "863703031970192;460072419808827";
nbiot_device_t *dev = NULL;
uint8_t inscount = 0;

time_t last_time=0;
time_t cur_time=0;


void res_update(time_t interval)
{
	     SHT20_INFO sht20;
       if(cur_time>=last_time+interval){ 
            cur_time=0;
            last_time=0;				 
             
				    temp.flag |= NBIOT_UPDATED;		
            humi.flag |= NBIOT_UPDATED;					 
            sht20=SHT20_GetValue();
				    temp.value.as_float=(int64_t)sht20.tempreture;
				    humi.value.as_float=(int64_t)sht20.humidity;
       
				    illumi.flag |= NBIOT_UPDATED;
            BH1750_test();				 
				    illumi.value.as_float=result_lx;
				 /*
				    press.flag |= NBIOT_UPDATED;		
            altitude.flag |= NBIOT_UPDATED;	
				    BMP180_test();
				    press.value.as_float=bmp180.p;
				    altitude.value.as_float=bmp180.altitude;
				    
				    acceleX.flag |= NBIOT_UPDATED;		
            acceleY.flag |= NBIOT_UPDATED;
            acceleZ.flag |= NBIOT_UPDATED;							
            ADXL345_GetValue();
						acceleX.value.as_float=adxlInfo.incidence_Xf;
						acceleY.value.as_float=adxlInfo.incidence_Yf;
						acceleZ.value.as_float=adxlInfo.incidence_Zf;
         */
			}else if(cur_time==0&&last_time==0){
			
			    cur_time=nbiot_time();
			    last_time=cur_time;
			
			}else{
			
			   cur_time=nbiot_time();
			   
			} 	 

}	

/**
  * @brief System Clock Configuration
  * @retval None
  */
#include "stm32f10x_rcc.h"

void SystemClock_Config(void)
{
			RCC_DeInit(); 

			RCC_HSEConfig(RCC_HSE_ON); 

			while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET); 

			RCC_HCLKConfig(RCC_SYSCLK_Div1);     

			RCC_PCLK1Config(RCC_HCLK_Div2);  

			RCC_PCLK2Config(RCC_HCLK_Div1);   

			RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);  

			RCC_PLLCmd(ENABLE);

			while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);  

			RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);  

			while(RCC_GetSYSCLKSource()!= 0x08); 
}
#include "usart1.h"
#include "usart3.h"
#include "led.h"
#include "delay.h"

int main(void)
{
		SystemClock_Config();
		//USART1_Init();
    //USART3_Init(); 
		nbiot_init_environment();  
		//Led_Init();
		while(1){
			Led1_Set(LED_OFF);
			Led2_Set(LED_OFF);
			Led3_Set(LED_OFF);
			Led4_Set(LED_OFF);
			//printf( "qitas test code\r\n");
			mDelay(50);	
			Led1_Set(LED_ON);
			Led2_Set(LED_ON);
			Led3_Set(LED_ON);
			Led4_Set(LED_ON);
		}
}

int main1(void)
{
		 int life_time = 300;
		 int ret;
		 nbiot_init_environment();  
		 printf( "qitas test code\r\n");
		 ret = nbiot_device_create( &dev,
																 endpoint_name,
																 uri,
																 life_time,
																 write_callback,
																 read_callback,
																 execute_callback );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(/3200/0/5750) failed, code = %d.\r\n", ret );
        }
				LED.type = NBIOT_BOOLEAN;
        LED.flag = NBIOT_READABLE|NBIOT_WRITABLE;
        ret = nbiot_resource_add( dev,
                                  3311,
                                  0,
                                  5850,
				                          1,
				                          1,
                                  &LED );
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(LED) failed, code = %d.\r\n", ret );
        }
        temp.type = NBIOT_FLOAT;
        temp.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3303,
                                  0,
                                  5700,
				                          1,
				                          1,
                                  &temp );//温度
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(temp) failed, code = %d.\r\n", ret );
        }
        humi.type = NBIOT_FLOAT;
        humi.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3304,
                                  0,
                                  5700,
				                          1,
				                          1,
                                  &humi );//湿度
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(humi) failed, code = %d.\r\n", ret );
        }
        illumi.type = NBIOT_FLOAT;
        illumi.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3301,
                                  0,
                                  5700,
				                          1,
				                          1,
                                  &illumi );//光照
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(illumi) failed, code = %d.\r\n", ret );
        }
				/*
        press.type = NBIOT_FLOAT;
        press.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  3,
                                  5600,
                                  &press );//压力
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(press) failed, code = %d.\r\n", ret );
        }
				
        altitude.type = NBIOT_FLOAT;
        altitude.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  4,
                                  5600,
                                  &altitude );//海拔
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(temp) failed, code = %d.\r\n", ret );
        }
				
        acceleX.type = NBIOT_FLOAT;
        acceleX.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  5,
                                  5600,
                                  &acceleX );//x轴
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(acceleX) failed, code = %d.\r\n", ret );
        }
				acceleY.type = NBIOT_FLOAT;
        acceleY.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  6,
                                  5600,
                                  &acceleY );//y轴
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(acceleY) failed, code = %d.\r\n", ret );
        }
				acceleZ.type = NBIOT_FLOAT;
        acceleZ.flag = NBIOT_READABLE;
        ret = nbiot_resource_add( dev,
                                  3202,
                                  7,
                                  5600,
                                  &acceleZ );//z轴
        if ( ret )
        {
            nbiot_device_destroy( dev );
            printf( "device add resource(acceleZ) failed, code = %d.\r\n", ret );
        }
				*/
        ret = nbiot_device_connect(dev,60);

			if ( ret )
			{
					nbiot_device_close( dev, 100);
					nbiot_device_destroy( dev );
					printf( "connect OneNET failed.\r\n" );
					nbiot_reset();
			}else{
					Led4_Set(LED_ON);
					printf( "connect OneNET success.\r\n" );
			 
			}
			do{        
						ret = nbiot_device_step( dev, 1);
						if ( ret )
						{
							 printf( "device step error, code = %d.\r\n", ret );
						} 
						res_update(30);					 		      
    } while(1);
    nbiot_clear_environment();
    return 0;
}
