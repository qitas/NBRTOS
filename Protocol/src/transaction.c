/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"
#include "stdio.h"

int nbiot_transaction_add( nbiot_device_t              *dev,
	                       bool                         update,
                           const uint8_t               *buffer,
                           size_t                       buffer_len,
                           nbiot_transaction_callback_t callback )
{
    uint8_t *temp;
    nbiot_transaction_t *transaction;

    transaction = (nbiot_transaction_t*)nbiot_malloc( sizeof(nbiot_transaction_t) );
    if ( !transaction )
    {
        return NBIOT_ERR_NO_MEMORY;
    }

    temp = nbiot_malloc( buffer_len );
    if ( !temp )
    {
        nbiot_free( transaction );

        return NBIOT_ERR_NO_MEMORY;
    }
    
    nbiot_memzero( transaction, sizeof(nbiot_transaction_t) );
    nbiot_memmove( temp, buffer, buffer_len );
		if(update==true)
			 transaction->mid = dev->first_mid;
		else
    transaction->mid = dev->next_mid;
    transaction->buffer = temp;
    transaction->buffer_len = buffer_len;
    transaction->callback = callback;
    dev->transactions = (nbiot_transaction_t*)NBIOT_LIST_ADD( dev->transactions, transaction );

    return NBIOT_ERR_OK;
}

int nbiot_transaction_del( nbiot_device_t *dev,
	                           bool            succ,
                               uint16_t        mid )
{
    nbiot_transaction_t *transaction = NULL;

    dev->transactions = (nbiot_transaction_t*)NBIOT_LIST_DEL( dev->transactions,
                                                              mid,
                                                              &transaction );
    if ( !transaction )
    {
        return NBIOT_ERR_NOT_FOUND;
    }

    if (transaction->callback )
    {
        transaction->callback( dev,
                               succ,
                               transaction->ack );
   }

    nbiot_free( transaction->buffer );
    nbiot_free( transaction );

    return NBIOT_ERR_OK;
}

void nbiot_transaction_step( nbiot_device_t *dev,
                             time_t          now,
                             uint8_t        *buffer,
                             size_t          buffer_len )
{
    nbiot_transaction_t *next=NULL;  
   	nbiot_transaction_t *transaction = dev->transactions;
    while ( transaction )
    {
        next = transaction->next;
        if ( transaction->time <= now )
        {
            bool max_counter = false;
            if ( !transaction->ack )
            {
                if ( 0 == transaction->counter )
                {
                    transaction->counter = 1;
                    transaction->time = now + COAP_RESPONSE_TIMEOUT;
                }
                else if ( COAP_MAX_RETRANSMIT+1 >= transaction->counter )
                {
                    /* retry send */
				
									      printf("retramit:");
					             	nbiot_udp_send(transaction->buffer,transaction->buffer_len);
	
                  //  transaction->time += COAP_RESPONSE_TIMEOUT << (++transaction->counter);
                  transaction->time += COAP_RESPONSE_TIMEOUT;
                  transaction->counter=transaction->counter+1;
                }
                else
                {
                    max_counter = true;
                }
            }

            if ( max_counter )
            {
                nbiot_transaction_del( dev,
							           false,
                                       transaction->mid );
            }
        }

        transaction = next;
    }
}
