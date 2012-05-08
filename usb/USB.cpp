#include "USB.hpp"

#include <cstdio>

USBCTRL USB::ctrl;
usbdesc_base *USB::descriptors[N_DESCRIPTORS];

usbdesc_device USB::device = {
	DL_DEVICE,
	DT_DEVICE,
	USB_VERSION_1_1,	// .bcdUSB
	UC_PER_INTERFACE,	// .bDeviceClass
	0,					// .bDeviceSubClass
	0,					// .bDeviceProtocol
	8,					// .bMaxPacketSize0
	0xFFFF,				// .idVendor
	0x0005,				// .idProduct
	0x0100,				// .bcdDevice
	1,					// .iManufacturer
	2,					// .iProduct
	3,					// .iSerialNumber
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

static usbdesc_string_l(4) product = {
	10,					// .bLength: 2 + 2 * nchars
	DT_STRING,			// .bDescType
	{ 'R','2','C','2' }
};

static usbdesc_string_l(6) serial = {
	14,					// .bLength: 2 + 2 * nchars
	DT_STRING,			// .bDescType
	{ 'A', '1', 'X', 'T', 'Q', 'Z' }
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
	addString(&manufacturer);
	addString(&product);
	addString(&serial);
}

void USB::init() {
	printf("ctrl.init\n");
	ctrl.init(descriptors);

	printf("OK\nctrl.connect\n");
 	ctrl.connect();
	printf("OK");
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
	printf("[inserting Interface %p at %d]\n", ifp, i);
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
	printf("[EP ");
	uint8_t i;
	usbdesc_interface *lastif = (usbdesc_interface *) 0;
	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0)
			break;
		if (descriptors[i]->bDescType == DT_INTERFACE)
			lastif = (usbdesc_interface *) descriptors[i];
	}
	printf("%d:%p ", i, lastif);
	if (i >= N_DESCRIPTORS) return -1;
	if (lastif == (usbdesc_interface *) 0) return -1;

	int n = getFreeEndpoint();

	printf("n:%d ", n);

	printf("0x%02X:%s ", epp->bEndpointAddress, ((epp->bEndpointAddress & EP_DIR_IN)?"IN":"OUT"));

	printf("%p ", epp);

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

	printf("lep:%d ", lepaddr);

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

	printf("pep:%d ", lepaddr);

	// store logical address and direction bit as physical address, this is how the LPC17xx has its USB set up
	epp->bEndpointAddress = lepaddr | (epp->bEndpointAddress & 0x80);

	printf("eaddr:0x%02X ", epp->bEndpointAddress);

	descriptors[i] = (usbdesc_base *) epp;
	// 	lastif->bNumEndPoints = n + 1;

	printf(" EP complete]\n");

	conf.wTotalLength += descriptors[i]->bLength;

	return n;
}

int USB::addString(void *ss) {
	usbdesc_base *s = (usbdesc_base *) ss;
	printf("[AST %p ", s);
	if (s->bDescType == DT_STRING) {
		printf("LEN:%d ", s->bLength);

		uint8_t i;
		uint8_t stringcount = 0;
		for (i = 0; i < N_DESCRIPTORS; i++) {
			if (descriptors[i] == (usbdesc_base *) 0)
				break;
			if (descriptors[i]->bDescType == DT_STRING)
				stringcount++;
		}
		if (i >= N_DESCRIPTORS) return -1;

		printf("INS %d ", i);

		descriptors[i] = s;

		conf.wTotalLength += descriptors[i]->bLength;

		printf("%p STROK]", descriptors[i]);

		return stringcount;
	}
	printf("INVAL]\n");
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
	printf("Device:\n");
	printf("\tUSB Version:  %d.%d\n", d->bcdUSB >> 8, d->bcdUSB & 0xFF);
	printf("\tClass:        0x%04X\n", d->bDeviceClass);
	printf("\tSubClass:     0x%04X\n", d->bDeviceSubClass);
	printf("\tProtocol:     0x%04X\n", d->bDeviceProtocol);
	printf("\tMax Packet:   %d\n", d->bMaxPacketSize);
	printf("\tVendor:       0x%04X\n", d->idVendor);
	printf("\tProduct:      0x%04X\n", d->idProduct);
	printf("\tManufacturer: "); dumpString(d->iManufacturer);
	printf("\tProduct:      "); dumpString(d->iProduct);
	printf("\tSerial:       "); dumpString(d->iSerialNumber);
	printf("\tNum Configs:  %d\n", d->bNumConfigurations);
}

void USB::dumpConfiguration(usbdesc_configuration *c) {
	printf("Configuration:\n");
	printf("\tTotal Length:        %db\n", c->wTotalLength);
	printf("\tNum Interfaces:      %d\n", c->bNumInterfaces);
	printf("\tConfiguration Value: %d\n", c->bConfigurationValue);
	printf("\tConfiguration:       "); dumpString(c->iConfiguration);
	printf("\tAttributes:          %s\n", ((c->bmAttributes & 0x80)?"Bus Powered":"Self Powered"));
	printf("\tMax Power:           %dmA\n", c->bMaxPower * 2);
}

