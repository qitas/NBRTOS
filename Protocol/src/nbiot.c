/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#include "internal.h"
#include "stdio.h"
#include "NB_BC95.h"
#include "at_cmd.h"
//#include "led.h"

uint32_t array[2][3]={0};
int nbiot_device_create( nbiot_device_t         **dev,
                         const char              *endpoint_name,
                         const char               *uri,
                         int                      life_time,
                         nbiot_write_callback_t   write_func,
                         nbiot_read_callback_t    read_func,
                         nbiot_execute_callback_t execute_func )
{
    nbiot_device_t *tmp;
    tmp = (nbiot_device_t*)nbiot_malloc( sizeof(nbiot_device_t) );
    if ( !tmp )
    {
        return NBIOT_ERR_NO_MEMORY;
    }
    else
    {
        nbiot_memzero( tmp, sizeof(nbiot_device_t) );
    }
    
    *dev = tmp;
		tmp->next_mid = nbiot_rand();
		tmp->first_mid = tmp->next_mid;
    tmp->life_time = life_time;
    tmp->write_func = write_func;
		tmp->read_func = read_func;
    tmp->execute_func = execute_func;
    tmp->endpoint_name = endpoint_name;	
		SendCmd("AT+MIPLCREATE\r\n","OK",300,0,10);
    return NBIOT_ERR_OK;
}


static void device_free_nodes( nbiot_device_t *dev )
{
    nbiot_node_t *obj;
    nbiot_node_t *inst;

    obj = dev->nodes;
    while ( obj )
    {
        inst = (nbiot_node_t*)obj->data;
        while ( inst )
        {
            NBIOT_LIST_FREE( inst->data );
			bc95_delobj( obj->id);
            inst = inst->next;
        }
        NBIOT_LIST_FREE( obj->data );
        obj = obj->next;
    }

    NBIOT_LIST_FREE( dev->nodes );
    dev->nodes = NULL;
}

static void device_free_observes( nbiot_device_t *dev )
{
    nbiot_observe_t *obj;
    nbiot_observe_t *inst;

    obj = dev->observes;
    while ( obj )
    {
        inst = (nbiot_observe_t*)obj->list;
        while ( inst )
        {
            NBIOT_LIST_FREE( inst->list );
            inst = inst->next;
        }

        NBIOT_LIST_FREE( obj->list );
        obj = obj->next;
    }

    NBIOT_LIST_FREE( dev->observes );
    dev->observes = NULL;
}

static void device_free_transactions( nbiot_device_t *dev )
{
    nbiot_transaction_t *transaction;

    transaction = dev->transactions;
    while ( transaction )
    {
        nbiot_free( transaction->buffer );
        transaction = transaction->next;
    }

    NBIOT_LIST_FREE( dev->transactions );
    dev->transactions = NULL;
}

void nbiot_device_destroy( nbiot_device_t *dev )
{
    device_free_transactions( dev );
    device_free_observes( dev );
    device_free_nodes( dev );
    nbiot_free( dev );
}

static void handle_read( nbiot_device_t    *dev,
                         nbiot_uri_t       *uri,
                         uint8_t           *buffer,
                         size_t             buffer_len
                         )
{
    do
    {
        int ret;
        nbiot_node_t *node;
			  
        node = nbiot_node_find( dev, uri );
        if ( !node )
        {
            break;
        }
		if(dev->read_func!=NULL)
		 (dev->read_func)(uri->objid,uri->instid,uri->resid,node->data);
        ret = nbiot_node_read( node,
                               uri,
				                       uri->flag,
                               buffer,
				                        buffer_len,
                               false );
		nbiot_send_buffer(uri,buffer,ret,false);
			 
    } while (0);
}

static void handle_write( nbiot_device_t        *dev,
                          nbiot_uri_t            *uri,
                          uint16_t               ackid,
                          uint8_t                *buffer,
                          size_t                 buffer_len,
                          nbiot_write_callback_t write_func )
{
    do
    {
        int ret;
        nbiot_node_t *node;

        node = nbiot_node_find( dev, uri );
        if ( !node )
        {
            bc95_write_rsp(0, ackid);
            break;
        }

        /* write */
        ret = nbiot_node_write( node,
                                uri,
                                ackid,
                                buffer,
                                buffer_len,
                                write_func );
    } while (0);
}

