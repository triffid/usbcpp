#include "USBCDC.hpp"

#include <cstdio>

USBCDC::USBCDC() {
	usbif.bLength				= DL_INTERFACE;
	usbif.bDescType				= DT_INTERFACE;
	usbif.bInterfaceNumber		= 0;
	usbif.bAlternateSetting		= 0;
	usbif.bNumEndPoints			= 1;
	usbif.bInterfaceClass		= UC_COMM;
	usbif.bInterfaceSubClass	= USB_CDC_SUBCLASS_ACM;
	usbif.bInterfaceProtocol	= 1;
	usbif.iInterface			= 0;
	usbif.setupReceiver			= this;

	cdcheader.bLength           = USB_CDC_LENGTH_HEADER;
	cdcheader.bDescType         = DT_CDC_DESCRIPTOR;
	cdcheader.bDescSubType      = USB_CDC_SUBTYPE_HEADER;
	cdcheader.bcdCDC            = 0x0110;

	cmgmt.bLength               = USB_CDC_LENGTH_CALLMGMT;
	cmgmt.bDescType             = DT_CDC_DESCRIPTOR;
	cmgmt.bDescSubType          = USB_CDC_SUBTYPE_CALL_MANAGEMENT;
	cmgmt.bmCapabilities        = USB_CDC_CALLMGMT_CAP_CALLMGMT | USB_CDC_CALLMGMT_CAP_DATAINTF;
	cmgmt.bDataInterface        = 0; // fill me in

	acm.bLength                 = USB_CDC_LENGTH_ACM;
	acm.bDescType               = DT_CDC_DESCRIPTOR;
	acm.bDescSubType            = USB_CDC_SUBTYPE_ACM;
	acm.bmCapabilities          = USB_CDC_ACM_CAP_LINE | USB_CDC_ACM_CAP_BRK;

	cdcunion.bLength               = USB_CDC_LENGTH_UNION;
	cdcunion.bDescType             = DT_CDC_DESCRIPTOR;
	cdcunion.bDescSubType          = USB_CDC_SUBTYPE_UNION;
	cdcunion.bMasterInterface      = 0; // fill me in
	cdcunion.bSlaveInterface0      = 0; // fill me in

	slaveif.bLength					= DL_INTERFACE;
	slaveif.bDescType				= DT_INTERFACE;
	slaveif.bInterfaceNumber		= 0;
	slaveif.bAlternateSetting		= 0;
	slaveif.bNumEndPoints			= 2;
	slaveif.bInterfaceClass			= UC_CDC_DATA;
	slaveif.bInterfaceSubClass		= 0;
	slaveif.bInterfaceProtocol		= 0;
	slaveif.iInterface				= 0;
	slaveif.setupReceiver			= this;

	intep = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_IN,	// .bEndpointAddress
		EA_INTERRUPT,
		8,
		16,
		0,
		this,
	};

	outep = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_OUT,	// .bEndpointAddress
		EA_BULK,	// .bmAttributes
		64,			// .wMaxPacketSize
		0,			// .bInterval
		0,
		this,
	};

	inep = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_IN,	// .bEndpointAddress
		EA_BULK,	// .bmAttributes
		64,			// .wMaxPacketSize
		0,			// .bInterval
		0,
		this,
	};
}

USBCDC::~USBCDC() {}

void USBCDC::attach(USB *usb) {
	IfAddr = usb->addInterface(&usbif);

		EpIntAddr = usb->addEndpoint(&intep);

		usb->addDescriptor(&cmgmt);
		usb->addDescriptor(&cdcheader);
		usb->addDescriptor(&acm);
		usb->addDescriptor(&cdcunion);

	slaveIfAddr = usb->addInterface(&slaveif);

		EpOutAddr = usb->addEndpoint(&outep);
		EpInAddr = usb->addEndpoint(&inep);

	cdcunion.bSlaveInterface0 = slaveIfAddr;
	cdcunion.bMasterInterface = IfAddr;
}

void USBCDC::EPIntHandler(USBHW *u, uint8_t bEP, uint8_t bEPStatus) {
	iprintf("[CDC] Ep %02X: %d\n", bEP, bEPStatus);
	if ((bEP & EP_DIR_MASK) == EP_DIR_IN) {
		// IN: host reads from device
		// dummy: no data
		u->HwEPWrite(bEP, 0, 0);
	}
	else {
		// OUT: host writes to device
		int r = u->HwEPRead(bEP, rxbuffer, sizeof(rxbuffer));
		iprintf("[CDC] Recv %d bytes:\n", r);
		if (r) {
			rxbuffer[r] = 0;
			iprintf("%s\n", rxbuffer);
		}
	}
}

void USBCDC::SetupHandler(USBHW *u, uint8_t bEP, TSetupPacket *Setup) {
	iprintf("[CDC] Setup\n");
	switch(Setup->bRequest) {
		case CDC_SET_LINE_CODING: {
			iprintf("Set Line Encoding\n");

			break;
		};
		case CDC_GET_LINE_CODING: {
			iprintf("Get Line Encoding\n");
			break;
		};
		case CDC_SET_CONTROL_LINE_STATE: {
			iprintf("Set Control Line State\n");

			break;
		};
		case CDC_SEND_BREAK: {
			iprintf("Send Break\n");
			break;
		};
		default: {
			iprintf("UNKNOWN\n");
			break;
		};
	}
}
