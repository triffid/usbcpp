#ifndef	_USBCTRL_HPP
#define _USBCTRL_HPP

#include "USBHW.hpp"

#include "descriptor.h"

class USBCTRL {
	static USBHW hw;
	static usbdesc_base ** descriptors;
	static usbdesc_device *dev;
	static usbdesc_configuration *conf;
	static uint8_t confBuffer[];

	static uint16_t confSize;
	static uint16_t confRemain;
	static uint8_t confIndex;
	static uint8_t confSubIndex;

	static uint8_t *pbData;
	static uint8_t iResidue;
	static int iLen;

	static uint8_t apbDataStore[4][8];

	static const uint8_t *pabDescrip;
	static uint8_t bConfiguration;
	static uint8_t bAlternate;

	static TFnHandleRequest	*pfnHandleCustomReq;
	static TSetupPacket Setup;

	static void FrameHandler(uint16_t);
	static void DevIntHandler(uint8_t bDevStatus);
	static void EPIntHandler(uint8_t bEP, uint8_t bEPStatus);

	static uint8_t HandleStdDeviceReq(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData);
	static uint8_t HandleStdInterfaceReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);
	static uint8_t HandleStdEndPointReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);
	static uint8_t HandleStandardRequest(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);

	static uint8_t GetDescriptor(uint16_t wTypeIndex, uint16_t wLangID, int *piLen, uint8_t **ppbData);

	static uint8_t SetConfiguration(uint8_t bConfigIndex, uint8_t bAltSetting);

	static void DataIn(void);

public:
	USBCTRL();
	static void init(usbdesc_base **);

	static void connect(void);
};

#endif /* _USBCTRL_HPP */