static void handle_execute( nbiot_device_t          *dev,
                            nbiot_uri_t             *uri,
                            uint16_t                ackid,
                            uint8_t                *buffer,
                            size_t                 buffer_len,
                            nbiot_execute_callback_t execute_func )
{
	   
	do
    {
        
        nbiot_node_t *node;
        nbiot_value_t *data;

        if ( !(uri->flag&NBIOT_SET_RESID) )
        {
            break;
        }

        node = nbiot_node_find( dev, uri );
        if ( !node )
        {
           
            break;
        }

        data = (nbiot_value_t*)node->data;
        if ( !(data->flag&NBIOT_EXECUTABLE) )
        {
            bc95_execute_rsp(0,ackid);
            break;
        }

        if ( execute_func )
        {
            execute_func( uri->objid,
                          uri->instid,
                          uri->resid,
                          (nbiot_value_t*)node->data,
                          buffer,
                          buffer_len );
        }
         bc95_execute_rsp(2,ackid);
    } while (0);
}



static void handle_observe( nbiot_device_t    *dev,
                            const nbiot_uri_t *uri)
{
   nbiot_observe_t *obs=NULL;  
	do
    {
        nbiot_node_t *node;
			  bc95_observe_rsp(1,*uri);
        node = nbiot_node_find( dev, uri );
        if ( !node )
        {
            break;
        }
        obs= nbiot_observe_add(dev,uri);
            if ( !obs )
                break;
    } while (0);
}

static void handle_discover(const nbiot_uri_t *uri,size_t lenth,char *value)
{
	do
    {
				   bc95_discover_rsp(uri,lenth,value);
    } while (0);
}

static void handle_request( nbiot_device_t    *dev,
                            uint16_t           code,
                            uint8_t           *buffer,
                            size_t             buffer_len,
                            size_t             max_buffer_len )
{
    nbiot_uri_t uri;
    uint16_t  msgid;
    nbiot_transaction_t *transaction = NULL;
    char tmp[10],i=0;
    /* initialize response */
  char *msg=NULL;
	msg=strstr(buffer,",");
	if(msg==NULL)
	 return ;
	msg=msg+1;
	while(*msg!=',')
	tmp[i++]=*(msg++);
	tmp[i]='\0';
	i=0;
	msg=msg+1;
	msgid=uri.msgid=nbiot_atoi(tmp,strlen(tmp));
    while(*msg!=',')
	   tmp[i++]=*(msg++);
	tmp[i]='\0';
	i=0;
	msg=msg+1;
	uri.objid=nbiot_atoi(tmp,strlen(tmp));
	while(*msg!=',')
	tmp[i++]=*(msg++);
	tmp[i]='\0';
	i=0;
	msg=msg+1;
	uri.instid=nbiot_atoi(tmp,strlen(tmp));
	while((*msg!=',')&&(*msg!=0x0D))
	tmp[i++]=*(msg++);
	tmp[i]='\0';
	i=0;
	msg=msg+5;
	uri.resid=nbiot_atoi(tmp,strlen(tmp));
	  uri.flag|= NBIOT_SET_OBJID;
    if(uri.instid!=-1)
	  uri.flag|= NBIOT_SET_INSTID;
	if(uri.resid!=-1)
	  uri.flag|= NBIOT_SET_RESID;
  if ( COAP_READ == code )
    {
        printf("read objid %d instid %d resid %d msgid %d\r\n",uri.objid,uri.instid,uri.resid,uri.msgid);
			  memset(buffer,0,max_buffer_len);
        handle_read(dev,&uri,buffer,max_buffer_len);
      
    }
	if ( COAP_WRITE == code )
    {
			
      printf("write objid %d instid %d resid %d\r\n",uri.objid,uri.instid,uri.resid);
         handle_write(  dev,
                        &uri,
                        msgid,
                        msg,
                        strlen(msg),
                        dev->write_func );
     }

	 if ( COAP_EXECUTE == code )
     {
       printf("execute objid %d instid %d resid %d\r\n",uri.objid,uri.instid,uri.resid);
          handle_execute( dev,
                          &uri,
                          msgid,
                          msg,
                          strlen(msg),
                          dev->execute_func );
     }
	

}

