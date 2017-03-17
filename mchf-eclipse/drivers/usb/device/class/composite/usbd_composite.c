/**
 ******************************************************************************
 * @file    USBD_COMP.c
 * @author  DB4PLE
 * @version V1.0.0
 * @date    11-December-2015
 * @brief   This file provides the composite core functions.
 *
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <drivers/usb/device/class/composite/usbd_composite.h>
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "mchf_board.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
 * @{
 */


/** @defgroup USBD_COMP
 * @brief usbd core module
 * @{
 */

/** @defgroup USBD_COMP_Private_TypesDefinitions
 * @{
 */
/**
 * @}
 */


/** @defgroup USBD_COMP_Private_Defines
 * @{
 */

/**
 * @}
 */


/** @defgroup USBD_COMP_Private_Macros
 * @{
 */


/**
 * @}
 */




/** @defgroup USBD_COMP_Private_FunctionPrototypes
 * @{
 */

static uint8_t  USBD_COMP_Init (USBD_HandleTypeDef *pdev,
        uint8_t cfgidx);

static uint8_t  USBD_COMP_DeInit (USBD_HandleTypeDef *pdev,
        uint8_t cfgidx);

static uint8_t  USBD_COMP_Setup (USBD_HandleTypeDef *pdev,
        USBD_SetupReqTypedef *req);

static uint8_t  *USBD_COMP_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_COMP_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_COMP_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_COMP_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_COMP_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_COMP_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_COMP_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_COMP_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_COMP_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

/**
 * @}
 */

 /** @defgroup USBD_COMP_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_COMP =
{
        USBD_COMP_Init,
        USBD_COMP_DeInit,
        USBD_COMP_Setup,
        USBD_COMP_EP0_TxReady,
        USBD_COMP_EP0_RxReady,
        USBD_COMP_DataIn,
        USBD_COMP_DataOut,
        USBD_COMP_SOF,
        USBD_COMP_IsoINIncomplete,
        USBD_COMP_IsoOutIncomplete,
        USBD_COMP_GetCfgDesc,
        USBD_COMP_GetCfgDesc,
        USBD_COMP_GetCfgDesc,
        USBD_COMP_GetDeviceQualifierDesc,
};

USBD_COMP_ItfTypeDef USBD_COMP_fops_FS;

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_COMP_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END=
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
 * @}
 */

/** @defgroup USBD_COMP_Private_Functions
 * @{
 */

/**
 * @brief  USBD_COMP_Init
 *         Initialize the AUDIO interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */


static inline void switchToClass(USBD_HandleTypeDef *pdev,USBD_ClassCompInfo* class)
{
    pdev->pClassData = class->classData;
    pdev->pUserData = class->userData;
}
static inline void saveClass(USBD_HandleTypeDef *pdev,USBD_ClassCompInfo* class)
{
    class->classData = pdev->pClassData;
    class->userData = pdev->pUserData;
}


static uint8_t  USBD_COMP_Init (USBD_HandleTypeDef *pdev,
        uint8_t cfgidx)
{
    uint8_t retval = USBD_OK;

    for (int i = 0; i < CLASS_NUM && retval == USBD_OK; i++)
    {
        switchToClass(pdev,&dev_instance[i]);
        retval = dev_instance[i].class->Init(pdev,cfgidx);
        saveClass(pdev,&dev_instance[i]);
    }
    return retval;
}

/**
 * @brief  USBD_COMP_Init
 *         DeInitialize the AUDIO layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t  USBD_COMP_DeInit (USBD_HandleTypeDef *pdev,
        uint8_t cfgidx)
{
    uint8_t retval = USBD_OK;

    for (int i = 0; i < CLASS_NUM && retval == USBD_OK; i++)
    {
        switchToClass(pdev,&dev_instance[i]);
        retval = dev_instance[i].class->DeInit(pdev,cfgidx);
        saveClass(pdev,&dev_instance[i]);
    }
    return retval;
}

/**
 * @brief  USBD_COMP_Setup
 *         Handle the AUDIO specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */

