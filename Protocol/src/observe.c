/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/
#include <stdio.h>
#include <string.h>
#include "internal.h"
extern uint32_t array[2][3];
nbiot_observe_t* nbiot_observe_add( nbiot_device_t    *dev,
                                    const nbiot_uri_t *uri )
{
    if ( uri->flag & NBIOT_SET_OBJID )
    {
        bool obj_new = false;
        nbiot_observe_t *obj = (nbiot_observe_t*)NBIOT_LIST_GET( dev->observes, uri->objid );
        if ( !obj )
        {
            obj = (nbiot_observe_t*)nbiot_malloc( sizeof(nbiot_observe_t) );
            if ( !obj )
            {
                return NULL;
            }

            obj_new = true;
            nbiot_memzero( obj, sizeof( nbiot_observe_t ) );
            obj->id = uri->objid;
            dev->observes = (nbiot_observe_t*)NBIOT_LIST_ADD( dev->observes, obj );
        }

        if ( uri->flag & NBIOT_SET_OBJID )
        {
            bool inst_new = false;
            nbiot_observe_t *inst = (nbiot_observe_t*)NBIOT_LIST_GET( obj->list, uri->instid );
            if ( !inst )
            {
                inst = (nbiot_observe_t*)nbiot_malloc( sizeof(nbiot_observe_t) );
                if ( !inst )
                {
                    if ( obj_new )
                    {
                        dev->observes = (nbiot_observe_t*)NBIOT_LIST_DEL( dev->observes, obj->id, NULL );
                        nbiot_free( obj );
                    }

                    return NULL;
                }

                inst_new = true;
                nbiot_memzero( inst, sizeof(nbiot_observe_t) );
                inst->id = uri->instid;
                obj->list = (nbiot_observe_t*)NBIOT_LIST_ADD( obj->list, inst );
            }

            if ( uri->flag & NBIOT_SET_RESID )
            {
                nbiot_observe_t *res = (nbiot_observe_t*)NBIOT_LIST_GET( inst->list, uri->resid );
                if ( !res )
                {
                    res = (nbiot_observe_t*)nbiot_malloc( sizeof(nbiot_observe_t) );
                    if ( !res )
                    {
                        if ( inst_new )
                        {
                            obj->list = (nbiot_observe_t*)NBIOT_LIST_DEL( obj->list, inst->id, NULL );
                            nbiot_free( inst );
                        }

                        if ( obj_new )
                        {
                            dev->observes = (nbiot_observe_t*)NBIOT_LIST_DEL( dev->observes, obj->id, NULL );
                            nbiot_free( obj );
                        }

                        return NULL;
                    }

                    nbiot_memzero( res, sizeof(nbiot_observe_t) );
                    res->id = uri->resid;
                    inst->list = (nbiot_observe_t*)NBIOT_LIST_ADD( inst->list, res );
                }

                res->active = true;
                res->lasttime = nbiot_time();
 

                return res;
            }
            else
            {
                inst->active = true;
                inst->lasttime = nbiot_time();
                return inst;
            }
        }
        else
        {
            obj->active = true;
            obj->lasttime = nbiot_time();

            return obj;
        }
    }
    
    return NULL;
}

int nbiot_observe_del( nbiot_device_t    *dev,
                       const nbiot_uri_t *uri )
{
    if ( uri->flag & NBIOT_SET_OBJID )
    {
        nbiot_observe_t *obj = (nbiot_observe_t*)NBIOT_LIST_GET( dev->observes, uri->objid );
        if ( !obj )
        {
            return COAP_NOT_FOUND_404;
        }

        if ( uri->flag & NBIOT_SET_INSTID )
        {
            nbiot_observe_t *inst = (nbiot_observe_t*)NBIOT_LIST_GET( obj->list, uri->instid );
            if ( !inst )
            {
                return COAP_NOT_FOUND_404;
            }

            if ( uri->flag & NBIOT_SET_RESID )
            {
                nbiot_observe_t *res = (nbiot_observe_t*)NBIOT_LIST_GET( inst->list, uri->resid );
                if ( !res )
                {
                    return COAP_NOT_FOUND_404;
                }

                inst->list = (nbiot_observe_t*)NBIOT_LIST_DEL( inst->list, uri->resid, &res );
                nbiot_free( res );
            }

            if ( !inst->list && (!inst->active || !(uri->flag&NBIOT_SET_RESID)) )
            {
                obj->list = (nbiot_observe_t*)NBIOT_LIST_DEL( obj->list, uri->instid, NULL );
                nbiot_free( inst );
            }
        }

        if ( !obj->list && (!obj->active || !(uri->flag&NBIOT_SET_INSTID)) )
        {
            dev->observes = (nbiot_observe_t*)NBIOT_LIST_DEL( dev->observes, uri->objid, NULL );
            nbiot_free( obj );
        }

        return COAP_CONTENT_205;
    }

    return COAP_BAD_REQUEST_400;
}

