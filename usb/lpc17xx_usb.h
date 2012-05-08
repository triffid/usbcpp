/*
 LPCUSB, an USB device driver for LPC microcontrollers
 Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. The name of the author may not be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef __LPC17XX__
#ifndef _LPC17XX_USB
#define _LPC17XX_USB

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *	USB configuration
 **************************************************************************/

#define MAX_PACKET_SIZE0	64		/**< maximum packet size for EP 0 */

/*************************************************************************
 *	USB hardware interface
 **************************************************************************/

// endpoint status sent through callback
#define EP_STATUS_DATA		(1<<0)		/**< EP has data */
#define EP_STATUS_STALLED	(1<<1)		/**< EP is stalled */
#define EP_STATUS_SETUP		(1<<2)		/**< EP received setup packet */
#define EP_STATUS_ERROR		(1<<3)		/**< EP data was overwritten by setup packet */
#define EP_STATUS_NACKED	(1<<4)		/**< EP sent NAK */

// device status sent through callback
#define DEV_STATUS_CONNECT		(1<<0)	/**< device just got connected */
#define DEV_STATUS_SUSPEND		(1<<2)	/**< device entered suspend state */
#define DEV_STATUS_RESET		(1<<4)	/**< device just got reset */

// interrupt bits for NACK events in USBHwNakIntEnable
// (these bits conveniently coincide with the LPC176x USB controller bit)
#define INACK_CI		(1<<1)			/**< interrupt on NACK for control in */
#define INACK_CO		(1<<2)			/**< interrupt on NACK for control out */
#define INACK_II		(1<<3)			/**< interrupt on NACK for interrupt in */
#define INACK_IO		(1<<4)			/**< interrupt on NACK for interrupt out */
#define INACK_BI		(1<<5)			/**< interrupt on NACK for bulk in */
#define INACK_BO		(1<<6)			/**< interrupt on NACK for bulk out */

/* USBIntSt bits */
#define USB_INT_REQ_LP				(1<<0)
#define USB_INT_REQ_HP				(1<<1)
#define USB_INT_REQ_DMA				(1<<2)
#define USB_need_clock				(1<<8)
#define EN_USB_BITS					(1<<31)

/* USBDevInt... bits */
#define FRAME						(1<<0)
#define EP_FAST						(1<<1)
#define EP_SLOW						(1<<2)
#define DEV_STAT					(1<<3)
#define CCEMTY						(1<<4)
#define CDFULL						(1<<5)
#define RxENDPKT					(1<<6)
#define TxENDPKT					(1<<7)
#define EP_RLZED					(1<<8)
#define ERR_INT						(1<<9)

/* USBRxPLen bits */
#define PKT_LNGTH					(1<<0)
#define PKT_LNGTH_MASK				0x3FF
#define DV							(1<<10)
#define PKT_RDY						(1<<11)

/* USBCtrl bits */
#define RD_EN						(1<<0)
#define WR_EN						(1<<1)
#define LOG_ENDPOINT				(1<<2)

/* protocol engine command codes */
/* device commands */
#define CMD_DEV_SET_ADDRESS			0xD0
#define CMD_DEV_CONFIG				0xD8
#define CMD_DEV_SET_MODE			0xF3
#define CMD_DEV_READ_CUR_FRAME_NR	0xF5
#define CMD_DEV_READ_TEST_REG		0xFD
#define CMD_DEV_STATUS				0xFE		/* read/write */
#define CMD_DEV_GET_ERROR_CODE		0xFF
#define CMD_DEV_READ_ERROR_STATUS	0xFB

/* endpoint commands */
#define CMD_EP_SELECT				0x00
#define CMD_EP_SELECT_CLEAR			0x40
#define CMD_EP_SET_STATUS			0x40
#define CMD_EP_CLEAR_BUFFER			0xF2
#define CMD_EP_VALIDATE_BUFFER		0xFA

/* set address command */
#define DEV_ADDR					(1<<0)
#define DEV_EN						(1<<7)

/* configure device command */
#define CONF_DEVICE					(1<<0)

/* set mode command */
#define AP_CLK						(1<<0)
#define INAK_CI						(1<<1)
#define INAK_CO						(1<<2)
#define INAK_II						(1<<3)
#define INAK_IO						(1<<4)
#define INAK_BI						(1<<5)
#define INAK_BO						(1<<6)

/* set get device status command */
#define CON							(1<<0)
#define CON_CH						(1<<1)
#define SUS							(1<<2)
#define SUS_CH						(1<<3)
#define RST							(1<<4)

/* get error code command */
// ...

