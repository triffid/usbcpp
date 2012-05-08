#ifndef _USBHW_HPP
#define _USBHW_HPP

#include <cstdint>

#include "lpc17xx_usb.h"

typedef void (TFnEPIntHandler)	(uint8_t bEP, uint8_t bEPStatus);
typedef void (TFnDevIntHandler)	(uint8_t bDevStatus);
typedef void (TFnFrameHandler)(uint16_t wFrame);
typedef uint8_t (TFnHandleRequest)(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData);

class USBHW {
	static TFnDevIntHandler *_pfnDevIntHandler;
	static TFnEPIntHandler	*_apfnEPIntHandlers[32];
	static TFnFrameHandler	*_pfnFrameHandler;

	static void		Wait4DevInt(uint32_t dwIntr);
public:
	USBHW();
	static void		init(void);
	static void		connect(void);
	static void		disconnect(void);

	static void		HwEPRealize(int idx, uint16_t wMaxPSize);
	static void		HwEPEnable(int idx, uint8_t fEnable);
	static void		HwEPConfig(uint8_t bEP, uint16_t wMaxPacketSize);
	static uint8_t	HwEPGetStatus(uint8_t bEP);
	static void		HwEPStall(uint8_t bEP, uint8_t fStall);
	static int		HwEPWrite(uint8_t bEP, uint8_t *pbBuf, int iLen);
	static int		HwEPRead(uint8_t bEP, uint8_t *pbBuf, int iMaxLen);

	static void		HwCmd(uint8_t bCmd);
	static void		HwCmdWrite(uint8_t bCmd, uint16_t bData);
	static uint8_t	HwCmdRead(uint8_t bCmd);

	static void		HwRegisterEPIntHandler(uint8_t bEP, TFnEPIntHandler *pfnHandler);
	static void		HwRegisterDevIntHandler(TFnDevIntHandler *pfnHandler);
	static void		HwRegisterFrameHandler(TFnFrameHandler *pfnHandler);

	static void		HwSetAddress(uint8_t bAddr);
	static void		HwConnect(uint8_t fConnect);
	static void		HwNakIntEnable(uint8_t bIntBits);
	static void		HwConfigDevice(uint8_t fConfigured);
	static void		HwISR(void);
};

#endif /* _USBHW_HPP */

