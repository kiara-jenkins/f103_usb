
/* Includes ------------------------------------------------------------------*/
#include "usbd_cla.h"
#include "usbd_ctlreq.h"


static uint8_t  USBD_CDC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t  USBD_CDC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);

static uint8_t  USBD_CDC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t  USBD_CDC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_CDC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  *USBD_CDC_GetFSCfgDesc(uint16_t *length);

static uint8_t  *USBD_CDC_GetHSCfgDesc(uint16_t *length);

uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor(uint16_t *length);


/* CDC interface class callbacks structure */
USBD_ClassTypeDef  USBD_CDC =
{
  USBD_CDC_Init,
  USBD_CDC_DeInit,
  USBD_CDC_Setup,
  NULL,                 /* EP0_TxSent, */
  NULL,		// USBD_CDC_EP0_RxReady,
  USBD_CDC_DataIn,
  USBD_CDC_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_CDC_GetHSCfgDesc,
  USBD_CDC_GetFSCfgDesc,
  NULL,		// USBD_CDC_GetOtherSpeedCfgDesc,
  USBD_CDC_GetDeviceQualifierDescriptor,
};


/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDC_CfgFSDesc[]  __ALIGN_END =
{
/* MIDI Adapter Configuration Descriptor: 9Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 37,38 */
0x09,		// Length of the Descriptor (1Byte)
0x02,		// Descriptor Type: Configuration (1Byte)
0x65,    	// Total Length of the config. block including this descriptor: length is 101 bytes (2bytes Low-byte first)
0x00,   	// Total Length high-byte, continuing from above
0x02,		// Number of Interfaces: 2 interfaces: Standard AC and Standard MIDI-streaming (1Byte)
0x01,		// Configuration Value: ID of this configuration is 1 (1Byte)
0x00,		// iConfiguration: Unused (1Byte)
0x80,		// bmAttributes:   BUS Powered and not Battery/Self powered and no remote wake-up (1Byte)
0x32,		// MaxPower = 100 mA, in steps of 2mA (1Byte)

/* MIDI Adapter Standard Audio Control (AC) Interface Descriptor: 9Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 38 */
0x09,		// Length of the Descriptor (1Byte)
0x04,		// Descriptor Type: Interface (1Byte)
0x00,		// Index of this interface (1Byte)
0x00,		// Alternate Setting: Index of this Setting (1Byte)
0x00,		// Number of End-points (1Byte)
0x01,		// Interface Class: Audio (1Byte)
0x01,		// Interface Sub-Class: Audio Control (1Byte)
0x00,		// Interface Protocol: Unused (1Byte)
0x00,		// iInterface: Unused (1Byte)

/* MIDI Adapter Class-specific AC Interface Descriptor: 9Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 39 */
0x09,		// Length of the Descriptor (1Byte)
0x24,		// Descriptor Type: Class specific interface (1Byte)
0x01,		// Descriptor Sub-type: Class Specific Interface Header (1Byte)
0x00,		// Class Specification Revision No.: 1.00 (2Bytes Low-byte first)
0x01,		// Class Specification revision No.: High-byte, continuing from above
0x09,		// Total Length of class-specific descriptor: 9-bytes (2Bytes Low-byte first)
0x00,		// Total Length of class-specific descriptor: High-byte, Continuing from above
0x01,     	// Number of streaming interfaces: 1 (1Byte)
0x01,		// baInterfaceNr: MIDI-Streaming interface 1 belongs to this AudioControl interface. (1Byte)

/* MIDI Adapter Standard MIDI Streaming (MS) Interface Descriptor: 9Bytes  */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 39 */
0x09,		// Length of the Descriptor (1Byte)
0x04,		// Descriptor Type: Interface (1Byte)
0x01,		// Index of this interface (1Byte)
0x00,		// Alternate Setting: Index of this Setting (1Byte)
0x02,		// Number of End-points (1Byte)
0x01,		// Interface Class: Audio (1Byte)
0x03,		// Interface Sub-Class: MIDI-Streaming (1Byte)
0x00,		// Interface Protocol: Unused (1Byte)
0x00,		// iInterface: Unused (1Byte)

/*  MIDI Adapter Class-specific MS Interface Descriptor: 7Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 40 */
0x07,		// Length of the Descriptor (1Byte)
0x24,		// Descriptor Type: Class specific interface (1Byte)
0x01,		// Descriptor Sub-type: Class Specific Interface Header (1Byte)
0x00,		// Class Specification Revision No.: 1.00 (2Bytes Low-byte first)
0x01,		// Class Specification revision No.: High-byte, continuing from above
0x41,		// Total length of class specific descriptor: length is 65bytes (2bytes Low-byte first)
0x00,		// Total Length high-byte, continuing from above

/* MIDI Adapter MIDI IN Jack Descriptor (Embedded): 6Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 40 */
0x06,		// Length of the Descriptor (1Byte)
0x24,		// Descriptor Type: Class specific interface (1Byte)
0x02,		// Descriptor Sub-type: MIDI IN Jack (1Byte)
0x01,		// Jack Type: Embedded (1Byte)
0x01,		// Jack ID: 1 (1Byte)
0x00,		// iJack: Unused (1Byte)

/* MIDI Adapter MIDI IN Jack Descriptor (External): 6Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 40 */
0x06,		// Length of the Descriptor (1Byte)
0x24,		// Descriptor Type: Class specific interface (1Byte)
0x02,		// Descriptor Sub-type: MIDI IN Jack (1Byte)
0x02,		// Jack Type: External (1Byte)
0x02,		// Jack ID: 2 (1Byte)
0x00,		// iJack: Unused (1Byte)

/* MIDI Adapter MIDI OUT Jack Descriptor (Embedded): 9Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 41 */
0x09,		// Length of the Descriptor (1Byte)
0x24,		// Descriptor Type: Class specific interface (1Byte)
0x03,		// Descriptor Sub-type: MIDI OUT Jack (1Byte)
0x01,		// Jack Type: Embedded (1Byte)
0x03,		// Jack ID: 3 (1Byte)
0x01,		// Number of Input Pins for this jack: 1 (1Byte)
0x02,		// Source ID: ID of the Entity to which this Pin is connected: Connected to External MIDI In Jack??? (1Byte)
0x01,		// Source Pin: Output Pin number of the Entity to which this Input Pin is connected (1Byte)
0x00,		// iJack: Unused (1Byte)

/* MIDI Adapter MIDI OUT Jack Descriptor (External): 9Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 41 */
0x09,		// Length of the Descriptor (1Byte)
0x24,		// Descriptor Type: Class specific interface (1Byte)
0x03,		// Descriptor Sub-type: MIDI OUT Jack (1Byte)
0x02,		// Jack Type: External (1Byte)
0x04,		// Jack ID: 4 (1Byte)
0x01,		// Number of Input Pins for this jack: 1 (1Byte)
0x01,		// Source ID: ID of the Entity to which this Pin is connected: Connected to Embedded MIDI In Jack??? (1Byte)
0x01,		// Source Pin: Output Pin number of the Entity to which this Input Pin is connected (1Byte)
0x00,		// iJack: Unused (1Byte)

/* MIDI Adapter Standard Bulk OUT Endpoint Descriptor: 9Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 42 */
0x09,		// Length of the Descriptor (1Byte)
0x05,		// Descriptor Type: Endpoint (1Byte)
0x02,		// Endpoint Address: OUT Endpoint 1 (1Byte)
0x02,		// Attributes: Bulk, Not shared (1Byte)
0x40,		// Max Packet Size: 64 Bytes (2Bytes low-byte first)
0x00,		// Max Packet Size: high-byte, continuing from above
0x00,		// Interval: Ignored for bulk mode (1Byte)
0x00,		// Refresh: Unused (1Byte)
0x00,		// Synch. Address: Unused (1Byte)

/* MIDI Adapter Class-specific Bulk OUT Endpoint Descriptor: 5Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 42 */
0x05,		// Length of the Descriptor (1Byte)
0x25,		// Descriptor Type: Class Specific Endpoint descriptor (1Byte)
0x01,		// Descriptor Sub-type: MIDI-Streaming General sub-type (1Byte)
0x01,		// No. of Embedded MIDI IN Jack: 1 (1Byte)
0x01,		// ID of Embedded MIDI IN Jack: 1 (1Byte)

/* MIDI Adapter Standard Bulk IN Endpoint Descriptor: 9Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 42,43 */
0x09,		// Length of the Descriptor (1Byte)
0x05,		// Descriptor Type: Endpoint (1Byte)
0x81,		// Endpoint Address: IN Endpoint 1 (1Byte)
0x02,		// Attributes: Bulk, Not shared (1Byte)
0x40,		// Max Packet Size: 64 Bytes (2Bytes low-byte first)
0x00,		// Max Packet Size: high-byte, continuing from above
0x00,		// Interval: Ignored for bulk mode (1Byte)
0x00,		// Refresh: Unused (1Byte)
0x00,		// Synch. Address: Unused (1Byte)

/* MIDI Adapter Class-specific Bulk IN Endpoint Descriptor: 5Bytes */
/* Reference: https://www.usb.org/sites/default/files/midi10.pdf Page: 43 */
0x05,		// Length of the Descriptor (1Byte)
0x25,		// Descriptor Type: Class Specific Endpoint descriptor (1Byte)
0x01,		// Descriptor Sub-type: MIDI-Streaming General sub-type (1Byte)
0x01,		// No. of Embedded MIDI OUT Jack: 1 (1Byte)
0x03		// ID of Embedded MIDI OUT Jack: 3 (1Byte)
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @brief  USBD_CDC_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t ret = 0U;
  USBD_CDC_HandleTypeDef   *hcdc;

    /* Open EP IN */
    USBD_LL_OpenEP(pdev, CDC_IN_EP, USBD_EP_TYPE_BULK, CDC_DATA_PACKET_SIZE);
    pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 1U;

    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, CDC_OUT_EP, USBD_EP_TYPE_BULK, CDC_DATA_PACKET_SIZE);
    pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 1U;

  pdev->pClassData = USBD_malloc(sizeof(USBD_CDC_HandleTypeDef));

  if (pdev->pClassData == NULL)
  {
    ret = 1U;
  }
  else
  {
    hcdc = (USBD_CDC_HandleTypeDef *) pdev->pClassData;

    /* Init Xfer states */
    hcdc->RxBuffer = hcdc->defaultRxBuffer;
    hcdc->TxState = 0U;
    hcdc->RxState = 0U;
    hcdc->RxCnt = 0;

    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer, CDC_DATA_PACKET_SIZE);
  }
  return ret;
}