static void handle_transaction( nbiot_device_t *dev,
	                                  uint16_t        code,
                                      uint8_t        *buffer,                              
                                      size_t          buffer_len,
                                      size_t          max_buffer_len )
{
	char *msg=NULL,i=0,tmp[10];
	static uint8_t j = 0;
	uint16_t  mid;
	nbiot_uri_t  uri;
    nbiot_transaction_t *transaction = NULL;
 if(COAP_OBSERVE == code){
 	msg=strstr(buffer,",");
			if(msg==NULL)
	   return ;
		 	msg=msg+1;
          while(*msg!=',')
	           tmp[i++]=*(msg++);
	           tmp[i]='\0';
	           i=0;
	           msg=msg+2;
	           uri.msgid=nbiot_atoi(tmp,strlen(tmp));	
	           
         while(*msg!=',')
	           tmp[i++]=*(msg++);
	           tmp[i]='\0';
	           i=0;
	           msg=msg+1;
	           uri.instid=nbiot_atoi(tmp,strlen(tmp));
				     if(uri.instid>=0)
						 uri.flag|=NBIOT_SET_INSTID;
			 while(*msg!=',')
						 tmp[i++]=*(msg++);
						 tmp[i]='\0';
						 i=0;
						 msg=msg+1;
						 uri.objid=nbiot_atoi(tmp,strlen(tmp));
						 if(uri.objid>0)
							 uri.flag|=NBIOT_SET_OBJID;
		  while(*msg!=',')
	           tmp[i++]=*(msg++);
	           tmp[i]='\0';
	           i=0;
	           msg=msg+1;
	           uri.resid=nbiot_atoi(tmp,strlen(tmp));
			   if(uri.resid>0)
				uri.flag|=NBIOT_SET_RESID;
				 if(j>3) j=0;
				 array[j][0] = uri.objid;
				 array[j][1] = uri.msgid;
				 j++;
			   handle_observe(dev,&uri);
 }else if(COAP_DISCOVER == code) 
 {
	 msg=strstr(buffer,",");
			if(msg==NULL)
	   return ;
		 	msg=msg+1;
          while(*msg!=',')
	           tmp[i++]=*(msg++);
	           tmp[i]='\0';
	           i=0;
	           msg=msg+1;
	           uri.msgid=nbiot_atoi(tmp,strlen(tmp));
				 while(*msg!=',')
						 tmp[i++]=*(msg++);
						 tmp[i]='\0';
						 i=0;
						 msg=msg+1;
						 uri.objid=nbiot_atoi(tmp,strlen(tmp));
						 if(uri.objid>0)
							 uri.flag|=NBIOT_SET_OBJID;
         while(*msg!=',')
	           tmp[i++]=*(msg++);
	           tmp[i]='\0';
	           i=0;
	           msg=msg+1;
	           uri.instid=nbiot_atoi(tmp,strlen(tmp));
				     if(uri.instid>=0)
						 uri.flag|=NBIOT_SET_INSTID;
						 printf("discover: msgid %d objid %d",uri.msgid,uri.objid);
						 if(uri.objid==3311)
			       handle_discover(&uri,strlen("5850"),"5850"); 
						 else
						handle_discover(&uri,strlen("5700"),"5700");  
 }else if(COAP_REG_RSP == code){     
	 msg=strstr(buffer,",");
		 if(msg!=NULL){
			    transaction = (nbiot_transaction_t*)NBIOT_LIST_GET( dev->transactions, dev->first_mid);
            if(*(msg+1)!='0'){
							//Led4_Set(LED_ON);
							dev->state=STATE_REGISTERED;
							if(transaction!=NULL)
								nbiot_transaction_del( dev,
			                                 true,
                                       dev->first_mid);
						}
			       else{
							nbiot_transaction_del( dev,
			                               false,
                                     dev->first_mid);
						}	
			
							        
		 }	

	}
 else if(COAP_CLOSE_RSP == code){

		msg=strstr(buffer,":");
		 if(msg!=NULL){
       if(*(msg+1)=='0'){
				 //Led4_Set(LED_OFF);
		  if(dev->state==STATE_DEREG_PENDING){		 
			 dev->state=STATE_DEREGISTERED;
			 transaction = (nbiot_transaction_t*)NBIOT_LIST_GET( dev->transactions, dev->first_mid);
			 if(transaction)
	        nbiot_transaction_del( dev,
			                           true,
                                 dev->next_mid);
			
			}else{
				dev->state=STATE_REG_FAILED;
			}	
	 
		 } 
	  }

	}else if(UPDATE_RSP == code){
	  
	   
		msg=strstr(buffer,":");
		 if(msg!=NULL){
       if(*(msg+1)=='0'){
		  if(dev->state==STATE_REG_UPDATE_PENDING){		 
			dev->state=STATE_REGISTERED;
			 transaction = (nbiot_transaction_t*)NBIOT_LIST_GET( dev->transactions, dev->first_mid);
			 if(transaction)
	        nbiot_transaction_del( dev,
			                           true,
                                 dev->next_mid);
			
			  }
		 } 
	  }
	
	}
}

