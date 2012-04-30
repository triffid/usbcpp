#ifndef _USB_HPP
#define _USB_HPP

#include "descriptor.h"

#ifndef N_DESCRIPTORS
#define N_DESCRIPTORS 32
#endif

#include "USBHW.hpp"
#include "USBCTRL.hpp"

class USB {
	USBHW hw;
	USBCTRL ctrl;

	usbdesc_base *descriptors[N_DESCRIPTORS];

	static usbdesc_device device;
	static usbdesc_configuration conf;
public:
	USB();
	void init(void);

	static void IRQ(void);

	int addDescriptor(usbdesc_base *descriptor);
	int addDescriptor(void *descriptor);

	int addInterface(usbdesc_interface *);

	int addEndpoint(usbdesc_endpoint *);

	int addString(void *);

	int getFreeEndpoint();

	int findStringIndex(uint8_t strid);

	void dumpDescriptors();
	void dumpDevice(usbdesc_device *);
	void dumpConfiguration(usbdesc_configuration *);
	void dumpInterface(usbdesc_interface *);
	void dumpEndpoint(usbdesc_endpoint *);
	void dumpString(int i);
	void dumpString(usbdesc_string *);
	void dumpCDC(uint8_t *);
};

#endif /* _USB_HPP */
