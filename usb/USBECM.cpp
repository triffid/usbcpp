#include "USBECM.hpp"

#include <cstdio>

USBECM::USBECM() {
	if0 = {
		DL_INTERFACE,
		DT_INTERFACE,
		0,
		0,
		1,
		UC_COMM,
		USB_CDC_SUBCLASS_ETHERNET,
		0,
		0,
	};
	cdcheader = {
		USB_CDC_LENGTH_HEADER,
		DT_CDC_DESCRIPTOR,
		USB_CDC_SUBTYPE_HEADER,
		0x0110,
	};
	cdcunion = {
		USB_CDC_LENGTH_UNION,
		DT_CDC_DESCRIPTOR,
		USB_CDC_SUBTYPE_UNION,
		0,
		0,
	};
	cdcether = {
		USB_CDC_LENGTH_ETHER,
		DT_CDC_DESCRIPTOR,
		USB_CDC_SUBTYPE_ETHERNET,
		0,
		0,
		1514,
		0,
		0,
	};
	notifyEP = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_IN,
		EA_INTERRUPT,
		8,
		10,
		0,
		this,
	};
	ifnop = {
		DL_INTERFACE,
		DT_INTERFACE,
		0,
		0,
		0,
		UC_CDC_DATA,
		0,
		0,
		0,
	};
	ifdata = {
		DL_INTERFACE,
		DT_INTERFACE,
		0,
		1,
		2,
		UC_CDC_DATA,
		0,
		0,
		0,
	};
	OutEP = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_OUT,
		EA_BULK,
		64,
		0,
		0,
		this,
	};
	InEP = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_IN,
		EA_BULK,
		64,
		0,
		0,
		this,
	};
	macaddr.bLength = 26;
	macaddr.bDescType = DT_STRING;
	macaddr.str = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
}

USBECM::~USBECM() {}

void USBECM::setMAC(uint8_t mac[6]) {
	for (int i = 0; i < 12; i++) {
		uint8_t c = (mac[i / 2] >> ((i & 1)?0:4)) & 0x0F;
		if (c >= 10)
			c += 'A' - 10;
		else
			c += '0';
		macaddr.str[i] = c;
	}
}

void USBECM::attach(USB *u) {
	int r;
	myusb = u;

	r = u->addInterface(&if0);
		iprintf("ECM:if0 = %d\n", r);

	cdcunion.bMasterInterface = r;

	r = u->addDescriptor(&cdcheader);
		iprintf("ECM:head = %d\n", r);
	r = u->addDescriptor(&cdcunion);
		iprintf("ECM:union = %d\n", r);
	r = u->addDescriptor(&cdcether);
		iprintf("ECM:ether = %d\n", r);
	r = u->addEndpoint(&notifyEP);
		iprintf("ECM:notifyEP = %d\n", r);
	r = u->addInterface(&ifnop);
		iprintf("ECM:ifnop = %d\n", r);
	r = u->addInterface(&ifdata);
		iprintf("ECM:ifdata = %d\n", r);

	cdcunion.bSlaveInterface0 = r;

	r = u->addEndpoint(&OutEP);
		iprintf("ECM:outep = %d\n", r);
	r = u->addEndpoint(&InEP);
		iprintf("ECM:inep = %d\n", r);
	r = u->addString(&macaddr);
		iprintf("ECM:macstr = %d\n", r);

	cdcether.iMacAddress = r;
}

void USBECM::EPIntHandler(uint8_t bEP, uint8_t bEPStatus) {
	iprintf("[ECM] Ep %02X: %d\n", bEP, bEPStatus);
}
