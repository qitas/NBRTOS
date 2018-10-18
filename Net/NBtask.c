/**
 * Copyright (c) 2018 Qitas.
 * All rights reserved.
**/
#include <nbiot.h>
#include <utils.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "gpio.h"
#include "usart.h"
#include "BC95.h"
#include "NBtask.h"

nbiot_value_t temp;   /* 温度 */
nbiot_value_t humi;   /* 湿度 */
nbiot_value_t illumi; //光照
nbiot_value_t press;  //压力
nbiot_value_t LED;


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
			 //Led1_Set(data->value.as_bool);
		}
}

void read_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{

		   
        if(objid==3301&&instid==0&&resid==5700){		    
                  //BH1750_test();				 
				  //illumi.value.as_float=result_lx;
		}else if(objid==3303&&instid==0&&resid==5700){
			      //SHT20_INFO sht20={0};					 
                  //sht20=SHT20_GetValue();
				  //temp.value.as_float=(int64_t)sht20.tempreture;
		
		}else if(objid==3304&&instid==0&&resid==5700){
			      //SHT20_INFO sht20={0};				 
                  //sht20=SHT20_GetValue();
				  //humi.value.as_float=(int64_t)sht20.humidity;
			
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

char uri[] = "coap://120.79.63.40:5683";
char *serv_addr="120.79.63.40";
const char endpoint_name[] = "863703031970192;460072419808827";
nbiot_device_t *dev = NULL;
uint8_t inscount = 0;

uint8_t last_time=0;
uint8_t cur_time=0;


void res_update(time_t interval)
{
	 if(cur_time>=last_time+interval){ 
            cur_time=0;
            last_time=0;				 
						temp.flag |= NBIOT_UPDATED;		
            humi.flag |= NBIOT_UPDATED;					 
		}else if(cur_time==0&&last_time==0){
			
			    cur_time=nbiot_time();
			    last_time=cur_time;
			
		}else{
			
			 cur_time=nbiot_time();
			   
		} 	 

}	
void get_data(void)
{
	int ret;
	LED.type = NBIOT_BOOLEAN;
	LED.flag = NBIOT_READABLE|NBIOT_WRITABLE;
	ret = nbiot_resource_add( dev,
							  3311,
							  0,
							  5850,
							  1,
							  1,
							  &LED );
	if( ret )
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
	printf("IN 2 NBTask");
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
		
}

static bool _nbiot_init_state = false;

void StartNBTask(void const * argument)
{
    int life_time = 300;
		int ret;
	  if ( !_nbiot_init_state )
    {
     //nbiot_time_init();
     //osDelay(5000);		
	   netdev_init();
    }
		printf("IN NBTask");		
		ret = nbiot_device_create( &dev,
							   endpoint_name,
								 uri,
							   life_time,
							   write_callback,
							   read_callback,
							   execute_callback);
		if ( ret )
		{
				nbiot_device_destroy( dev );
				printf( "device create failed, code = %d.\r\n", ret );
		}
		ret = nbiot_device_connect(dev,60);
		if (ret)
		{
				nbiot_device_close( dev, 100);
				nbiot_device_destroy( dev );
				printf( "connect OneNET failed.\r\n" );
		}
		else{
				printf( "connect OneNET success.\r\n" );	 
		}
		for(;;)
		{   
				 printf("IN 6 NBTask");					
				 ret = nbiot_device_step( dev, 1);
				 if ( ret )
				 {
					 printf( "device step error, code = %d.\r\n", ret );
				 } 
				 res_update(30);							
		}
}
