/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
#include <stdio.h>
#include "internal.h"

#ifdef BC26
#include "BC26.h"
#elif BC28
#include "BC28.h"
#else
#include "BC95.h"
#endif

//#include "led.h"

#define REGISTER_START  1
#define REGISTER_UPDATE 2
#define DEREGISTER      3



static void registraction_reply( nbiot_device_t *dev,
                                 bool            suc,
                                 bool            ack )
{
    if ( dev->state == STATE_REG_PENDING )
    {
        dev->registraction = nbiot_time();	
        if ( suc == true )
        {
            dev->state = STATE_REGISTERED;
        }
        else
        {
            dev->state = STATE_REG_FAILED;
        }
    }
}

int nbiot_register_start( nbiot_device_t *dev,
	                        uint8_t        *buffer,
                          size_t          buffer_len)
{
	    size_t          length=0;
#ifdef NBIOT_BOOTSTRAP
    if ( dev->state == STATE_BS_FINISHED )
#else
    if ( dev->state == STATE_REG_FAILED ||
         dev->state == STATE_DEREGISTERED )
#endif
    {
      length=nb_register_request(buffer,buffer_len);
		 if(length>0){
		 	dev->state = STATE_REG_PENDING;
		  nbiot_transaction_add( dev,
								true,
								buffer,
								length,
								registraction_reply );
	   }else{

            return COAP_INTERNAL_SERVER_ERROR_500; 

	   }
		
    }

    return COAP_NO_ERROR;
}

static void registraction_update_reply( nbiot_device_t *dev,
                                        bool            suc,
                                        bool            ack )
{
    if ( dev->state == STATE_REG_UPDATE_PENDING )
    {
        dev->registraction = nbiot_time();
        if (suc == true)
        {
            dev->state = STATE_REGISTERED;
        }
        else
        {
            dev->state = STATE_REG_FAILED;
        }
    }
}

int nbiot_register_update( nbiot_device_t *dev,
	                         uint8_t        *buffer,
                           size_t          buffer_len)
{
    size_t          length=0;
    if (dev->state == STATE_REG_UPDATE_NEEDED )
    {
			   printf("update\r\n");
         length=nb_register_update(dev->life_time,buffer,buffer_len);
		 if(length>0){
		 	  dev->state = STATE_REG_UPDATE_PENDING;
			   nbiot_transaction_add( dev,
			                          true,
									             buffer,
									             length,
									             registraction_update_reply );
	   }else{

            return COAP_INTERNAL_SERVER_ERROR_500; 

	   }
		
    }

    return COAP_NO_ERROR;
}

static void deregister_reply( nbiot_device_t    *dev,
                              bool               suc,
                              bool               ack)
{
    if ( dev->state == STATE_DEREG_PENDING )
    {
        dev->state = STATE_DEREGISTERED;
    }
}

int nbiot_deregister( nbiot_device_t *dev,
                      uint8_t        *buffer,
                      size_t          buffer_len )

{
     size_t          length=0;
    if ( dev->state == STATE_REGISTERED ||
         dev->state == STATE_REG_UPDATE_NEEDED ||
         dev->state == STATE_REG_UPDATE_PENDING )
    {
		length=nb_close_request(buffer,buffer_len);
		 if(length>0){
           dev->state = STATE_DEREG_PENDING;
			   nbiot_transaction_add( dev,
			                          true,
									  buffer,
									  length,
									  deregister_reply);
    }else{

            return COAP_INTERNAL_SERVER_ERROR_500; 

	   }
    }

    return COAP_NO_ERROR;
}

void nbiot_register_step( nbiot_device_t *dev,
                                time_t         now,
                                uint8_t        *buffer,
                                 size_t        buffer_len )
{
     int next_update= dev->life_time;
    if ( dev->state == STATE_REGISTERED )
    {
          next_update = next_update >> 2;
        if (dev->registraction + next_update <= now)
        {
            nbiot_register_update(dev,buffer,buffer_len);
        }
    }
     if (dev->state == STATE_REG_UPDATE_NEEDED)
     {
            nbiot_register_update(dev,buffer,buffer_len);
     }
		if ( dev->state == STATE_REG_FAILED)
    {
			//Led1_Set(LED_OFF);
      nbiot_register_start(dev,buffer,buffer_len);         
    }
}
