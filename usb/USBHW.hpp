#ifndef _USBHW_HPP
#define _USBHW_HPP

#include <cstdint>

#include "lpc17xx_usb.h"

/** convert from endpoint address to endpoint index */
#define EP2IDX(bEP)	((((bEP)&0xF)<<1)|(((bEP)&0x80)>>7))

/** convert from endpoint index to endpoint address */
#define IDX2EP(idx)	((((idx)<<7)&0x80)|(((idx)>>1)&0xF))

typedef void (TFnEPIntHandler)	(uint8_t bEP, uint8_t bEPStatus);
typedef void (TFnDevIntHandler)	(uint8_t bDevStatus);
typedef void (TFnFrameHandler)(uint16_t wFrame);
typedef uint8_t (TFnHandleRequest)(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData);

class USBHW;

class USB_EP_Receiver {
public:
	virtual void EPIntHandler(USBHW *u, uint8_t bEP, uint8_t bEPStatus) = 0;
};

class USB_DevInt_Receiver {
public:
	virtual void DevIntHandler(USBHW *u, uint8_t bDevStatus) = 0;
};

class USB_Frame_Receiver {
public:
	virtual void FrameHandler(USBHW *u, uint16_t wFrame) = 0;
};

class USB_Setup_Receiver {
public:
	virtual void SetupHandler(USBHW *u, uint8_t bEP, TSetupPacket *Setup) = 0;
};

class USBHW : public USB_EP_Receiver, public USB_DevInt_Receiver, USB_Frame_Receiver, USB_Setup_Receiver {
public:
	USBHW();
	USBHW(int HwPortIndex);

	static USBHW	*HwPorts[];

	static int		HwPortCount(void);
	static void		HwISR(void);

	static uint32_t		HwGetSerialNumber(int, uint32_t *);

// 	TFnDevIntHandler	*_pfnDevIntHandler;
// 	TFnEPIntHandler	*_apfnEPIntHandlers[32];
// 	TFnFrameHandler	*_pfnFrameHandler;

	void EPIntHandler(USBHW *, uint8_t bEP, uint8_t bEPStatus);
	void DevIntHandler(USBHW *, uint8_t bDevStatus);
	void FrameHandler(USBHW *, uint16_t wFrame);
	void SetupHandler(USBHW *, uint8_t bEP, TSetupPacket *Setup);

	void			Wait4DevInt(uint32_t dwIntr);

	void			init(void);
	void			connect(void);
	void			disconnect(void);

	void			HwEPRealize(int idx, uint16_t wMaxPSize);
	void			HwEPEnable(int idx, uint8_t fEnable);
	void			HwEPConfig(uint8_t bEP, uint16_t wMaxPacketSize);
	uint8_t			HwEPGetStatus(uint8_t bEP);
	void			HwEPStall(uint8_t bEP, uint8_t fStall);
	int				HwEPWrite(uint8_t bEP, uint8_t *pbBuf, int iLen);
	int				HwEPRead(uint8_t bEP, uint8_t *pbBuf, int iMaxLen);

	void			HwCmd(uint8_t bCmd);
	void			HwCmdWrite(uint8_t bCmd, uint16_t bData);
	uint8_t			HwCmdRead(uint8_t bCmd);
	uint16_t		HwCmdRead16(uint8_t bCmd);

// 	void			HwRegisterEPIntHandler(uint8_t bEP, TFnEPIntHandler *pfnHandler);
// 	void			HwRegisterDevIntHandler(TFnDevIntHandler *pfnHandler);
// 	void			HwRegisterFrameHandler(TFnFrameHandler *pfnHandler);

	void			HwSetAddress(uint8_t bAddr);
	void			HwConnect(uint8_t fConnect);
	void			HwNakIntEnable(uint8_t bIntBits);
	void			HwConfigDevice(uint8_t fConfigured);
};

#endif /* _USBHW_HPP */