static void nbiot_handle_buffer( nbiot_device_t *dev,
                                 uint8_t        *buffer,
                                 size_t          buffer_len,
                                 size_t          max_buffer_len )
{
  uint16_t           code=0;
	char *read=NULL,*write=NULL,*excute=NULL;
	char *discover=NULL,*observe=NULL,*reg_rsp=NULL,*close_rsp=NULL,*update_rsp=NULL;
	read=strstr((const char *)buffer, "+MIPLREAD");
	write=strstr((const char *)buffer, "+MIPLWRITE");
	excute=strstr((const char *)buffer, "+MIPLEXEUTE");
	discover=strstr((const char *)buffer, "+MIPLDISCOVER");
	observe=strstr((const char *)buffer, "+MIPLOBSERVE");
	reg_rsp=strstr((const char *)buffer, "+MIPLEVENT");
	close_rsp=strstr((const char *)buffer, "+MIPLCLOSE");
	update_rsp=strstr((const char *)buffer, "+MIPLUPDATE");
	if(read!=NULL)
       code=COAP_READ;
	else if(write!=NULL)
       code=COAP_WRITE;
	else if(excute!=NULL)
	   code=COAP_EXECUTE;
	else if(discover!=NULL)
		   code=COAP_DISCOVER;
	else if(observe!=NULL)
       code=COAP_OBSERVE;
	else if(reg_rsp!=NULL)
       code=COAP_REG_RSP;
	else if(close_rsp!=NULL)
	   code=COAP_CLOSE_RSP;
  else if(update_rsp!=NULL)
	   code=UPDATE_RSP;
    if (COAP_READ<= code&&code<=COAP_EXECUTE)//下行命令
    {
        if ( dev->state == STATE_REGISTERED||dev->state == STATE_REG_UPDATE_PENDING||dev->state ==STATE_REG_UPDATE_NEEDED ||dev->state ==STATE_DEREG_PENDING )
              handle_request( dev,				                  
                              code,
                              buffer,
                              buffer_len,
                              max_buffer_len );
    }
    else //observer或者注册状态回复
    {
        handle_transaction( dev,
      			                code,
                            buffer,
                            buffer_len,
                            max_buffer_len );
    }
}

int nbiot_device_connect( nbiot_device_t *dev,                    
                                  int         timeout )
{
    int ret;
    time_t last;
    time_t curr;
    uint8_t buffer[NBIOT_SOCK_BUF_SIZE];

    /* registraction */
	  memset(buffer,0,NBIOT_SOCK_BUF_SIZE);
    ret = nbiot_register_start(dev,buffer,sizeof(buffer));   
    if ( ret )     
    {
        return NBIOT_ERR_REG_FAILED;
    }
    last = nbiot_time();
    do
    {
        int buffer_len = nbiot_recv_buffer( buffer,
                                            sizeof(buffer) );
        if ( buffer_len > 0 )
        {
          
            nbiot_handle_buffer( dev,
                                 buffer,
                                 buffer_len,
                                 sizeof(buffer) );
        }

        curr = nbiot_time();
	      nbiot_transaction_step( dev,
                                curr,
                                buffer,
                                sizeof(buffer) );
//        /* ok */
//        if (curr>last + timeout&&dev->state == STATE_REGISTERED)
//        {
//            return NBIOT_ERR_OK;
//        }

//        /* failed */
//        if (curr>=last + timeout&& dev->state == STATE_REG_FAILED )
//        {
//            return NBIOT_ERR_REG_FAILED;
//        }

        /* continue */
        nbiot_sleep(100);
    } while( curr <= last + timeout );
		dev->state=STATE_REGISTERED;
    return 0;  
}

