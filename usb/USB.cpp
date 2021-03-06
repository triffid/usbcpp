#include "USB.hpp"

#include <cstdio>

// USBCTRL USB::ctrl;
usbdesc_base *USB::descriptors[N_DESCRIPTORS];

usbdesc_device USB::device = {
	DL_DEVICE,
	DT_DEVICE,
	USB_VERSION_1_1,	// .bcdUSB
	UC_PER_INTERFACE,	// .bDeviceClass
	0,					// .bDeviceSubClass
	0,					// .bDeviceProtocol
	64,					// .bMaxPacketSize0
	0x1d50,				// .idVendor
	0x6015,				// .idProduct
	0x0100,				// .bcdDevice
	0,					// .iManufacturer
	0,					// .iProduct
	0,					// .iSerialNumber
	1,					// .bNumConfigurations
};

static usbdesc_language lang = {
	DL_LANGUAGE,
	DT_LANGUAGE,
	{ SL_USENGLISH, },
};

static usbdesc_string_l(10) manufacturer = {
	22,					// .bLength: 2 + 2 * nchars
	DT_STRING,			// .bDescType
	{ 'U', 'S', 'B', ' ', 'M', 'a', 'g', 'i', 'c', '!' }
};

static usbdesc_string_l(23) product = {
	48,					// .bLength: 2 + 2 * nchars
	DT_STRING,			// .bDescType
	{ 'S','m','o','o','t','h','i','e',' ','v','0','.','1',' ','p','r','o','t','o','t','y','p','e' }
};

static usbdesc_string_l(32) serial = {
	66,					// .bLength: 2 + 2 * nchars
	DT_STRING,			// .bDescType
	{	'0', '1', '2', '3', '4', '5', '6', '7',
		'0', '1', '2', '3', '4', '5', '6', '7',
		'0', '1', '2', '3', '4', '5', '6', '7',
		'0', '1', '2', '3', '4', '5', '6', '7',
	}
};

static usbdesc_string_l(7) str_default = {
	16,					// .bLength: 2 + 2 * nchars
	DT_STRING,			// .bDescType
	{ 'D','e','f','a','u','l','t' }
};

usbdesc_configuration USB::conf = {
	DL_CONFIGURATION,
	DT_CONFIGURATION,
	sizeof(usbdesc_configuration),
	0,					// .bNumInterfaces
	1,					// .bConfigurationValue
	0,					// .iConfiguration
	CA_BUSPOWERED,		// .bmAttributes
	500 mA,				// .bMaxPower
};

USB::USB() {
	int i;

	i = 0;
	descriptors[i++] = (usbdesc_base *) &device;
	descriptors[i++] = (usbdesc_base *) &conf;
	for (;i < N_DESCRIPTORS; i++)
		descriptors[i] = (usbdesc_base *) 0;

	addString(&lang);
	device.iManufacturer = addString(&manufacturer);
	device.iProduct = addString(&product);
	device.iSerialNumber = addString(&serial);
	conf.iConfiguration = addString(&str_default);
}

void USB::init() {
	iprintf("init\n");

	uint32_t chip_serial[4];
	HwGetSerialNumber(4, chip_serial);
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 8; i++) {
			uint8_t c = (chip_serial[j] & 15);
			serial.str[j * 8 + 7 - i] = (c < 10)?(c + '0'):(c - 10 + 'A');
			chip_serial[j] >>= 4;
		}
	}

	USBCTRL::init(descriptors);

	iprintf("OK\nconnect\n");
 	connect();
	iprintf("OK");
}

int USB::addDescriptor(usbdesc_base *descriptor) {
	for (int i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0) {
			descriptors[i] = descriptor;
			conf.wTotalLength += descriptor->bLength;
			return i;
		}
	}
	return -1;
}

int USB::addDescriptor(void *descriptor) {
	return addDescriptor((usbdesc_base *) descriptor);
}

int USB::addInterface(usbdesc_interface *ifp) {
	usbdesc_interface *lastif = (usbdesc_interface *) 0;
	uint8_t i;
	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0)
			break;
		if (descriptors[i]->bDescType == DT_INTERFACE)
			lastif = (usbdesc_interface *) descriptors[i];
	}
	if (i >= N_DESCRIPTORS) return -1;

	uint8_t n = conf.bNumInterfaces;

	if (lastif)
		if (ifp->bAlternateSetting != lastif->bAlternateSetting)
			n--;

	ifp->bInterfaceNumber = n;
