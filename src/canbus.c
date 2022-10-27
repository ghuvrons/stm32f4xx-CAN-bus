/*
 * canbus.c
 *
 *  Created on: Nov 10, 2021
 *      Author: janoko
 */

/**** Includes ****************************/
#include "canbus.h"
#include <string.h>

/**** Public Function Implementations *****/

/**
 * @brief  Initialize event listener.
 * @param  eListener pointer to EventListener_t
 * @param  listenersNb max number of listener
 */
EL_Status_t EventListener_Init(EventListener_t *eListener, uint16_t listenersNb)
{
  eListener->listenerSz = listenersNb;
  if (eListener->listenerSz == 0) return EL_ERROR;

  eListener->listeners = EL_MALLOC(sizeof(Listener_t) * listenersNb);
  eListener->singleEventListenerNb = 0;
  eListener->multEventListenerNb = 0;
  return EL_OK;
}

/**
 * @brief  Setup event handler for one event.
 * @param  eListener pointer to EventListener_t
 * @param  event
 * @param  cb is event callback to handling event
 */
EL_Status_t EventListener_On(EventListener_t *eListener, Event_t event, EventCallback_t cb)
{
  uint16_t i;
  uint16_t listenerLen = eListener->singleEventListenerNb + eListener->multEventListenerNb;

  // check whether listener is full
  if (eListener->listenerSz == listenerLen)
    return EL_ERROR;

  for (i = listenerLen; i < eListener->singleEventListenerNb; i--) {

  }

  // save as sorted listener
  for (;; i--) {
    if (i == 0) break;

    // check prev index
    if (eListener->listeners[i-1].event.event > event)
      // right shift
      memcpy(&eListener->listeners[i], &eListener->listeners[i-1], sizeof(Listener_t));

    else if (eListener->listeners[i-1].event.event == event) {
      eListener->listeners[i-1].cb = cb;
      goto handleExistsListener;
    }

    else
      break;

  }

  eListener->listeners[i].event.event = event;
  eListener->listeners[i].cb = cb;
  eListener->singleEventListenerNb++;
  return EL_OK;

handleExistsListener:
  for (; i < listenerLen; i++) {
    memcpy(&eListener->listeners[i], &eListener->listeners[i+1], sizeof(Listener_t));
  }
  return EL_OK;
}

/**
 * @brief  Setup event handler for multiple event.
 * @param  eListener pointer to EventListener_t
 * @param  filter for filtering event
 * @param  cb is event callback to handling event
 */
EL_Status_t EventListener_OnMultiple(EventListener_t *eListener,
                                     EventFilter_t *filter,
                                     EventCallback_t cb)
{
  uint16_t listenerLen = eListener->singleEventListenerNb + eListener->multEventListenerNb;

  // check whether listener is full
  if (eListener->listenerSz == listenerLen)
    return EL_ERROR;

  Listener_t *listener = &eListener->listeners[listenerLen];

  memcpy(&listener->event.filter, &filter, sizeof(EventFilter_t));

  listener->event.filter.eventHigh &= listener->event.filter.maskEventHigh;
  listener->event.filter.eventLow &= listener->event.filter.maskEventLow;
  listener->cb = cb;

  return EL_OK;
}


/**
 * @brief  Handling event when event done. Then run callback
 * @param  eListener pointer to an EventListener_t structure 
 * @param  event
 * @param  data which is followed to event.
 * @param  dataSz is size of data.
 */
EL_Status_t EventListener_Handle(EventListener_t *eListener, Event_t event, void *data, uint16_t dataSz)
{
  uint16_t listenerLen = eListener->singleEventListenerNb + eListener->multEventListenerNb;
  uint16_t startFlag = 0, endFlag = eListener->singleEventListenerNb, midFlag = 0;
  Listener_t *listener;
  EL_Status_t status = EL_ERROR;

  while (1) {
    if (startFlag == endFlag) break;
    midFlag = (startFlag+endFlag)/2;
    listener = &eListener->listeners[midFlag];

    if (listener->event.event > event) {
      endFlag = midFlag;
      continue;
    }
    
    if (listener->event.event < event) {
      startFlag = midFlag+1;
      continue;
    }

    // found
    listener->cb(event, data, dataSz);
    return EL_OK;
  }

  for (uint8_t i = eListener->singleEventListenerNb; i < listenerLen; i++) {
    listener = &eListener->listeners[i];

    if (listener->cb != NULL
        && (listener->event.filter.maskEventHigh&event) == listener->event.filter.eventHigh
        && (listener->event.filter.eventLow&(~event)) == listener->event.filter.eventLow
    ) {
      listener->cb(event, data, dataSz);
      status = EL_OK;
    }
  }

  return status;
}
