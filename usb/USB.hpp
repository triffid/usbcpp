#ifndef _USB_HPP
#define _USB_HPP

#include "descriptor.h"

#ifndef N_DESCRIPTORS
#define N_DESCRIPTORS 32
#endif

// typedef void (*epCallback_f)(uint8_t, uint8_t);

#include "USBCTRL.hpp"

class USBCTRL;

class USB : public USBCTRL {
// 	static USBCTRL ctrl;
	static usbdesc_base *descriptors[N_DESCRIPTORS];

	static usbdesc_device device;
	static usbdesc_configuration conf;
public:
	USB();
	void init(void);

	int addDescriptor(usbdesc_base *descriptor);
	int addDescriptor(void *descriptor);
	int addInterface(usbdesc_interface *);
	int addEndpoint(usbdesc_endpoint *);
	int addString(void *);
	int getFreeEndpoint();
	int findStringIndex(uint8_t strid);

// 	epCallback_f EpCallback(uint8_t);
// 	void EpCallback(uint8_t, epCallback_f);
// 	void setEpCallback(uint8_t, USB_EP_Receiver *);

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