// 	iprintf("[inserting Interface %p at %d]\n", ifp, i);
	descriptors[i] = (usbdesc_base *) ifp;
	conf.bNumInterfaces = n + 1;
	conf.wTotalLength += descriptors[i]->bLength;

	return n;
}

int USB::getFreeEndpoint() {
	uint8_t i;
	uint8_t lastii = 0;
	usbdesc_interface *lastif = (usbdesc_interface *) 0;

	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0)
			break;
		if (descriptors[i]->bDescType == DT_INTERFACE) {
			lastif = (usbdesc_interface *) descriptors[i];
			lastii = i;
		}
	}
	if (i >= N_DESCRIPTORS) return -1;
	if (lastif == (usbdesc_interface *) 0) return -1;
	if (lastii == 0) return -1;

	uint8_t ep_count = 0;
	for (i = lastii; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0)
			return ep_count;
		if (descriptors[i]->bDescType == DT_ENDPOINT)
			ep_count++;
	}
	return ep_count;
}

int USB::addEndpoint(usbdesc_endpoint *epp) {
// 	iprintf("[EP ");
	uint8_t i;
	usbdesc_interface *lastif = (usbdesc_interface *) 0;
	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0)
			break;
		if (descriptors[i]->bDescType == DT_INTERFACE)
			lastif = (usbdesc_interface *) descriptors[i];
	}
// 	iprintf("%d:%p ", i, lastif);
	if (i >= N_DESCRIPTORS) return -1;
	if (lastif == (usbdesc_interface *) 0) return -1;

	int n = getFreeEndpoint();

// 	iprintf("n:%d ", n);

// 	iprintf("0x%02X:%s ", epp->bEndpointAddress, ((epp->bEndpointAddress & EP_DIR_IN)?"IN":"OUT"));

// 	iprintf("%p ", epp);

	// TODO: assign .bEndpointAddress appropriately
	// we need to scan through our descriptors, and find the first unused logical endpoint of the appropriate type
	// the LPC17xx has 16 logical endpoints mapped to 32 "physical" endpoints, looks like this means we have 16 endpoints to use and we get to pick direction
	// SO, let's first find all the endpoints in our list of the same type, pick the highest address then go find the next one

	// pick a starting logical endpoint- interrupt = 1, bulk = 2, iso = 3 then subtract 3 so we pick the first one later on
	int lepaddr = (4 - epp->bmAttributes) - 3;

	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0)
			break;
		if (descriptors[i]->bDescType == DT_ENDPOINT) {
			usbdesc_endpoint *x = (usbdesc_endpoint *) descriptors[i];
			if (x->bmAttributes == epp->bmAttributes && ((x->bEndpointAddress & 0x80) == (epp->bEndpointAddress & 0x80)))
				if ((x->bEndpointAddress & 0x0F) > lepaddr)
					lepaddr = (x->bEndpointAddress & 0x0F);
		}
	}

// 	iprintf("lep:%d ", lepaddr);

	// now, lepaddr is the last logical endpoint of appropriate type
	// the endpoints go in groups of 3, except for the last one which is a bulk instead of isochronous
	// find the next free lep using this simple pattern
	if (epp->bmAttributes == EA_BULK && lepaddr == 14)
		lepaddr = 15;
	else
		lepaddr += 3;
	// now we have the next free logical endpoint of the appropriate type

	// if it's >15 we've run out, spit an error
	if (lepaddr > 15) return -1;

// 	iprintf("pep:%d ", lepaddr);

	// store logical address and direction bit as physical address, this is how the LPC17xx has its USB set up
	epp->bEndpointAddress = lepaddr | (epp->bEndpointAddress & 0x80);

// 	iprintf("eaddr:0x%02X ", epp->bEndpointAddress);

	descriptors[i] = (usbdesc_base *) epp;
	// 	lastif->bNumEndPoints = n + 1;

// 	iprintf(" EP complete]\n");

	conf.wTotalLength += descriptors[i]->bLength;

	return n;
}

int USB::addString(void *ss) {
	usbdesc_base *s = (usbdesc_base *) ss;
// 	iprintf("[AST %p ", s);
	if (s->bDescType == DT_STRING) {
// 		iprintf("LEN:%d ", s->bLength);

		uint8_t i;
		uint8_t stringcount = 0;
		for (i = 0; i < N_DESCRIPTORS; i++) {
			if (descriptors[i] == (usbdesc_base *) 0)
				break;
			if (descriptors[i]->bDescType == DT_STRING)
				stringcount++;
		}
		if (i >= N_DESCRIPTORS) return -1;

// 		iprintf("INS %d ", i);

		descriptors[i] = s;

		conf.wTotalLength += descriptors[i]->bLength;

// 		iprintf("%p STROK]", descriptors[i]);

		return stringcount;
	}
// 	iprintf("INVAL]\n");
	return -1;
}

