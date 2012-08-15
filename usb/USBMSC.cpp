#include "USBMSC.hpp"

#include <cstdio>

USBMSC::USBMSC() {
	mscint.bLength				= DL_INTERFACE;
	mscint.bDescType			= DT_INTERFACE;
	mscint.bInterfaceNumber		= 0;
	mscint.bAlternateSetting	= 0;
	mscint.bNumEndPoints		= 2;
	mscint.bInterfaceClass		= UC_MASS_STORAGE;
	mscint.bInterfaceSubClass	= MSC_SUBCLASS_SCSI;
	mscint.bInterfaceProtocol	= MSC_PROTOCOL_BULK_ONLY;
	mscint.iInterface			= 0;
	mscint.setupReceiver		= this;

	epIN = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_IN,
		EA_BULK,
		64,
		0,
		0,
		this,
	};

	epOUT = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_OUT,
		EA_BULK,
		64,
		0,
		0,
		this,
	};
}

USBMSC::~USBMSC() {}

void USBMSC::attach(USB *u) {
	u->addInterface(&mscint);
	u->addEndpoint(&epOUT);
	u->addEndpoint(&epIN);
}

void USBMSC::EPIntHandler(USBHW *u, uint8_t bEP, uint8_t bEPStatus) {
	iprintf("[MSC] Ep %02X: %d\n", bEP, bEPStatus);
}

void USBMSC::SetupHandler(USBHW *u, uint8_t bEP, TSetupPacket *Setup) {
	iprintf("[MSC] Setup\n");
}
