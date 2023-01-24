/*
 * event-listener.h
 *
 *  Created on: Nov 10, 2021
 *      Author: janoko
 */

#ifndef EVENT_LISTENER_H_
#define EVENT_LISTENER_H_

/**** Includes ****************************/
#include <stddef.h>
#include <stdint.h>

/**** Defines *****************************/
#ifndef EL_MALLOC
#include <stdlib.h>
#define EL_MALLOC(sz) malloc(sz)
#endif

/**** Exported Types **********************/

typedef enum {
  EL_OK,
  EL_ERROR,
} EL_Status_t;

typedef uint32_t Event_t;
typedef struct {
  uint32_t        eventHigh;
  uint32_t        eventLow;
  uint32_t        maskEventHigh;
  uint32_t        maskEventLow;
} EventFilter_t;

typedef void (*EventCallback_t)(Event_t, void* data, uint16_t dataSz);

typedef struct Listener_t {
  union {
    Event_t         event;
    EventFilter_t   filter;
  } event;
  EventCallback_t cb;
} Listener_t;

typedef struct EventListener_t {
  Listener_t *listeners;
  uint16_t   listenerSz;
  uint16_t   singleEventListenerNb;
  uint16_t   multEventListenerNb;
} EventListener_t;

/**** Public Variables ********************/


/**** Public Function Prototypes **********/
// interrupt handler

// CAN Tx Methods
//void CAN_Tx_Init(CAN_Tx_t*, CAN_HANDLER*, uint32_t id, uint8_t isExtended);
//HAL_StatusTypeDef CAN_Tx_SendData(CAN_Tx_t*, CAN_Data_t *data, uint8_t length, uint32_t timeout);

// CAN Rx Methods
EL_Status_t EventListener_Init(EventListener_t*, uint16_t listenersNb);
EL_Status_t EventListener_On(EventListener_t*, Event_t, EventCallback_t);
EL_Status_t EventListener_OnMultiple(EventListener_t*, EventFilter_t*, EventCallback_t);
EL_Status_t EventListener_Handle(EventListener_t*, Event_t, void *data, uint16_t dataSz);
#endif /* EVENT_LISTENER_H_ */