int USB::findStringIndex(uint8_t strid) {
	uint8_t i;
	uint8_t strcounter = 0;
	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0)
			return -1;
		if (descriptors[i]->bDescType == DT_STRING) {
			if (strcounter == strid)
				return i;
			strcounter++;
		}
	}
	return -1;
}

void USB::dumpDevice(usbdesc_device *d) {
	iprintf("Device:\n");
	iprintf("\tUSB Version:  %d.%d\n", d->bcdUSB >> 8, d->bcdUSB & 0xFF);
	iprintf("\tClass:        0x%04X\n", d->bDeviceClass);
	iprintf("\tSubClass:     0x%04X\n", d->bDeviceSubClass);
	iprintf("\tProtocol:     0x%04X\n", d->bDeviceProtocol);
	iprintf("\tMax Packet:   %d\n", d->bMaxPacketSize);
	iprintf("\tVendor:       0x%04X\n", d->idVendor);
	iprintf("\tProduct:      0x%04X\n", d->idProduct);
	iprintf("\tManufacturer: "); dumpString(d->iManufacturer);
	iprintf("\tProduct:      "); dumpString(d->iProduct);
	iprintf("\tSerial:       "); dumpString(d->iSerialNumber);
	iprintf("\tNum Configs:  %d\n", d->bNumConfigurations);
}

void USB::dumpConfiguration(usbdesc_configuration *c) {
	iprintf("Configuration:\n");
	iprintf("\tTotal Length:        %db\n", c->wTotalLength);
	iprintf("\tNum Interfaces:      %d\n", c->bNumInterfaces);
	iprintf("\tConfiguration Value: %d\n", c->bConfigurationValue);
	iprintf("\tConfiguration:       "); dumpString(c->iConfiguration);
	iprintf("\tAttributes:          %s\n", ((c->bmAttributes & 0x80)?"Bus Powered":"Self Powered"));
	iprintf("\tMax Power:           %dmA\n", c->bMaxPower * 2);
}

void USB::dumpInterface(usbdesc_interface *i) {
	iprintf("\t*Interface\n");
	iprintf("\t\tNumber:        %d\n", i->bInterfaceNumber);
	iprintf("\t\tAlternate:     %d\n", i->bAlternateSetting);
	iprintf("\t\tNum Endpoints: %d\n", i->bNumEndPoints);
	iprintf("\t\tClass:         0x%02X ", i->bInterfaceClass);
	switch(i->bInterfaceClass) {
		case UC_COMM:
			iprintf("(COMM)");
			break;
		case UC_MASS_STORAGE:
			iprintf("(MSC)");
			break;
		case UC_CDC_DATA:
			iprintf("(CDC DATA)");
			break;
	}
	iprintf("\n");
	iprintf("\t\tSubClass:      0x%02X ", i->bInterfaceSubClass);
	switch(i->bInterfaceClass) {
		case UC_COMM: {
			switch(i->bInterfaceSubClass) {
				case USB_CDC_SUBCLASS_ACM:
					iprintf("(ACM)");
					break;
				case USB_CDC_SUBCLASS_ETHERNET:
					iprintf("(ETHERNET)");
					break;
			}
			break;
		}
		case UC_MASS_STORAGE: {
			switch(i->bInterfaceSubClass) {
				case MSC_SUBCLASS_SCSI:
					iprintf("(SCSI)");
					break;
			}
			break;
		}
	}
	iprintf("\n");
	iprintf("\t\tProtocol:      0x%02X ", i->bInterfaceProtocol);
	iprintf("\n");
	iprintf("\t\tInterface:     "); dumpString(i->iInterface);
}
void USB::dumpEndpoint(usbdesc_endpoint *e) {
	static const char* const attr[4] = { "", "Isochronous", "Bulk", "Interrupt" };
	iprintf("\t\t*Endpoint\n");
	iprintf("\t\t\tAddress:    0x%02X (%s)\n", e->bEndpointAddress, ((e->bEndpointAddress & EP_DIR_IN)?"IN":"OUT"));
	iprintf("\t\t\tAttributes: 0x%02X (%s)\n", e->bmAttributes, attr[e->bmAttributes]);
	iprintf("\t\t\tMax Packet: %d\n", e->wMaxPacketSize);
	iprintf("\t\t\tInterval:   %d\n", e->bInterval);
}