/* Select Endpoint command read bits */
#define EPSTAT_FE					(1<<0)
#define EPSTAT_ST					(1<<1)
#define EPSTAT_STP					(1<<2)
#define EPSTAT_PO					(1<<3)
#define EPSTAT_EPN					(1<<4)
#define EPSTAT_B1FULL				(1<<5)
#define EPSTAT_B2FULL				(1<<6)

/* CMD_EP_SET_STATUS command */
#define EP_ST						(1<<0)
#define EP_DA						(1<<5)
#define EP_RF_MO					(1<<6)
#define EP_CND_ST					(1<<7)

/* read error status command */
#define PID_ERR						(1<<0)
#define UEPKT						(1<<1)
#define DCRC						(1<<2)
#define TIMEOUT						(1<<3)
#define EOP							(1<<4)
#define B_OVRN						(1<<5)
#define BTSTF						(1<<6)
#define TGL_ERR						(1<<7)

/** setup packet definitions */
typedef struct {
	uint8_t		bmRequestType;			/**< characteristics of the specific request */
	uint8_t		bRequest;				/**< specific request */
	uint16_t	wValue;					/**< request specific parameter */
	uint16_t	wIndex;					/**< request specific parameter */
	uint16_t	wLength;				/**< length of data transfered in data phase */
} TSetupPacket;


#define REQTYPE_GET_DIR(x)		(((x)>>7)&0x01)
#define REQTYPE_GET_TYPE(x)		(((x)>>5)&0x03)
#define REQTYPE_GET_RECIP(x)	((x)&0x1F)

#define REQTYPE_DIR_TO_DEVICE	0
#define REQTYPE_DIR_TO_HOST		1

#define REQTYPE_TYPE_STANDARD	0
#define REQTYPE_TYPE_CLASS		1
#define REQTYPE_TYPE_VENDOR		2
#define REQTYPE_TYPE_RESERVED	3

#define REQTYPE_RECIP_DEVICE	0
#define REQTYPE_RECIP_INTERFACE	1
#define REQTYPE_RECIP_ENDPOINT	2
#define REQTYPE_RECIP_OTHER		3

/* standard requests */
#define	REQ_GET_STATUS			0x00
#define REQ_CLEAR_FEATURE		0x01
#define REQ_SET_FEATURE			0x03
#define REQ_SET_ADDRESS			0x05
#define REQ_GET_DESCRIPTOR		0x06
#define REQ_SET_DESCRIPTOR		0x07
#define REQ_GET_CONFIGURATION	0x08
#define REQ_SET_CONFIGURATION	0x09
#define REQ_GET_INTERFACE		0x0A
#define REQ_SET_INTERFACE		0x0B
#define REQ_SYNCH_FRAME			0x0C

/* class requests HID */
#define HID_GET_REPORT			0x01
#define HID_GET_IDLE			0x02
#define HID_GET_PROTOCOL	 	0x03
#define HID_SET_REPORT			0x09
#define HID_SET_IDLE			0x0A
#define HID_SET_PROTOCOL		0x0B

/* feature selectors */
#define FEA_ENDPOINT_HALT		0x00
#define FEA_REMOTE_WAKEUP		0x01
#define FEA_TEST_MODE			0x02

/*
 *	USB descriptors
 */

/** USB descriptor header */
typedef struct {
	uint8_t	bLength;			/**< descriptor length */
	uint8_t	bDescriptorType;	/**< descriptor type */
} TUSBDescHeader;

#define DESC_DEVICE				1
#define DESC_CONFIGURATION		2
#define DESC_STRING				3
#define DESC_INTERFACE			4
#define DESC_ENDPOINT			5
#define DESC_DEVICE_QUALIFIER	6
#define DESC_OTHER_SPEED		7
#define DESC_INTERFACE_POWER	8

#define DESC_HID_HID			0x21
#define DESC_HID_REPORT			0x22
#define DESC_HID_PHYSICAL		0x23

#define GET_DESC_TYPE(x)		(((x)>>8)&0xFF)
#define GET_DESC_INDEX(x)		((x)&0xFF)


/*
 *	LPCUSB, an USB device driver for LPC microcontrollers
 *	Copyright (C) 2006 Bertrik Sikken (bertrik@sikken.nl)
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions are met:
 *
 *	1. Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions and the following disclaimer.
 *	2. Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *	3. The name of the author may not be used to endorse or promote products
 *	   derived from this software without specific prior written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *	IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *	NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/**
 *	Hardware definitions for the LPC176x USB controller
 *
 *	These are private to the usbhw module
 */

// CodeRed - pull in defines from NXP header file
#include "LPC17xx.h"


