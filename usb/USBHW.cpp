#include "USBHW.hpp"

#include <cstdio>

#include "LPC17xx.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_usb.h"

USBHW::USBHW() {}

void USBHW::init() {
	// P2.9 -> USB_CONNECT
	LPC_PINCON->PINSEL4 &= ~0x000C0000;
	LPC_PINCON->PINSEL4 |=  0x00040000;

	// P1.18 -> USB_UP_LED
	// P1.30 -> VBUS
	LPC_PINCON->PINSEL3 &= ~0x30000030;
	LPC_PINCON->PINSEL3 |=  0x20000010;

	// P0.29 -> USB_D+
	// P0.30 -> USB_D-
	LPC_PINCON->PINSEL1 &= ~0x3C000000;
	LPC_PINCON->PINSEL1 |=  0x14000000;

	// enable PUSB
	LPC_SC->PCONP |= CLKPWR_PCONP_PCUSB;

	LPC_USB->USBClkCtrl = 0x12;	                  /* Dev clock, AHB clock enable  */
	while ((LPC_USB->USBClkSt & 0x12) != 0x12);

	// disable/clear all interrupts for now
	LPC_USB->USBDevIntEn = 0;
	LPC_USB->USBDevIntClr = 0xFFFFFFFF;
	LPC_USB->USBDevIntPri = 0;

	LPC_USB->USBEpIntEn = 0;
	LPC_USB->USBEpIntClr = 0xFFFFFFFF;
	LPC_USB->USBEpIntPri = 0;

	// by default, only ACKs generate interrupts
	HwNakIntEnable(0);

	NVIC_EnableIRQ(USB_IRQn);

// 	LPC_USB->USBDevIntEn |= FRAME | DEV_STAT | EP_SLOW;
}

void USBHW::connect() {
	HwConnect(TRUE);
}

void USBHW::disconnect() {
	HwConnect(FALSE);
}

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


/** @file
 *	USB hardware layer
 */

/** Installed device interrupt handler */
TFnDevIntHandler *USBHW::_pfnDevIntHandler = (TFnDevIntHandler *) NULL;
/** Installed endpoint interrupt handlers */
TFnEPIntHandler	*USBHW::_apfnEPIntHandlers[32];
/** Installed frame interrupt handlers */
TFnFrameHandler	*USBHW::_pfnFrameHandler = (TFnFrameHandler *) NULL;

/** convert from endpoint address to endpoint index */
#define EP2IDX(bEP)	((((bEP)&0xF)<<1)|(((bEP)&0x80)>>7))
/** convert from endpoint index to endpoint address */
#define IDX2EP(idx)	((((idx)<<7)&0x80)|(((idx)>>1)&0xF))

/**
 *	Local function to wait for a device interrupt (and clear it)
 *
 *	@param [in]	dwIntr		Bitmask of interrupts to wait for
 */
void USBHW::Wait4DevInt(uint32_t dwIntr) {
	// wait for specific interrupt
	while ((LPC_USB->USBDevIntSt & dwIntr) != dwIntr);
	// clear the interrupt bits
	LPC_USB->USBDevIntClr = dwIntr;
}


/**
 *	Local function to send a command to the USB protocol engine
 *
 *	@param [in]	bCmd		Command to send
 */
void USBHW::HwCmd(uint8_t bCmd) {
	// clear CDFULL/CCEMTY
	LPC_USB->USBDevIntClr = CDFULL | CCEMTY;
	// write command code
	LPC_USB->USBCmdCode = 0x00000500 | (bCmd << 16);
	Wait4DevInt(CCEMTY);
}


/**
 *	Local function to send a command + data to the USB protocol engine
 *
 *	@param [in]	bCmd		Command to send
 *	@param [in]	bData		Data to send
 */
void USBHW::HwCmdWrite(uint8_t bCmd, uint16_t bData) {
	// write command code
	HwCmd(bCmd);

	// write command data
	LPC_USB->USBCmdCode = 0x00000100 | (bData << 16);
	Wait4DevInt(CCEMTY);
}


/**
 *	Local function to send a command to the USB protocol engine and read data
 *
 *	@param [in]	bCmd		Command to send
 *
 *	@return the data
 */