static uint8_t* USBD_COMP_FindDescriptor(uint8_t desc_type)
{

    uint8_t* retval = NULL;
    uint16_t desc_len;
    uint8_t* desc_ptr = USBD_COMP_GetCfgDesc(&desc_len);

    for (int32_t i = 0; i < desc_len; i += desc_ptr[i])
    {
        if (desc_ptr[i+1] == desc_type)
        {
            retval = &desc_ptr[i];
            break;
        }
    }
    return retval;
}

static uint8_t  USBD_COMP_Setup (USBD_HandleTypeDef *pdev,
        USBD_SetupReqTypedef *req)
{
    uint16_t len;
    uint8_t *pbuf;
    uint8_t ret = USBD_OK;
    uint8_t done = 0;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    case USB_REQ_TYPE_CLASS :
        switch(req->bmRequest & USB_REQ_RECIPIENT_MASK)
        {
        case USB_REQ_RECIPIENT_INTERFACE:
            for (int i = 0; i < CLASS_NUM; i++)
            {
                uint8_t interface = req->wValue >> 8;
                if (interface >= dev_instance[i].minIf && interface <= dev_instance[i].maxIf)
                {
                    switchToClass(pdev,&dev_instance[i]);
                    ret = dev_instance[i].class->Setup(pdev,req);
                    done = 1;
                    break;
                }
            }
            break;
        default:
            break;
        }
        if (done != 1)
        {
            USBD_CtlError (pdev, req);
            ret = USBD_FAIL;
        }
        break;

    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_DESCRIPTOR:
        {
            pbuf = USBD_COMP_FindDescriptor(req->wValue >> 8);

            if(pbuf != NULL)
            {
                len = MIN(pbuf[0], req->wLength);

                USBD_CtlSendData (pdev,
                        pbuf,
                        len);
            }
        }
        break;

        case USB_REQ_GET_INTERFACE :
        case USB_REQ_SET_INTERFACE :
            if ((uint8_t)(req->wIndex) <= USBD_MAX_NUM_INTERFACES)
            {
                for (int i = 0; i < CLASS_NUM; i++)
                {
                    if (req->wIndex >= dev_instance[i].minIf && req->wIndex <= dev_instance[i].maxIf)
                    {
                        switchToClass(pdev,&dev_instance[i]);
                        ret = dev_instance[i].class->Setup(pdev,req);
                        done = 1;
                        break;
                    }
                }

            }
            else
            {
                /* Call the error management function (command will be nacked */
                USBD_CtlError (pdev, req);
            }
            break;

        default:
            USBD_CtlError (pdev, req);
            ret = USBD_FAIL;
        }
    }
    return ret;
}


