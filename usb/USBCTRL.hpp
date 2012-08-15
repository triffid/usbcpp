#ifndef	_USBCTRL_HPP
#define _USBCTRL_HPP

#include "USBHW.hpp"

#include "descriptor.h"

class USBCTRL : public USBHW {
// 	USBHW hw;
	usbdesc_base ** descriptors;
	usbdesc_device *dev;
	usbdesc_configuration *conf;

// 	USB_EP_Receiver *EpHolders[16];

	uint8_t confBuffer[64];

	uint16_t confSize;
	uint16_t confRemain;
	uint8_t confIndex;
	uint8_t confSubIndex;

	uint8_t *pbData;
	uint16_t iResidue;
	int iLen;

	uint8_t apbDataStore[4][8];

	const uint8_t *pabDescrip;
	uint8_t bConfiguration;
	uint8_t bAlternate;

	TFnHandleRequest	*pfnHandleCustomReq;
	TSetupPacket Setup;

	void FrameHandler(USBHW *, uint16_t);
	void DevIntHandler(USBHW *, uint8_t bDevStatus);
	void EPIntHandler(USBHW *, uint8_t bEP, uint8_t bEPStatus);
	void SetupHandler(USBHW *, uint8_t bEP, TSetupPacket *Setup);

	uint8_t HandleStdDeviceReq(TSetupPacket *pSetup, int *piLen, uint8_t **ppbData);
	uint8_t HandleStdInterfaceReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);
	uint8_t HandleStdEndPointReq(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);
	uint8_t HandleStandardRequest(TSetupPacket	*pSetup, int *piLen, uint8_t **ppbData);

	uint8_t GetDescriptor(uint16_t wTypeIndex, uint16_t wLangID, int *piLen, uint8_t **ppbData);

	uint8_t SetConfiguration(uint8_t bConfigIndex, uint8_t bAltSetting);

	int GatherConfigurationDescriptor(int);
	void DataIn(void);

	volatile uint16_t lastFrameNumber;

public:
	USBCTRL();
	void init(usbdesc_base **);

	void connect(void);

	uint16_t lastFrame(void);
};

#endif /* _USBCTRL_HPP */
