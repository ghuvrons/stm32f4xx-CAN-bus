/*
 * canbus.h
 *
 *  Created on: Nov 10, 2021
 *      Author: janoko
 */

#ifndef DRIVER_CANBUS_H_
#define DRIVER_CANBUS_H_

#include "stm32f4xx_hal.h"

#ifndef CAN_LISTENER_MAX
#define CAN_LISTENER_MAX 3
#endif

#ifndef CAN_Delay
#define CAN_Delay(ms) HAL_Delay(ms)
#endif

#ifndef CAN_GetTick
#define CAN_GetTick() HAL_GetTick()
#endif


typedef union
{
  uint8_t u8[8];
  uint16_t u16[4];
  uint32_t u32[2];
  uint64_t u64;
  int8_t i8[8];
  int16_t i16[4];
  int32_t i32[2];
  int64_t i64;
  char CHAR[8];
  float FLOAT[2];
  double DOUBLE;
} CAN_Data_t;

typedef struct {
  CAN_HandleTypeDef   *hcan;
  uint32_t            id;
  uint32_t            mailbox;
  uint32_t            timeout;
} CAN_Tx_t;

typedef struct {
  CAN_HandleTypeDef   *hcan;
  uint32_t            id;
  void                (*onRecvData)(CAN_RxHeaderTypeDef *header, CAN_Data_t data);
} CAN_Rx_t;


CAN_Rx_t *CAN_Listener[CAN_LISTENER_MAX];

void CAN_IrqHandler(CAN_HandleTypeDef *hcan, uint32_t RxFifo);

// CAN Tx Methods
void CAN_Tx_Init(CAN_Tx_t*, CAN_HandleTypeDef*, uint32_t StdId);
HAL_StatusTypeDef CAN_Tx_SendData(CAN_Tx_t*, CAN_Data_t *data, uint8_t length, uint32_t timeout);

// CAN Rx Methods
void CAN_Rx_Init(CAN_Rx_t*, CAN_HandleTypeDef*, uint32_t id);
#endif /* CANBUS_CANBUS_H_ */