/**
  * @brief  USBD_CDC_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_CDC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t ret = 0U;

  /* Close EP IN */
  USBD_LL_CloseEP(pdev, CDC_IN_EP);
  pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 0U;

  /* Close EP OUT */
  USBD_LL_CloseEP(pdev, CDC_OUT_EP);
  pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 0U;

  USBD_free(pdev->pClassData);
  pdev->pClassData = NULL;

return ret;
}

/**
  * @brief  USBD_CDC_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_CDC_Setup(USBD_HandleTypeDef *pdev,
                               USBD_SetupReqTypedef *req)
{
// USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef *) pdev->pClassData;
  uint8_t ifalt = 0U;
  uint16_t status_info = 0U;
  uint8_t ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS :
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, (uint8_t *)(void *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_DESCRIPTOR:
          // utile pour HID : report descriptors
          break;

        // 1 seul interface, pas d'alt setting
        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return ret;
}

/**
  * @brief  USBD_CDC_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  // PCD_HandleTypeDef *hpcd = pdev->pData;

  if (pdev->pClassData != NULL)
  { /*
    if ((pdev->ep_in[epnum].total_length > 0U) && ((pdev->ep_in[epnum].total_length % hpcd->IN_ep[epnum].maxpacket) == 0U))
    {
      // Update the packet total length
      pdev->ep_in[epnum].total_length = 0U;

      // Send ZLP
      USBD_LL_Transmit(pdev, epnum, NULL, 0U);
    }
    else */
    {
      hcdc->TxState = 0U;
    }
    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_CDC_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_CDC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef *) pdev->pClassData;

  /* Get the received data length */
  hcdc->RxLength = USBD_LL_GetRxDataSize(pdev, epnum);

  // ici on peut inserer le traitement du message, lui passer
  // lui passer hcdc->RxBuffer et hcdc->RxLength
  // pendant ce temps la, le flux est controle (NAKed)
  // ou alors simplement alerter l'appli
  hcdc->RxCnt += 1;

  // et puis il faut preparer la reception du prochain
  USBD_LL_PrepareReceive( pdev, CDC_OUT_EP, hcdc->RxBuffer, CDC_DATA_PACKET_SIZE);

  return USBD_OK;

}


