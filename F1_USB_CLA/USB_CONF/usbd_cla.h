
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CLA_H
#define __USB_CLA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

#define CLA_IN_EP                                   0x81U  /* EP1 for data IN */
#define CLA_OUT_EP                                  0x02U  /* EP2 for data OUT */

#ifndef CLA_HS_BINTERVAL
#define CLA_HS_BINTERVAL                          0x10U
#endif /* CLA_HS_BINTERVAL */

#ifndef CLA_FS_BINTERVAL
#define CLA_FS_BINTERVAL                          0x10U
#endif /* CLA_FS_BINTERVAL */

/* CLA Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
// le jour ou on voudra, on pourra mettre cela dans une variable
//et monter sa valeur a 512 si (pdev->dev_speed == USBD_SPEED_HIGH)
#define CLA_DATA_PACKET_SIZE                 	64U

typedef struct
{
  uint8_t  defaultRxBuffer[CLA_DATA_PACKET_SIZE];
  uint8_t  *RxBuffer;
  uint8_t  *TxBuffer;
  uint32_t RxLength;
  uint32_t TxLength;
  __IO uint32_t RxCnt;
  __IO uint32_t TxState;
  __IO uint32_t RxState;
}
USBD_CLA_HandleTypeDef;


extern USBD_ClassTypeDef  USBD_CLA;
#define USBD_CLA_CLASS    &USBD_CLA


uint8_t  USBD_CLA_SetRxBuffer(USBD_HandleTypeDef   *pdev,
                              uint8_t  *pbuff);

uint8_t  USBD_CLA_ReceivePacket(USBD_HandleTypeDef *pdev);

//uint8_t  USBD_CLA_SetTxBuffer(USBD_HandleTypeDef   *pdev,
//                              uint8_t  *pbuff,
//                              uint16_t length);

//uint8_t  USBD_CLA_TransmitPacket(USBD_HandleTypeDef *pdev);

// USBD_CLA_SendPacket() is the merge of USBD_CLA_SetTxBuffer() and USBD_CLA_TransmitPacket()
uint8_t  USBD_CLA_SendPacket( USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length );


#ifdef __cplusplus
}
#endif

#endif  /* __USB_CLA_H */
