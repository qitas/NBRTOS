/**
 * Copyright (c) 2017 China Mobile IOT.
 * All rights reserved.
**/

#ifndef ONENET_INTERNAL_H_
#define ONENET_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "coap.h"
#include <utils.h>
#include "nbiot.h"

typedef struct _nbiot_list_t
{
    struct _nbiot_list_t *next;
    uint16_t              id;
} nbiot_list_t;

/**
 * list.c
**/
uint16_t nbiot_list_newid( nbiot_list_t *list );
nbiot_list_t* nbiot_list_add( nbiot_list_t *list,
                              nbiot_list_t *node );
nbiot_list_t* nbiot_list_get( nbiot_list_t *list,
                              uint16_t      id );
nbiot_list_t* nbiot_list_del( nbiot_list_t  *list,
                              uint16_t       id,
                              nbiot_list_t **node );
void nbiot_list_free( nbiot_list_t *list );

#define NBIOT_LIST_ADD(l,n)   nbiot_list_add((nbiot_list_t*)(l),(nbiot_list_t*)(n))
#define NBIOT_LIST_GET(l,i)   nbiot_list_get((nbiot_list_t*)(l),(i))
#define NBIOT_LIST_DEL(l,i,n) nbiot_list_del((nbiot_list_t*)(l),(i),(nbiot_list_t**)(n))
#define NBIOT_LIST_FREE(l)    nbiot_list_free((nbiot_list_t*)(l))





/**
 * resource uri flag
**/
#define NBIOT_SET_OBJID     0x1
#define NBIOT_SET_INSTID    0x2
#define NBIOT_SET_RESID     0x4
#define NBIOT_SET_MSGID     0x8
/**
 * resource uri
**/



typedef enum
{
    STATE_DEREGISTERED = 0,   /* not registered or boostrap not started */
    STATE_REG_PENDING,        /* registration pending */
    STATE_REGISTERED,         /* successfully registered */
    STATE_REG_FAILED,         /* last registration failed */
    STATE_REG_UPDATE_PENDING, /* registration update pending */
    STATE_REG_UPDATE_NEEDED,  /* registration update required */
    STATE_DEREG_PENDING,      /* deregistration pending */
    STATE_BS_INITIATED,       /* bootstrap request sent */
    STATE_BS_PENDING,         /* boostrap on going */
    STATE_BS_FINISHED,        /* bootstrap done */
    STATE_BS_FAILED,          /* bootstrap failed */
    STATE_SERVER_RESET        /* server reset */
} nbiot_status_t;

#define STATE_ERROR(x) \
        ((x)->state == STATE_DEREGISTERED ? NBIOT_ERR_NOT_REGISTER : \
        ((x)->state == STATE_REGISTERED ? NBIOT_ERR_OK : \
        ((x)->state == STATE_REG_UPDATE_PENDING ? NBIOT_ERR_OK : \
        ((x)->state == STATE_REG_UPDATE_NEEDED ? NBIOT_ERR_OK : \
        ((x)->state == STATE_REG_FAILED ? NBIOT_ERR_REG_FAILED : \
        ((x)->state == STATE_SERVER_RESET ? NBIOT_ERR_SERVER_RESET : \
        ((x)->state == STATE_BS_FAILED ? NBIOT_ERR_BS_FAILED : NBIOT_ERR_PENDING)))))))

typedef struct _nbiot_node_t
{
    struct _nbiot_node_t *next;
    uint16_t              id;
    void                 *data;
} nbiot_node_t;

#ifdef NBIOT_LOCATION
typedef struct _nbiot_location_t
{
    struct _nbiot_location_t *next;
    uint8_t                   size;
    uint8_t                   name[1];
} nbiot_location_t;
#define LOCATION_SIZE(x) (sizeof(nbiot_location_t)+(x)-1)
#endif

typedef struct _nbiot_observe_t
{
    struct _nbiot_observe_t *next;
    uint16_t                 id;
    uint16_t                 lastmid;
    uint32_t                 counter;
    time_t                   lasttime;
    struct _nbiot_observe_t *list;
    bool                     active;
} nbiot_observe_t;

static void registraction_update_reply( nbiot_device_t    *dev,
                                         bool              suc,
                                         bool              ack);