// CodeRed - these registers have been renamed on LPC176x
#define USBReEP USBReEp
#define OTG_CLK_CTRL USBClkCtrl
#define OTG_CLK_STAT USBClkSt

/* USBIntSt bits */
#define USB_INT_REQ_LP				(1<<0)
#define USB_INT_REQ_HP				(1<<1)
#define USB_INT_REQ_DMA				(1<<2)
#define USB_need_clock				(1<<8)
#define EN_USB_BITS					(1<<31)

/* USBDevInt... bits */
#define FRAME						(1<<0)
#define EP_FAST						(1<<1)
#define EP_SLOW						(1<<2)
#define DEV_STAT					(1<<3)
#define CCEMTY						(1<<4)
#define CDFULL						(1<<5)
#define RxENDPKT					(1<<6)
#define TxENDPKT					(1<<7)
#define EP_RLZED					(1<<8)
#define ERR_INT						(1<<9)

/* USBRxPLen bits */
#define PKT_LNGTH					(1<<0)
#define PKT_LNGTH_MASK				0x3FF
#define DV							(1<<10)
#define PKT_RDY						(1<<11)

/* USBCtrl bits */
#define RD_EN						(1<<0)
#define WR_EN						(1<<1)
#define LOG_ENDPOINT				(1<<2)

/* protocol engine command codes */
/* device commands */
#define CMD_DEV_SET_ADDRESS			0xD0
#define CMD_DEV_CONFIG				0xD8
#define CMD_DEV_SET_MODE			0xF3
#define CMD_DEV_READ_CUR_FRAME_NR	0xF5
#define CMD_DEV_READ_TEST_REG		0xFD
#define CMD_DEV_STATUS				0xFE		/* read/write */
#define CMD_DEV_GET_ERROR_CODE		0xFF
#define CMD_DEV_READ_ERROR_STATUS	0xFB
/* endpoint commands */
#define CMD_EP_SELECT				0x00
#define CMD_EP_SELECT_CLEAR			0x40
#define CMD_EP_SET_STATUS			0x40
#define CMD_EP_CLEAR_BUFFER			0xF2
#define CMD_EP_VALIDATE_BUFFER		0xFA

/* set address command */
#define DEV_ADDR					(1<<0)
#define DEV_EN						(1<<7)

/* configure device command */
#define CONF_DEVICE					(1<<0)

/* set mode command */
#define AP_CLK						(1<<0)
#define INAK_CI						(1<<1)
#define INAK_CO						(1<<2)
#define INAK_II						(1<<3)
#define INAK_IO						(1<<4)
#define INAK_BI						(1<<5)
#define INAK_BO						(1<<6)

/* set get device status command */
#define CON							(1<<0)
#define CON_CH						(1<<1)
#define SUS							(1<<2)
#define SUS_CH						(1<<3)
#define RST							(1<<4)

/* get error code command */
// ...

/* Select Endpoint command read bits */
#define EPSTAT_FE					(1<<0)
#define EPSTAT_ST					(1<<1)
#define EPSTAT_STP					(1<<2)
#define EPSTAT_PO					(1<<3)
#define EPSTAT_EPN					(1<<4)
#define EPSTAT_B1FULL				(1<<5)
#define EPSTAT_B2FULL				(1<<6)

/* CMD_EP_SET_STATUS command */
#define EP_ST						(1<<0)
#define EP_DA						(1<<5)
#define EP_RF_MO					(1<<6)
#define EP_CND_ST					(1<<7)

/* read error status command */
#define PID_ERR						(1<<0)
#define UEPKT						(1<<1)
#define DCRC						(1<<2)
#define TIMEOUT						(1<<3)
#define EOP							(1<<4)
#define B_OVRN						(1<<5)
#define BTSTF						(1<<6)
#define TGL_ERR						(1<<7)


/* general descriptor field offsets */
#define DESC_bLength					0	/**< length offset */
#define DESC_bDescriptorType			1	/**< descriptor type offset */

/* config descriptor field offsets */
#define CONF_DESC_wTotalLength			2	/**< total length offset */
#define CONF_DESC_bConfigurationValue	5	/**< configuration value offset */
#define CONF_DESC_bmAttributes			7	/**< configuration characteristics */

/* interface descriptor field offsets */
#define INTF_DESC_bAlternateSetting		3	/**< alternate setting offset */

/* endpoint descriptor field offsets */
#define ENDP_DESC_bEndpointAddress		2	/**< endpoint address offset */
#define ENDP_DESC_wMaxPacketSize		4	/**< maximum packet size offset */


#ifdef __cplusplus
}
#endif

#endif /* _LPC17XX_USB */
#endif /* __LPC17XX__ */