void nbiot_device_close( nbiot_device_t *dev,
                         int             timeout )
{
    int ret;
    uint8_t buffer[NBIOT_SOCK_BUF_SIZE];
    memset(buffer,0,NBIOT_SOCK_BUF_SIZE);
    ret = nbiot_deregister(dev,buffer,sizeof(buffer));  
    if ( ret == COAP_NO_ERROR )
    {
        time_t last;
        time_t curr;

        last = nbiot_time();
        do
        {
            int buffer_len = nbiot_recv_buffer( buffer,
                                                sizeof( buffer ) );
            if ( buffer_len > 0 )
            {
               
                    nbiot_handle_buffer( dev,
                                         buffer,
                                         buffer_len,
                                         sizeof( buffer ) );
            }

            curr = nbiot_time();
            nbiot_transaction_step( dev,
                                    curr,
                                    buffer,
                                    sizeof( buffer ) );
            if (dev->state == STATE_DEREGISTERED)
            {
                
					 break;
            }

            /* continue */
            nbiot_sleep( 100 );
        } while ( curr <= last + timeout );
    }
  
}

int nbiot_device_step( nbiot_device_t *dev,int timeout )
{
    time_t last;
    time_t curr;
    uint8_t buffer[NBIOT_SOCK_BUF_SIZE];
    last = nbiot_time();
    do
    {
        int buffer_len = nbiot_recv_buffer( buffer,
                                         sizeof(buffer) ); 
        if ( buffer_len > 0 )
        {
              nbiot_handle_buffer( dev,
                                   buffer,
                                   buffer_len,
                                   sizeof(buffer) );
        }
        memset(buffer,0,sizeof(buffer));
        curr = nbiot_time();  
			
        nbiot_register_step( dev,
                             curr,
                             buffer,
                             sizeof(buffer));
     if (  dev->state == STATE_REGISTERED ||
           dev->state == STATE_REG_UPDATE_NEEDED ||
           dev->state == STATE_REG_UPDATE_PENDING||
           dev->state ==STATE_DEREG_PENDING||
           dev->state ==STATE_REG_PENDING||
		       dev->state ==STATE_REG_FAILED  ){					
        nbiot_transaction_step( dev,
                                curr,
                                buffer,
                                sizeof(buffer));                      
			}			 
      if ( dev->state  == STATE_REGISTERED||dev->state == STATE_REG_UPDATE_PENDING||dev->state == STATE_REG_UPDATE_NEEDED ){					                    
        nbiot_observe_step( dev,
                            curr,
                            buffer,
                            sizeof(buffer) );
			}		                      
        nbiot_sleep(10);
    } while ( curr <= last + timeout );
		return 0;
   // return STATE_ERROR( dev );
}

int nbiot_resource_add( nbiot_device_t *dev,
                        uint16_t        objid,
                        uint16_t        instid,
                        uint16_t        resid,
                        uint16_t        attrcount,
                        uint16_t		actcount,
                        nbiot_value_t  *data )
{
    nbiot_node_t *obj;
    nbiot_node_t *res;
    nbiot_node_t *inst;
    bool obj_new = false;
    bool inst_new = false;
    nbiot_uri_t uri;
	uri.objid=objid;
	uri.instid=instid;
	uri.resid=resid;
	uri.flag|=NBIOT_SET_OBJID;
	uri.flag|=NBIOT_SET_INSTID;
  uri.flag|=NBIOT_SET_RESID;
    obj = (nbiot_node_t*)NBIOT_LIST_GET( dev->nodes, objid );
    if ( !obj )
    {
        obj = (nbiot_node_t*)nbiot_malloc( sizeof(nbiot_node_t) );
        if ( !obj )
        {
            return NBIOT_ERR_NO_MEMORY;
        }

        obj_new = true;
        nbiot_memzero( obj, sizeof(nbiot_node_t) );
        obj->id = objid;
        dev->nodes = (nbiot_node_t*)NBIOT_LIST_ADD( dev->nodes, obj );
    }

    inst = (nbiot_node_t*)NBIOT_LIST_GET( obj->data, instid );
    if ( !inst )
    {
        inst = (nbiot_node_t*)nbiot_malloc( sizeof(nbiot_node_t) );
        if ( !inst )
        {
            if ( obj_new )
            {
                dev->nodes = (nbiot_node_t*)NBIOT_LIST_DEL( dev->nodes, objid, NULL );
                nbiot_free( obj );
            }

            return NBIOT_ERR_NO_MEMORY;
        }

        inst_new = true;
        nbiot_memzero( inst, sizeof(nbiot_node_t) );
        inst->id = instid;
        obj->data = NBIOT_LIST_ADD( obj->data, inst );
		    bc95_addobj(objid,attrcount,actcount);
				nbiot_observe_add(dev,(const nbiot_uri_t * )&uri);
    }

    res = (nbiot_node_t*)NBIOT_LIST_GET( inst->data, resid);
    if ( !res )
    {
        res = (nbiot_node_t*)nbiot_malloc( sizeof(nbiot_node_t) );
        if ( !res )
        {
            if ( inst_new )
            {
                obj->data = NBIOT_LIST_DEL( obj->data, instid, NULL );
                nbiot_free( inst );
            }

            if ( obj_new )
            {
                dev->nodes = (nbiot_node_t*)NBIOT_LIST_DEL( dev->nodes, objid, NULL );
                nbiot_free( obj );
            }

            return NBIOT_ERR_NO_MEMORY;
        }

        nbiot_memzero( res, sizeof(nbiot_node_t) );
        res->id = resid;
        inst->data = NBIOT_LIST_ADD( inst->data, res );
		}
        
		    
    /* not free */
    res->data = data;
		
    if ( !STATE_ERROR(dev) )
    {
        dev->state = STATE_REG_UPDATE_NEEDED;
    }
		
    return NBIOT_ERR_OK;
}