typedef void(*nbiot_transaction_callback_t)(nbiot_device_t*,bool,bool);
typedef struct _nbiot_transaction_t
{
    struct _nbiot_transaction_t *next;
    uint16_t                     mid;
    bool                         ack;
    uint8_t                      counter;
    time_t                       timeout;
    time_t                       time;
    uint8_t                     *buffer;
    size_t                       buffer_len;
    nbiot_transaction_callback_t callback;
} nbiot_transaction_t;

/**
 * write????(write???)
 * @param objid  object id
 *        instid object instance id
 *        resid  resource id
 *        data   ????
**/
typedef void(*nbiot_write_callback_t)(uint16_t       objid,
                                      uint16_t       instid,
                                      uint16_t       resid,
                                      nbiot_value_t *data);

typedef void(*nbiot_read_callback_t)(uint16_t       objid,
                                       uint16_t       instid,
                                       uint16_t       resid,
                                       nbiot_value_t *data);


/**
 * execute????
 * @param objid  object id
 *        instid object instance id
 *        resid  resource id
 *        data   ????
 *        buff   ????????
 *        size   ????????
**/
typedef void(*nbiot_execute_callback_t)(uint16_t       objid,
                                        uint16_t       instid,
                                        uint16_t       resid,
                                        nbiot_value_t *data,
                                        const void    *buff,
                                        size_t         size);
																				
struct _nbiot_device_t
{
    uint16_t                    next_mid;
	  uint16_t                    first_mid;
    uint8_t                     state;
    int                         life_time;
    time_t                      registraction;
    const char                 *endpoint_name;
    nbiot_node_t               *nodes;
    nbiot_observe_t            *observes;
#ifdef NBIOT_LOCATION
    nbiot_location_t           *locations;
#endif
    nbiot_transaction_t        *transactions;
    nbiot_write_callback_t      write_func;
	nbiot_read_callback_t         read_func;
    nbiot_execute_callback_t    execute_func;
    char                       miplbuf[512];
};

/**
 * node.c
**/
int nbiot_node_read( nbiot_node_t        *node,
                     nbiot_uri_t         *uri,
										 uint8_t             flag, 
					           uint8_t       *buffer,
                     size_t        buffer_len,
                     bool          updated );

int nbiot_node_write( nbiot_node_t          *node,
                      const nbiot_uri_t     *uri,
                      uint16_t               ackid, 
                      uint8_t                *buffer,
                      size_t                 buffer_len,
                      nbiot_write_callback_t write_func );

nbiot_node_t* nbiot_node_find( nbiot_device_t    *dev,
                               const nbiot_uri_t *uri );


/**
 * registraction.c
**/
int nbiot_register_start( nbiot_device_t *dev,
							   uint8_t		  *buffer,
							   size_t		   buffer_len);


int nbiot_register_update( nbiot_device_t *dev,
	                         uint8_t        *buffer,
                           size_t          buffer_len);

int  nbiot_deregister ( nbiot_device_t *dev,
							uint8_t		  *buffer,
							size_t		   buffer_len);


void nbiot_register_step( nbiot_device_t *dev,
                                time_t          now,
                                uint8_t		  *buffer,
							    size_t		   buffer_len);

/**
 * transaction.c
**/
int nbiot_transaction_add( nbiot_device_t              *dev,
	                         bool                         update,
                           const uint8_t               *buffer,
                           size_t                       buffer_len,
                           nbiot_transaction_callback_t callback );
int nbiot_transaction_del( nbiot_device_t *dev,
	                         bool            succ,
                           uint16_t        mid);
void nbiot_transaction_step( nbiot_device_t *dev,
                             time_t          now,
                             uint8_t        *buffer,
                             size_t          buffer_len );

/**
 * observe.c
**/
nbiot_observe_t* nbiot_observe_add( nbiot_device_t    *dev,
                                    const nbiot_uri_t *uri);
static void handle_discover(const nbiot_uri_t *uri,size_t lenth,char *value);
int nbiot_observe_del( nbiot_device_t    *dev,
                       const nbiot_uri_t *uri );
void nbiot_observe_step( nbiot_device_t *dev,
                         time_t          now,
                         uint8_t        *buffer,
                         size_t          buffer_len );

#ifdef __cplusplus
} /* extern "C"{ */
#endif

#endif /* ONENET_INTERNAL_H_ */
