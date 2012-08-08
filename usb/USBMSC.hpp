#ifndef	_USBMSC_HPP
#define	_USBMSC_HPP

#include "USB.hpp"
#include "descriptor_msc.h"

class USBMSC : public USB_EP_Receiver {
	usbdesc_interface mscint;
	usbdesc_endpoint epIN;
	usbdesc_endpoint epOUT;

	void EPIntHandler(uint8_t, uint8_t);

public:
	USBMSC();
	~USBMSC();
	void attach(USB*);
};

#endif /* _USBMSC_HPP */
