#ifndef	_USBCTRL_HPP
#define _USBCTRL_HPP

class USB;
class USBHW;
class USBCTRL;

#include "USB.hpp"
#include "USBHW.hpp"

// TODO: make hardware-independent
#include "lpc17xx_usb.h"

class USBCTRL {
	static USBHW *hw;
	static USB *u;
	static const uint8_t *pabDescrip;
	static uint8_t bConfiguration;
	static uint8_t bAlternate;
	static TFnHandleRequest	*pfnHandleCustomReq;
	static TSetupPacket Setup;
public:
	USBCTRL();
	static void init(USB *,USBHW *);
	static void FrameHandler(uint16_t);
	static void DevIntHandler(uint8_t bDevStatus);
	static void EPIntHandler(uint8_t bEP, uint8_t bEPStatus);

	static uint8_t GetDescriptor(uint16_t wTypeIndex, uint16_t wLangID, int *piLen, uint8_t **ppbData);
	static uint8_t SetConfiguration(uint8_t bConfigIndex, uint8_t bAltSetting);
	static uint8_t HandleStdDeviceReq(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData);
	static uint8_t HandleStdInterfaceReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);
	static uint8_t HandleStdEndPointReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);
	static uint8_t HandleStandardRequest(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);
};

#endif /* _USBCTRL_HPP */