/**
 * @brief  USBD_COMP_GetCfgDesc
 *         return configuration descriptor
 * @param  speed : current device speed
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t  *USBD_COMP_GetCfgDesc (uint16_t *length)
{
    *length = sizeof (USBD_COMP_CfgDesc);
    return USBD_COMP_CfgDesc;
}


/**
 * @brief  USBD_COMP_DataIn
 *         handle data IN Stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t  USBD_COMP_DataIn (USBD_HandleTypeDef *pdev,
        uint8_t epnum)
{
    uint8_t retval = USBD_OK;
    if (epnum <= USBD_MAX_EP)
    {
        uint8_t classIdx = usbdEpMap.in[epnum];

        if (classIdx < CLASS_NUM)
        {
            if (dev_instance[classIdx].class->DataIn != NULL)
            {
                switchToClass(pdev,&dev_instance[classIdx]);
                retval = dev_instance[classIdx].class->DataIn(pdev,epnum);
            }
        }
    }
    return retval;
}

/**
 * @brief  USBD_COMP_EP0_RxReady
 *         handle EP0 Rx Ready event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t  USBD_COMP_EP0_RxReady (USBD_HandleTypeDef *pdev)
{
    uint8_t retval = USBD_OK;
    for (int i = 0; i < CLASS_NUM; i++)
    {
            if (dev_instance[i].class->EP0_RxReady != NULL)
            {
                switchToClass(pdev,&dev_instance[i]);
                retval = dev_instance[i].class->EP0_RxReady(pdev);
            }
            if (retval != USBD_OK)
            {
                break;
            }
    }
    return retval;
}
/**
 * @brief  USBD_COMP_EP0_TxReady
 *         handle EP0 TRx Ready event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t  USBD_COMP_EP0_TxReady (USBD_HandleTypeDef *pdev)
{
    uint8_t retval = USBD_OK;
    for (int i = 0; i < CLASS_NUM; i++)
    {
            if (dev_instance[i].class->EP0_TxSent != NULL)
            {
                switchToClass(pdev,&dev_instance[i]);
                retval = dev_instance[i].class->EP0_TxSent(pdev);
            }
            if (retval != USBD_OK)
            {
                break;
            }
    }
    return retval;
}
/**
 * @brief  USBD_COMP_SOF
 *         handle SOF event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t  USBD_COMP_SOF (USBD_HandleTypeDef *pdev)
{
    uint8_t retval = USBD_OK;
    for (int i = 0; i < CLASS_NUM; i++)
    {
            if (dev_instance[i].class->SOF != NULL)
            {
                switchToClass(pdev,&dev_instance[i]);
                retval = dev_instance[i].class->SOF(pdev);
                if (retval != USBD_OK)
                {
                    break;
                }
            }
    }
    return retval;
}


/**
 * @brief  USBD_COMP_IsoINIncomplete
 *         handle data ISO IN Incomplete event
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t  USBD_COMP_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    uint8_t retval = USBD_OK;
    for (int i = 0; i < CLASS_NUM; i++)
    {
            if (dev_instance[i].class->IsoINIncomplete != NULL)
            {
                switchToClass(pdev,&dev_instance[i]);
                retval = dev_instance[i].class->IsoINIncomplete(pdev, epnum);
                if (retval != USBD_OK)
                {
                    break;
                }
            }
    }
    return retval;
}
/**
 * @brief  USBD_COMP_IsoOutIncomplete
 *         handle data ISO OUT Incomplete event
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t  USBD_COMP_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    uint8_t retval = USBD_OK;
    for (int i = 0; i < CLASS_NUM; i++)
    {
            if (dev_instance[i].class->IsoOUTIncomplete != NULL)
            {
                switchToClass(pdev,&dev_instance[i]);
                retval = dev_instance[i].class->IsoOUTIncomplete(pdev, epnum);
                if (retval != USBD_OK)
                {
                    break;
                }
            }
    }
    return retval;
}
/**
 * @brief  USBD_COMP_DataOut
 *         handle data OUT Stage
 * @param  pdev: device instance
 * @param  epnum: endpoint index
 * @retval status
 */
static uint8_t  USBD_COMP_DataOut (USBD_HandleTypeDef *pdev,
        uint8_t epnum)
{
    uint8_t retval = USBD_OK;
    if (epnum <= USBD_MAX_EP)
    {
        uint8_t classIdx = usbdEpMap.out[epnum];

        if (classIdx < CLASS_NUM)
        {
            if (dev_instance[classIdx].class->DataOut != NULL)
            {
                switchToClass(pdev,&dev_instance[classIdx]);
                retval = dev_instance[classIdx].class->DataOut(pdev,epnum);
            }
        }
    }
    return retval;
}



/**
 * @brief  DeviceQualifierDescriptor
 *         return Device Qualifier descriptor
 * @param  length : pointer data length
 * @retval pointer to descriptor buffer
 */
static uint8_t  *USBD_COMP_GetDeviceQualifierDesc (uint16_t *length)
{
    *length = sizeof (USBD_COMP_DeviceQualifierDesc);
    return USBD_COMP_DeviceQualifierDesc;
}

/**
 * @brief  USBD_COMP_RegisterInterface
 * @param  fops: Audio interface callback
 * @retval status
 */
uint8_t  USBD_COMP_RegisterInterface  (USBD_HandleTypeDef   *pdev,
        USBD_COMP_ItfTypeDef *fops)
{
    if(fops != NULL)
    {
        pdev->pUserData= fops;
    }
    return 0;
}
/**
 * @}
 */


/**
 * @}
 */


/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
