#include "CANbus.h"

/*
 * canbus.c
 *
 *  Created on: Nov 10, 2021
 *      Author: janoko
 */


void CAN_Tx_Init(CAN_Tx_t *cantx, CAN_HandleTypeDef *hcan, uint32_t StdId)
{
  cantx->hcan = hcan;
  cantx->id = StdId;
}

HAL_StatusTypeDef CAN_Tx_SendData(CAN_Tx_t *cantx, CAN_Data_t *data, uint8_t length, uint32_t timeout)
{
  CAN_TxHeaderTypeDef header;
  uint32_t tickstart = CAN_GetTick();

  while (HAL_CAN_GetTxMailboxesFreeLevel(cantx->hcan) == 0) {
    CAN_Delay(1);
    if (timeout > 0 && (CAN_GetTick()-tickstart) > timeout) {
      return HAL_TIMEOUT;
    }
  }

  header.IDE = CAN_ID_STD;
  header.DLC = length;
  header.RTR = CAN_RTR_DATA;
  header.StdId = cantx->id;
  return HAL_CAN_AddTxMessage(cantx->hcan, &(header), data->u8, &(cantx->mailbox));
}

void CAN_Rx_Init(CAN_Rx_t *canrx, CAN_HandleTypeDef *hcan, uint32_t id)
{
  canrx->hcan = hcan;
  canrx->id = id;

  for (uint8_t i = 0; i < CAN_LISTENER_MAX; i++) {
    if (CAN_Listener[i] == canrx || CAN_Listener[i] == NULL) {
      CAN_Listener[i] = canrx;
      break;
    }
  }
}


void CAN_IrqHandler(CAN_HandleTypeDef *hcan, uint32_t RxFifo)
{
  CAN_Rx_t            *canrx  = NULL;
  CAN_RxHeaderTypeDef header  = {0};
  CAN_Data_t          data;

  if (HAL_CAN_GetRxFifoFillLevel(hcan, RxFifo) > 0
      && HAL_CAN_GetRxMessage(hcan, RxFifo, &header, data.u8) == HAL_OK)
  {
    for(uint8_t i = 0; i < CAN_LISTENER_MAX; i++) {
      canrx = CAN_Listener[i];
      if (canrx != NULL
          && hcan == canrx->hcan
          && canrx->onRecvData != NULL
          && (header.StdId == canrx->id || header.ExtId == canrx->id)
      ) {
        canrx->onRecvData(&header, data);
      }
    }
  }
}
