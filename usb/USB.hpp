#ifndef _USB_HPP
#define _USB_HPP

#include "descriptor.h"

#ifndef N_DESCRIPTORS
#define N_DESCRIPTORS 32
#endif

class USB;
class USBHW;
class USBCTRL;

#include "USBHW.hpp"
#include "USBCTRL.hpp"

typedef void (*epCallback_f)(uint8_t, uint8_t);

class USB {
	friend class USBHW;
 	friend class USBCTRL;

	usbdesc_base *descriptors[N_DESCRIPTORS];

	static usbdesc_device device;
	static usbdesc_configuration conf;
public:
	USB();
	void init(void);

	static void IRQ(void);

	static usbdesc_configuration *getConf(void);

	int addDescriptor(usbdesc_base *descriptor);
	int addDescriptor(void *descriptor);
	int addInterface(usbdesc_interface *);
	int addEndpoint(usbdesc_endpoint *);
	int addString(void *);
	int getFreeEndpoint();
	int findStringIndex(uint8_t strid);

	epCallback_f EpCallback(uint8_t);
	void EpCallback(uint8_t, epCallback_f);

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