uint8_t USBHW::HwCmdRead(uint8_t bCmd) {
	// write command code
	HwCmd(bCmd);

	// get data
	LPC_USB->USBCmdCode = 0x00000200 | (bCmd << 16);
	Wait4DevInt(CDFULL);
	return LPC_USB->USBCmdData;
}


/**
 *	'Realizes' an endpoint, meaning that buffer space is reserved for
 *	it. An endpoint needs to be realised before it can be used.
 *
 *	From experiments, it appears that a USB reset causes USBReEP to
 *	re-initialise to 3 (= just the control endpoints).
 *	However, a USB bus reset does not disturb the USBMaxPSize settings.
 *
 *	@param [in]	idx			Endpoint index
 *	@param [in] wMaxPSize	Maximum packet size for this endpoint
 */
void USBHW::HwEPRealize(int idx, uint16_t wMaxPSize) {
// 	DBG("EP Realize %x\n", IDX2EP(idx));
	LPC_USB->USBReEP |= (1 << idx);
	LPC_USB->USBEpInd = idx;
	LPC_USB->USBMaxPSize = wMaxPSize;
	Wait4DevInt(EP_RLZED);
}


/**
 *	Enables or disables an endpoint
 *
 *	@param [in]	idx		Endpoint index
 *	@param [in]	fEnable	TRUE to enable, FALSE to disable
 */
void USBHW::HwEPEnable(int idx, uint8_t fEnable) {
// 	DBG("EP Enable %x\n", IDX2EP(idx));
	HwCmdWrite(CMD_EP_SET_STATUS | idx, fEnable ? 0 : EP_DA);
}


/**
 *	Configures an endpoint and enables it
 *
 *	@param [in]	bEP				Endpoint number
 *	@param [in]	wMaxPacketSize	Maximum packet size for this EP
 */
void USBHW::HwEPConfig(uint8_t bEP, uint16_t wMaxPacketSize) {
	int idx;

	idx = EP2IDX(bEP);

// 	DBG("EP Configure %x\n", bEP);

	// realise EP
	HwEPRealize(idx, wMaxPacketSize);

	// enable EP
	HwEPEnable(idx, TRUE);
}


/**
 *	Registers an endpoint event callback
 *
 *	@param [in]	bEP				Endpoint number
 *	@param [in]	pfnHandler		Callback function
 */
void USBHW::HwRegisterEPIntHandler(uint8_t bEP, TFnEPIntHandler *pfnHandler) {
	int idx;

	idx = EP2IDX(bEP);

// 	ASSERT(idx<32);

	/* add handler to list of EP handlers */
	_apfnEPIntHandlers[idx] = pfnHandler;

	/* enable EP interrupt */
	LPC_USB->USBEpIntEn |= (1 << idx);
	LPC_USB->USBDevIntEn |= EP_SLOW;

// 	DBG("Registered handler for EP 0x%x\n", bEP);
}


/**
 *	Registers an device status callback
 *
 *	@param [in]	pfnHandler	Callback function
 */
void USBHW::HwRegisterDevIntHandler(TFnDevIntHandler *pfnHandler) {
	_pfnDevIntHandler = pfnHandler;

	// enable device interrupt
	LPC_USB->USBDevIntEn |= DEV_STAT;

// 	DBG("Registered handler for device status\n");
}


/**
 *	Registers the frame callback
 *
 *	@param [in]	pfnHandler	Callback function
 */
void USBHW::HwRegisterFrameHandler(TFnFrameHandler *pfnHandler) {
	_pfnFrameHandler = pfnHandler;

	// enable device interrupt
	LPC_USB->USBDevIntEn |= FRAME;

// 	DBG("Registered handler for frame\n");
}


/**
 *	Sets the USB address.
 *
 *	@param [in]	bAddr		Device address to set
 */
void USBHW::HwSetAddress(uint8_t bAddr) {
	HwCmdWrite(CMD_DEV_SET_ADDRESS, DEV_EN | bAddr);
}


/**
 *	Connects or disconnects from the USB bus
 *
 *	@param [in]	fConnect	If TRUE, connect, otherwise disconnect
 */
