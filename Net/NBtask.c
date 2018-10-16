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


void write_callback( uint16_t       objid,
                     uint16_t       instid,
                     uint16_t       resid,
                     nbiot_value_t *data )
{
    printf( "write /%d/%d/%d��%d\r\n",
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
int NBTask(void)
{
     int life_time = 300;
	 int ret;
     nbiot_init_environment();  
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
							  &temp );//�¶�
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
							  &humi );//ʪ��
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
							  &illumi );//����
	if ( ret )
	{
		nbiot_device_destroy( dev );
		printf( "device add resource(illumi) failed, code = %d.\r\n", ret );
	}
	ret = nbiot_device_connect(dev,60);
	if ( ret )
	{
		nbiot_device_close( dev, 100);
		nbiot_device_destroy( dev );
		printf( "connect OneNET failed.\r\n" );
		nbiot_reset();
	}
	else{
		//Led4_Set(LED_ON);
		printf( "connect OneNET success.\r\n" );
			 
			}
	while(1)
	{        
			 ret = nbiot_device_step( dev, 1);
			 if ( ret )
			 {
				 printf( "device step error, code = %d.\r\n", ret );
			 } 
			res_update(30);					 
				  
	}
    nbiot_clear_environment();
    return 0;
}