void USB::dumpInterface(usbdesc_interface *i) {
	printf("\t*Interface\n");
	printf("\t\tNumber:        %d\n", i->bInterfaceNumber);
	printf("\t\tAlternate:     %d\n", i->bAlternateSetting);
	printf("\t\tNum Endpoints: %d\n", i->bNumEndPoints);
	printf("\t\tClass:         0x%02X ", i->bInterfaceClass);
	switch(i->bInterfaceClass) {
		case UC_COMM:
			printf("(COMM)");
			break;
		case UC_MASS_STORAGE:
			printf("(MSC)");
			break;
		case UC_CDC_DATA:
			printf("(CDC DATA)");
			break;
	}
	printf("\n");
	printf("\t\tSubClass:      0x%02X ", i->bInterfaceSubClass);
	switch(i->bInterfaceClass) {
		case UC_COMM: {
			switch(i->bInterfaceSubClass) {
				case USB_CDC_SUBCLASS_ACM:
					printf("(ACM)");
					break;
				case USB_CDC_SUBCLASS_ETHERNET:
					printf("(ETHERNET)");
					break;
			}
			break;
		}
		case UC_MASS_STORAGE: {
			switch(i->bInterfaceSubClass) {
				case MSC_SUBCLASS_SCSI:
					printf("(SCSI)");
					break;
			}
			break;
		}
	}
	printf("\n");
	printf("\t\tProtocol:      0x%02X ", i->bInterfaceProtocol);
	printf("\n");
	printf("\t\tInterface:     "); dumpString(i->iInterface);
}
void USB::dumpEndpoint(usbdesc_endpoint *e) {
	static const char* const attr[4] = { "", "Isochronous", "Bulk", "Interrupt" };
	printf("\t\t*Endpoint\n");
	printf("\t\t\tAddress:    0x%02X (%s)\n", e->bEndpointAddress, ((e->bEndpointAddress & EP_DIR_IN)?"IN":"OUT"));
	printf("\t\t\tAttributes: 0x%02X (%s)\n", e->bmAttributes, attr[e->bmAttributes]);
	printf("\t\t\tMax Packet: %d\n", e->wMaxPacketSize);
	printf("\t\t\tInterval:   %d\n", e->bInterval);
}

void USB::dumpString(int i) {
	if (i > 0) {
		uint8_t j = findStringIndex(i);
		if (j > 0) {
			printf("[%d] ", i);
			dumpString((usbdesc_string *) descriptors[j]);
			return;
		}
	}
	printf("-none-\n");
}

void USB::dumpString(usbdesc_string *s) {
	uint8_t i;
	for (i = 0; i < (s->bLength - 2) / 2; i++) {
		if (s->str[i] >= 32 && s->str[i] < 128)
			putchar(s->str[i]);
		else
			printf("\\0x%02X", s->str[i]);
	}
	putchar('\n');
}

void USB::dumpCDC(uint8_t *d) {
	switch(d[2]) {
		case USB_CDC_SUBTYPE_HEADER: {
			usbcdc_header *h = (usbcdc_header *) d;
			printf("\t\t*CDC header\n");
			printf("\t\t\tbcdCDC:  0x%04X\n", h->bcdCDC);
			break;
		}
		case USB_CDC_SUBTYPE_UNION: {
			usbcdc_union *u = (usbcdc_union *) d;
			printf("\t\t*CDC union\n");
			printf("\t\t\tMaster:  %d\n", u->bMasterInterface);
			printf("\t\t\tSlave:   %d\n", u->bSlaveInterface0);
			break;
		}
		case USB_CDC_SUBTYPE_CALL_MANAGEMENT: {
			usbcdc_callmgmt *m = (usbcdc_callmgmt *) d;
			printf("\t\t*CDC Call Management\n");
			printf("\t\t\tCapabilities:  0x%02X ", m->bmCapabilities);
			if (m->bmCapabilities & USB_CDC_CALLMGMT_CAP_CALLMGMT)
				printf("(CALLMGMT)");
			if (m->bmCapabilities & USB_CDC_CALLMGMT_CAP_DATAINTF)
				printf("(DATAINTF)");
			printf("\n");
			printf("\t\t\tData Interface: %d\n", m->bDataInterface);
			break;
		}
		case USB_CDC_SUBTYPE_ACM: {
			usbcdc_acm *a = (usbcdc_acm *) d;
			printf("\t\t*CDC ACM\n");
			printf("\t\t\tCapabilities: 0x%02X ", a->bmCapabilities);
			if (a->bmCapabilities & USB_CDC_ACM_CAP_COMM)
				printf("(COMM)");
			if (a->bmCapabilities & USB_CDC_ACM_CAP_LINE)
				printf("(LINE)");
			if (a->bmCapabilities & USB_CDC_ACM_CAP_BRK)
				printf("(BRK)");
			if (a->bmCapabilities & USB_CDC_ACM_CAP_NOTIFY)
				printf("(NOTIFY)");
			printf("\n");
			break;
		}
		case USB_CDC_SUBTYPE_ETHERNET: {
			usbcdc_ether *e = (usbcdc_ether *) d;
			printf("\t\t*CDC Ethernet\n");
			printf("\t\t\tMAC address: "); dumpString(e->iMacAddress);
			printf("\t\t\tStatistics: 0x%02lX\n", e->bmEthernetStatistics);
			printf("\t\t\tMax Segment Size: %d\n", e->wMaxSegmentSize);
			printf("\t\t\tMC Filters: %d\n", e->wNumberMCFilters);
			printf("\t\t\tPower Filters: %d\n", e->bNumberPowerFilters);
			break;
		}
	}
}

void USB::dumpDescriptors() {
	uint8_t i;
	for (i = 0; i < N_DESCRIPTORS; i++) {
		if (descriptors[i] == (usbdesc_base *) 0) {
			printf("--- FIN at %d\n", i);
			return;
		}
		printf("[%d:+%d]", i, descriptors[i]->bLength);
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