void USBHW::HwConnect(uint8_t fConnect) {
	HwCmdWrite(CMD_DEV_STATUS, fConnect ? CON : 0);
}


/**
 *	Enables interrupt on NAK condition
 *
 *	For IN endpoints a NAK is generated when the host wants to read data
 *	from the device, but none is available in the endpoint buffer.
 *	For OUT endpoints a NAK is generated when the host wants to write data
 *	to the device, but the endpoint buffer is still full.
 *
 *	The endpoint interrupt handlers can distinguish regular (ACK) interrupts
 *	from NAK interrupt by checking the bits in their bEPStatus argument.
 *
 *	@param [in]	bIntBits	Bitmap indicating which NAK interrupts to enable
 */
void USBHW::HwNakIntEnable(uint8_t bIntBits) {
	HwCmdWrite(CMD_DEV_SET_MODE, bIntBits);
}


/**
 *	Gets the status from a specific endpoint.
 *
 *	@param [in]	bEP		Endpoint number
 *	@return Endpoint status byte (containing EP_STATUS_xxx bits)
 */
uint8_t	USBHW::HwEPGetStatus(uint8_t bEP) {
	int idx = EP2IDX(bEP);

	return HwCmdRead(CMD_EP_SELECT | idx);
}


/**
 *	Sets the stalled property of an endpoint
 *
 *	@param [in]	bEP		Endpoint number
 *	@param [in]	fStall	TRUE to stall, FALSE to unstall
 */
void USBHW::HwEPStall(uint8_t bEP, uint8_t fStall) {
	int idx = EP2IDX(bEP);

	HwCmdWrite(CMD_EP_SET_STATUS | idx, fStall ? EP_ST : 0);
}


/**
 *	Writes data to an endpoint buffer
 *
 *	@param [in]	bEP		Endpoint number
 *	@param [in]	pbBuf	Endpoint data
 *	@param [in]	iLen	Number of bytes to write
 *
 *	@return TRUE if the data was successfully written or <0 in case of error.
 */
int USBHW::HwEPWrite(uint8_t bEP, uint8_t *pbBuf, int iLen) {
	int idx;

	idx = EP2IDX(bEP);

	// set write enable for specific endpoint
	LPC_USB->USBCtrl = WR_EN | ((bEP & 0xF) << 2);

	// set packet length
	LPC_USB->USBTxPLen = iLen;

	// write data
	while (LPC_USB->USBCtrl & WR_EN) {
		LPC_USB->USBTxData = (pbBuf[3] << 24) | (pbBuf[2] << 16) | (pbBuf[1] << 8) | pbBuf[0];
		pbBuf += 4;
	}

	// select endpoint and validate buffer
	HwCmd(CMD_EP_SELECT | idx);
	HwCmd(CMD_EP_VALIDATE_BUFFER);

	return iLen;
}


/**
 *	Reads data from an endpoint buffer
 *
 *	@param [in]	bEP		Endpoint number
 *	@param [in]	pbBuf	Endpoint data
 *	@param [in]	iMaxLen	Maximum number of bytes to read
 *
 *	@return the number of bytes available in the EP (possibly more than iMaxLen),
 *	or <0 in case of error.
 */
int USBHW::HwEPRead(uint8_t bEP, uint8_t *pbBuf, int iMaxLen) {
	int i, idx, j;
	uint32_t	dwData, dwLen;

	idx = EP2IDX(bEP);

	// set read enable bit for specific endpoint
	LPC_USB->USBCtrl = RD_EN | ((bEP & 0xF) << 2);

	// wait for PKT_RDY
	do {
		dwLen = LPC_USB->USBRxPLen;
	} while ((dwLen & PKT_RDY) == 0);

	// packet valid?
	if ((dwLen & DV) == 0) {
		return -1;
	}

	// get length
	dwLen &= PKT_LNGTH_MASK;

	// 	printf("[RDEP%02X:%lu/%d", bEP, dwLen, iMaxLen);

	// get data
	dwData = 0;
	for (i = 0, j = 0; LPC_USB->USBCtrl & RD_EN; i += 4) {
		//if ((i % 4) == 0) {
			dwData = LPC_USB->USBRxData;
			//}
			for (j=0; j < 4; j++) {
				if ((pbBuf != NULL) && ((i + j) < iMaxLen)) {
					pbBuf[(i + j)] = dwData & 0xFF;
				}
				dwData >>= 8;
			}
	}

	// 	printf(":%d]", i + j);

	// make sure RD_EN is clear
	LPC_USB->USBCtrl = 0;

	// select endpoint and clear buffer
	HwCmd(CMD_EP_SELECT | idx);
	HwCmd(CMD_EP_CLEAR_BUFFER);

	return dwLen;
}


