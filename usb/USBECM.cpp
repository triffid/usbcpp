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
	};
	InEP = {
		DL_ENDPOINT,
		DT_ENDPOINT,
		EP_DIR_IN,
		EA_BULK,
		64,
		0,
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
		printf("ECM:if0 = %d\n", r);

	cdcunion.bMasterInterface = r;

	r = u->addDescriptor(&cdcheader);
		printf("ECM:head = %d\n", r);
	r = u->addDescriptor(&cdcunion);
		printf("ECM:union = %d\n", r);
	r = u->addDescriptor(&cdcether);
		printf("ECM:ether = %d\n", r);
	r = u->addEndpoint(&notifyEP);
		printf("ECM:notifyEP = %d\n", r);
	r = u->addInterface(&ifnop);
		printf("ECM:ifnop = %d\n", r);
	r = u->addInterface(&ifdata);
		printf("ECM:ifdata = %d\n", r);

	cdcunion.bSlaveInterface0 = r;

	r = u->addEndpoint(&OutEP);
		printf("ECM:outep = %d\n", r);
	r = u->addEndpoint(&InEP);
		printf("ECM:inep = %d\n", r);
	r = u->addString(&macaddr);
		printf("ECM:macstr = %d\n", r);

	cdcether.iMacAddress = r;
}
