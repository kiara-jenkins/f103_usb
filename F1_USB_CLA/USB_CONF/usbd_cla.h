
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

#define CDC_IN_EP                                   0x81U  /* EP1 for data IN */
#define CDC_OUT_EP                                  0x02U  /* EP2 for data OUT */

#ifndef CDC_HS_BINTERVAL
#define CDC_HS_BINTERVAL                          0x10U
#endif /* CDC_HS_BINTERVAL */

#ifndef CDC_FS_BINTERVAL
#define CDC_FS_BINTERVAL                          0x10U
#endif /* CDC_FS_BINTERVAL */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
// le jour ou on voudra, on pourra mettre cela dans une variable
//et monter sa valeur a 512 si (pdev->dev_speed == USBD_SPEED_HIGH)
#define CDC_DATA_PACKET_SIZE                 	64U

typedef struct
{
  uint8_t  defaultRxBuffer[CDC_DATA_PACKET_SIZE];
  uint8_t  *RxBuffer;
  uint8_t  *TxBuffer;
  uint32_t RxLength;
  uint32_t TxLength;
  __IO uint32_t RxCnt;
  __IO uint32_t TxState;
  __IO uint32_t RxState;
}
USBD_CDC_HandleTypeDef;


extern USBD_ClassTypeDef  USBD_CDC;
#define USBD_CDC_CLASS    &USBD_CDC


uint8_t  USBD_CDC_SetRxBuffer(USBD_HandleTypeDef   *pdev,
                              uint8_t  *pbuff);

uint8_t  USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev);

//uint8_t  USBD_CDC_SetTxBuffer(USBD_HandleTypeDef   *pdev,
//                              uint8_t  *pbuff,
//                              uint16_t length);

//uint8_t  USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev);

// USBD_CDC_SendPacket() is the merge of USBD_CDC_SetTxBuffer() and USBD_CDC_TransmitPacket()
uint8_t  USBD_CDC_SendPacket( USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length );


#ifdef __cplusplus
}
#endif

#endif  /* __USB_CDC_H */