/**
 *	Sets the 'configured' state.
 *
 *	All registered endpoints are 'realised' and enabled, and the
 *	'configured' bit is set in the device status register.
 *
 *	@param [in]	fConfigured	If TRUE, configure device, else unconfigure
 */
void USBHW::HwConfigDevice(uint8_t fConfigured) {
	// set configured bit
	HwCmdWrite(CMD_DEV_CONFIG, fConfigured ? CONF_DEVICE : 0);
}

/**
 *	USB interrupt handler
 *
 *	@todo Get all 11 bits of frame number instead of just 8
 *
 *	Endpoint interrupts are mapped to the slow interrupt
 */
void USBHW::HwISR(void) {
	uint32_t	dwStatus;
	uint32_t dwIntBit;
	uint8_t	bEPStat, bDevStat, bStat;
	int i;
	uint16_t	wFrame;

	// handle device interrupts
	dwStatus = LPC_USB->USBDevIntSt;

	printf("DS:%lX \t", dwStatus);

	// frame interrupt
	if (dwStatus & FRAME) {
		// clear int
		LPC_USB->USBDevIntClr = FRAME;
		// call handler
		if (_pfnFrameHandler != NULL) {
			wFrame = HwCmdRead(CMD_DEV_READ_CUR_FRAME_NR);
			_pfnFrameHandler(wFrame);
		}
	}

	// device status interrupt
	if (dwStatus & DEV_STAT) {
		/*	Clear DEV_STAT interrupt before reading DEV_STAT register.
		 *			This prevents corrupted device status reads, see
		 *			LPC2148 User manual revision 2, 25 july 2006.
		 */
		LPC_USB->USBDevIntClr = DEV_STAT;
		bDevStat = HwCmdRead(CMD_DEV_STATUS);
		if (bDevStat & (CON_CH | SUS_CH | RST)) {
			// convert device status into something HW independent
			bStat = ((bDevStat & CON) ? DEV_STATUS_CONNECT : 0) |
			((bDevStat & SUS) ? DEV_STATUS_SUSPEND : 0) |
			((bDevStat & RST) ? DEV_STATUS_RESET : 0);
			// call handler
			if (_pfnDevIntHandler != NULL) {
				_pfnDevIntHandler(bStat);
			}
		}
	}

	// endpoint interrupt
	if (dwStatus & EP_SLOW) {
		// clear EP_SLOW
		LPC_USB->USBDevIntClr = EP_SLOW;
		// check all endpoints
		for (i = 0; i < 32; i++) {
			dwIntBit = (1 << i);
			if (LPC_USB->USBEpIntSt & dwIntBit) {
				// clear int (and retrieve status)
				LPC_USB->USBEpIntClr = dwIntBit;
				Wait4DevInt(CDFULL);
				bEPStat = LPC_USB->USBCmdData;
				// convert EP pipe stat into something HW independent
				bStat = ((bEPStat & EPSTAT_FE) ? EP_STATUS_DATA : 0) |
				((bEPStat & EPSTAT_ST) ? EP_STATUS_STALLED : 0) |
				((bEPStat & EPSTAT_STP) ? EP_STATUS_SETUP : 0) |
				((bEPStat & EPSTAT_EPN) ? EP_STATUS_NACKED : 0) |
				((bEPStat & EPSTAT_PO) ? EP_STATUS_ERROR : 0);
				// call handler
				if (_apfnEPIntHandlers[i] != NULL) {
					_apfnEPIntHandlers[i](IDX2EP(i), bStat);
				}
			}
		}
	}
}

extern "C" {
	__attribute__ ((interrupt)) void USB_IRQHandler()  {
		printf("!");
		USBHW::HwISR();
	}
}