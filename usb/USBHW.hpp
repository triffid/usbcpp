#ifndef _USBHW_HPP
#define _USBHW_HPP

#include <cstdint>

#include "lpc17xx_usb.h"

typedef void (TFnEPIntHandler)	(uint8_t bEP, uint8_t bEPStatus);
typedef void (TFnDevIntHandler)	(uint8_t bDevStatus);
typedef void (TFnFrameHandler)(uint16_t wFrame);
typedef uint8_t (TFnHandleRequest)(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData);

class USBHW;

class USBHW {
public:
	USBHW();
	USBHW(int HwPortIndex);

	static USBHW	*HwPorts[];

	static int		HwPortCount(void);
	static void		HwISR(void);

	TFnDevIntHandler	*_pfnDevIntHandler;
	TFnEPIntHandler	*_apfnEPIntHandlers[32];
	TFnFrameHandler	*_pfnFrameHandler;

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

	void			HwRegisterEPIntHandler(uint8_t bEP, TFnEPIntHandler *pfnHandler);
	void			HwRegisterDevIntHandler(TFnDevIntHandler *pfnHandler);
	void			HwRegisterFrameHandler(TFnFrameHandler *pfnHandler);

	void			HwSetAddress(uint8_t bAddr);
	void			HwConnect(uint8_t fConnect);
	void			HwNakIntEnable(uint8_t bIntBits);
	void			HwConfigDevice(uint8_t fConfigured);
};

#endif /* _USBHW_HPP */