int nbiot_resource_del( nbiot_device_t *dev,
                        uint16_t        objid,
                        uint16_t        instid,
                        uint16_t        resid )
{
    nbiot_node_t *obj;
    nbiot_node_t *res;
    nbiot_node_t *inst;

    obj = (nbiot_node_t*)NBIOT_LIST_GET( dev->nodes, objid );
    if ( obj )
    {
        inst = (nbiot_node_t*)NBIOT_LIST_GET( obj->data, instid );
        if ( inst )
        {
            inst->data = NBIOT_LIST_DEL( inst->data, resid, &res );
            if ( res )
            {
                nbiot_free( res );

                if ( inst->data )
                {
                    obj->data = NBIOT_LIST_DEL( obj->data, instid, NULL );
                    nbiot_free( inst );
                }

                if ( obj->data )
                {
                    dev->nodes = (nbiot_node_t*)NBIOT_LIST_DEL( dev->nodes, objid, NULL );
                    nbiot_free( obj );
                }

                if ( !STATE_ERROR(dev) )
                {
                    dev->state = STATE_REG_UPDATE_NEEDED;
                }
								bc95_delobj( objid);

                return NBIOT_ERR_OK;
            }
        }
    }

    return NBIOT_ERR_NOT_FOUND;
}



int nbiot_send_buffer(const nbiot_uri_t * uri, uint8_t  *buffer,size_t buffer_len,bool  updated)
{
  char tmp[8];
	char buf[1024];
	size_t  len=0;
	uint8_t type=0;
	uint8_t trigger=0;
  uint8_t *msg=NULL;
	nbiot_uri_t uri1;
	uri1.msgid=uri->msgid;
	uri1.objid=uri->objid;
	msg=buffer;
	while(1)
	{
	 if(msg!=NULL){  
     while(*msg!=',')
			tmp[len++]=*msg++;
			tmp[len]='\0';
			msg=msg+1;
			len=0;
			uri1.instid=nbiot_atoi(tmp,strlen(tmp));
		while(*msg!=',')
			tmp[len++]=*msg++;
			tmp[len]='\0';
			msg=msg+1;
		  len=0;
			uri1.resid=nbiot_atoi(tmp,strlen(tmp));
			 
    while(*msg!=',')
			tmp[len++]=*msg++;
			tmp[len]='\0';
			msg=msg+1;
		  len=0;
			type=nbiot_atoi(tmp,strlen(tmp));
	 while(*msg!=';')
       buf[len++]=*msg++;
	     buf[len]='\0';
		    msg=msg+1;
	      len=0;
	  	if(*msg=='\0')
		     trigger=1;
        if(updated==true){
           bc95_notify_upload(uri1,type,buf);
		   }	
        else
		       bc95_read_upload(uri1,type,buf);
				if(1==trigger)
					break;
	   }else{
         break;
	   }
      
	}
    return NBIOT_ERR_OK;
}

int nbiot_recv_buffer( uint8_t           *buffer,
                       size_t             buffer_len )
{
    int ret;
    size_t recv = 0;

    ret = nbiot_udp_recv( buffer,
                          buffer_len,
                          &recv);
    if ( ret )
    {
        return ret;
    }
    return recv;
}

