/*
 * canbus.c
 *
 *  Created on: Nov 10, 2021
 *      Author: janoko
 */

/**** Includes ****************************/
#include "CAN_Bus.h"


/**** Public Function Implementations *****/

/**
 * @brief  Initialing peripheral and Id for canrx handler
 * @param  cantx pointer to can tx handler
 * @param  hcan : pointer to can peripheral
 * @param  StdId is CAN Id
 */
void CAN_Tx_Init(CAN_Tx_t *cantx, CAN_HandleTypeDef *hcan, uint32_t StdId)
{
  cantx->hcan = hcan;
  cantx->id = StdId;
}


/**
 * @brief  Sending data
 * @param  cantx pointer to can tx handler
 * @param  data is bytes of send data
 * @param  length is length of data bytes
 * @param  operation timeout
 * @return HAL status
 */
HAL_StatusTypeDef CAN_Tx_SendData(CAN_Tx_t *cantx, CAN_Data_t *data, uint8_t length, uint32_t timeout)
{
  CAN_TxHeaderTypeDef header;
  uint32_t tickstart = CAN_GetTick();

  if (cantx->hcan == NULL) return HAL_ERROR;

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


/**
 * @brief  Initialing peripheral and Id for canrx handler
 * @param  cantx pointer to can tx handler
 * @param  hcan pointer to an CAN_HandleTypeDef (can peripheral)
 * @param  StdId is CAN Id
 */
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


/**
 * @brief  Handling interrupt/incoming data.
 *         Then run canrx listener.
 * @param  hcan pointer to an CAN_HandleTypeDef structure that contains
 *         the configuration information for the specified CAN.
 * @param  RxFifo Rx FIFO.
 *         This parameter can be a value of @arg CAN_receive_FIFO_number.
 */
void CAN_IrqHandler(CAN_HandleTypeDef *hcan, uint32_t RxFifo)
{
  CAN_Rx_t            *canrx  = NULL;
  CAN_RxHeaderTypeDef header  = {0};
  CAN_Data_t          data;
  uint32_t            id = 0;

  if (HAL_CAN_GetRxFifoFillLevel(hcan, RxFifo) > 0
      && HAL_CAN_GetRxMessage(hcan, RxFifo, &header, data.u8) == HAL_OK)
  {
    for(uint8_t i = 0; i < CAN_LISTENER_MAX; i++)
    {
      canrx = CAN_Listener[i];
      if ((canrx != NULL) && (hcan == canrx->hcan) && (canrx->onRecvData != NULL))
      {
        if (canrx->id == 0 && canrx->filterMaskIdHigh == 0 && canrx->filterMaskIdLow == 0)
          continue;

        if (header.IDE == CAN_ID_STD)
          id = header.StdId;
        else if (header.IDE == CAN_ID_EXT)
          id = header.ExtId;
        else continue;

        if (canrx->id == id
            || (canrx->id == 0
                && ((canrx->filterMaskIdHigh & id) & 0x1FFFFFFF) == 0
                && ((canrx->filterMaskIdLow & (~id)) & 0x1FFFFFFF) == 0
        )) {
          canrx->onRecvData(&header, &data);
        }
      }
    }
  }
}