static void observe_read( nbiot_device_t    *dev,
                          nbiot_node_t      *node,
                          nbiot_uri_t       *uri,
						              uint8_t           *buffer,
                          size_t             buffer_len)
{
	uint8_t i = 0;
    do
    {
        
			  uint16_t length;
			  memset(buffer,0,buffer_len);
        length = nbiot_node_read( node,
                                  uri,
			                            uri->flag,
			                            buffer,
			                            buffer_len,
                                  true );
			  length=strlen(buffer)+1;
        if ( length <= 0 )
        {
            break;
        }
        for( i = 0;i<3;i++ )
				{
				if(array[i][0]==uri->objid)	
				{
					uri->msgid=array[i][1];
					break;
				}
				}
	    	nbiot_send_buffer(uri,buffer,length,true);		 
    } while(0);
}

static bool observe_check( nbiot_node_t *node, uint8_t flag )
{
    if ( flag & NBIOT_SET_RESID )
    {
        nbiot_value_t *data = node->data;

        return (data->flag & NBIOT_UPDATED);
    }
    else if ( flag & NBIOT_SET_OBJID )
    {
        if ( flag & NBIOT_SET_INSTID )
        {
            flag |= NBIOT_SET_RESID;
        }
        else
        {
            flag |= NBIOT_SET_INSTID;
        }

        for ( node = (nbiot_node_t*)node->data;
              node != NULL;
              node = node->next )
        {
            if ( observe_check(node,flag) )
            {
                return true;
            }
        }
    }

    return false;
}

void nbiot_observe_step( nbiot_device_t *dev,
                         time_t          now,
                         uint8_t        *buffer,
                         size_t          buffer_len )
{
    nbiot_uri_t uri[1];
    nbiot_node_t *node;
    nbiot_observe_t *obj;
    nbiot_observe_t *inst;
    nbiot_observe_t *res;

    for ( obj = dev->observes;
          obj != NULL;
          obj = obj->next )
    {
        uri->objid = obj->id;
        uri->flag = NBIOT_SET_OBJID;

        for ( inst = obj->list;
              inst != NULL;
              inst = inst->next )
        {
            uri->instid = inst->id;
            uri->flag |= NBIOT_SET_INSTID;

            for ( res = inst->list;
                  res != NULL;
                  res = res->next )
            {
                if ( res->active )
                {
                    uri->resid = res->id;
                    uri->flag |= NBIOT_SET_RESID;
                    node = nbiot_node_find( dev, uri );
                    if ( node &&
                         observe_check(node,uri->flag) )
                    {
                        observe_read( dev,                                  
                                      node,
                                      uri,
											                buffer,
											                buffer_len);
                    }
                }
            }

            if ( inst->active )
            {
                uri->flag &= ~NBIOT_SET_RESID;
                node = nbiot_node_find( dev, uri );
                if ( node &&
                     observe_check(node,uri->flag))
                {
                        observe_read( dev,                                  
                                      node,
                                      uri,
											                buffer,
											                buffer_len);
                }
            }
        }

        if ( obj->active )
        {
            uri->flag = NBIOT_SET_OBJID;
            node = nbiot_node_find( dev, uri );
            if ( node &&observe_check(node,uri->flag))
            {
                    observe_read( dev,                                  
                                  node,
                                  uri,
											            buffer,
											            buffer_len);
            }
        }
    }
}