/**
  * @brief  USBD_CDC_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_CfgFSDesc);
  return USBD_CDC_CfgFSDesc;
}

/**
  * @brief  USBD_CDC_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CDC_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_CfgFSDesc);
  return USBD_CDC_CfgFSDesc;
}

/**
* @brief  DeviceQualifierDescriptor
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_CDC_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = sizeof(USBD_CDC_DeviceQualifierDesc);
  return USBD_CDC_DeviceQualifierDesc;
}

/**
  * @brief  USBD_CDC_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t  USBD_CDC_SetRxBuffer(USBD_HandleTypeDef   *pdev,
                              uint8_t  *pbuff)
{
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef *) pdev->pClassData;

  hcdc->RxBuffer = pbuff;

  return USBD_OK;
}


/**
  * @brief  USBD_CDC_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  *
uint8_t  USBD_CDC_SetTxBuffer(USBD_HandleTypeDef   *pdev,
                              uint8_t  *pbuff,
                              uint16_t length)
{
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef *) pdev->pClassData;

  hcdc->TxBuffer = pbuff;
  hcdc->TxLength = length;

  return USBD_OK;
} */



// USBD_CDC_SendPacket() is the merge of USBD_CDC_SetTxBuffer() and USBD_CDC_TransmitPacket()
//
uint8_t  USBD_CDC_SendPacket( USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint16_t length )
{
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef *) pdev->pClassData;

  hcdc->TxBuffer = pbuff;
  hcdc->TxLength = length;

  if (pdev->pClassData != NULL)
  {
    if (hcdc->TxState == 0U)
    {
      /* Tx Transfer in progress */
      hcdc->TxState = 1U;
      /* Update the packet total length */
      pdev->ep_in[CDC_IN_EP & 0xFU].total_length = hcdc->TxLength;
      /* Transmit next packet */
      USBD_LL_Transmit(pdev, CDC_IN_EP, hcdc->TxBuffer,
                       (uint16_t)hcdc->TxLength);
      return USBD_OK;
    }
    else
    {
      return USBD_BUSY;
    }
  }
  else
  {
    return USBD_FAIL;
  }
}


/**
  * @brief  USBD_CDC_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t  USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef *) pdev->pClassData;

  /* Suspend or Resume USB Out process */
  if (pdev->pClassData != NULL)
  {
      /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->RxBuffer, CDC_DATA_PACKET_SIZE);
    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}