void USB::dumpString(int i) {
	if (i > 0) {
		uint8_t j = findStringIndex(i);
		if (j > 0) {
			iprintf("[%d] ", i);
			dumpString((usbdesc_string *) descriptors[j]);
			return;
		}
	}
	iprintf("-none-\n");
}

void USB::dumpString(usbdesc_string *s) {
	uint8_t i;
	for (i = 0; i < (s->bLength - 2) / 2; i++) {
		if (s->str[i] >= 32 && s->str[i] < 128)
			putchar(s->str[i]);
		else
			iprintf("\\0x%02X", s->str[i]);
	}
	putchar('\n');
}

void USB::dumpCDC(uint8_t *d) {
	switch(d[2]) {
		case USB_CDC_SUBTYPE_HEADER: {
			usbcdc_header *h = (usbcdc_header *) d;
			iprintf("\t\t*CDC header\n");
			iprintf("\t\t\tbcdCDC:  0x%04X\n", h->bcdCDC);
			break;
		}
		case USB_CDC_SUBTYPE_UNION: {
			usbcdc_union *u = (usbcdc_union *) d;
			iprintf("\t\t*CDC union\n");
			iprintf("\t\t\tMaster:  %d\n", u->bMasterInterface);
			iprintf("\t\t\tSlave:   %d\n", u->bSlaveInterface0);
			break;
		}
		case USB_CDC_SUBTYPE_CALL_MANAGEMENT: {
			usbcdc_callmgmt *m = (usbcdc_callmgmt *) d;
			iprintf("\t\t*CDC Call Management\n");
			iprintf("\t\t\tCapabilities:  0x%02X ", m->bmCapabilities);
			if (m->bmCapabilities & USB_CDC_CALLMGMT_CAP_CALLMGMT)
				iprintf("(CALLMGMT)");
			if (m->bmCapabilities & USB_CDC_CALLMGMT_CAP_DATAINTF)
				iprintf("(DATAINTF)");
			iprintf("\n");
			iprintf("\t\t\tData Interface: %d\n", m->bDataInterface);
			break;
		}
		case USB_CDC_SUBTYPE_ACM: {
			usbcdc_acm *a = (usbcdc_acm *) d;
			iprintf("\t\t*CDC ACM\n");
			iprintf("\t\t\tCapabilities: 0x%02X ", a->bmCapabilities);
			if (a->bmCapabilities & USB_CDC_ACM_CAP_COMM)
				iprintf("(COMM)");
			if (a->bmCapabilities & USB_CDC_ACM_CAP_LINE)
				iprintf("(LINE)");
			if (a->bmCapabilities & USB_CDC_ACM_CAP_BRK)
				iprintf("(BRK)");
			if (a->bmCapabilities & USB_CDC_ACM_CAP_NOTIFY)
				iprintf("(NOTIFY)");
			iprintf("\n");
			break;
		}
		case USB_CDC_SUBTYPE_ETHERNET: {
			usbcdc_ether *e = (usbcdc_ether *) d;
			iprintf("\t\t*CDC Ethernet\n");
			iprintf("\t\t\tMAC address: "); dumpString(e->iMacAddress);
			iprintf("\t\t\tStatistics: 0x%02lX\n", e->bmEthernetStatistics);
			iprintf("\t\t\tMax Segment Size: %d\n", e->wMaxSegmentSize);
			iprintf("\t\t\tMC Filters: %d\n", e->wNumberMCFilters);
			iprintf("\t\t\tPower Filters: %d\n", e->bNumberPowerFilters);
			break;
		}
	}
}

void USB::dumpDescriptors() {
	uint8_t i;
	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0) {
			iprintf("--- FIN at %d\n", i);
			return;
		}
		iprintf("[%d:+%d]", i, descriptors[i]->bLength);
		switch (descriptors[i]->bDescType) {
			case DT_DEVICE: {
				dumpDevice((usbdesc_device *) descriptors[i]);
				break;
			}
			case DT_CONFIGURATION: {
				dumpConfiguration((usbdesc_configuration *) descriptors[i]);
				break;
			}
			case DT_INTERFACE: {
				dumpInterface((usbdesc_interface *) descriptors[i]);
				break;
			}
			case DT_ENDPOINT: {
				dumpEndpoint((usbdesc_endpoint *) descriptors[i]);
				break;
			}
			case DT_STRING: {
				dumpString((usbdesc_string *) descriptors[i]);
				break;
			}
			case DT_CDC_DESCRIPTOR: {
				dumpCDC((uint8_t *) descriptors[i]);
				break;
			}
		}
	}
}
