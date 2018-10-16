/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include <error.h>
#include <platform.h>
#include <utils.h>
#include <ctype.h>
#include <errno.h>
#include "bc95.h"
#include "at_cmd.h"


#ifdef NBIOT_DEBUG
#include <stdio.h>
void output_buffer( uint8_t *buffer, int length )
{
    int i;

    if ( length == 0 ) nbiot_printf( "\n" );

    i = 0;
    while ( i < length )
    {
        uint8_t array[16];
        int j;

        nbiot_memmove( array, buffer + i, 16 );
        for ( j = 0; j < 16 && i + j < length; j++ )
        {
            nbiot_printf( "%02X ", array[j] );
            if ( j % 4 == 3 ) nbiot_printf( " " );
        }
        if ( length > 16 )
        {
            while ( j < 16 )
            {
                nbiot_printf( "   " );
                if ( j % 4 == 3 ) nbiot_printf( " " );
                j++;
            }
        }
        nbiot_printf( " " );
        for ( j = 0; j < 16 && i + j < length; j++ )
        {
            if ( isprint( array[j] ) )
                nbiot_printf( "%c", array[j] );
            else
                nbiot_printf( "." );
        }
        nbiot_printf( "\n" );
        i += 16;
    }
}
#endif

#define INVALID_SOCKET (-1)



int nbiot_udp_send( const void   *buff,
                    size_t       size)
{

		   if (NULL == buff)
		{
			return NBIOT_ERR_BADPARAM;
		}
#ifdef NBIOT_DEBUG
			nbiot_printf( "sendto(len = %d)", size );
		//	output_buffer( (uint8_t*)buff, size );
		  printf("%s\r\n",buff);
#endif	
		 ip_SendData((int8_t *)buff,size);		
		 return NBIOT_ERR_OK;

}


int nbiot_udp_recv( void              *buff,
                    size_t             size,
                    size_t            *read)
{

	 *read=0;
	 if (NULL == buff ||NULL == read)
    {
        return NBIOT_ERR_BADPARAM;
    }

           netif_rx(buff,(uint16_t *)read);
            if(*read!=0){ 
							printf("recv:%s\r\n",buff);
            }
	   
    
    return NBIOT_ERR_OK;
}

