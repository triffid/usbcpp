#ifndef	_USBMSC_HPP
#define	_USBMSC_HPP

#include "USB.hpp"
#include "descriptor_msc.h"

class USBMSC : public USB_EP_Receiver, public USB_Setup_Receiver {
	usbdesc_interface mscint;
	usbdesc_endpoint epIN;
	usbdesc_endpoint epOUT;

	void EPIntHandler(USBHW *, uint8_t, uint8_t);
	void SetupHandler(USBHW *, uint8_t, TSetupPacket *);

public:
	USBMSC();
	~USBMSC();
	void attach(USB*);
};

#endif /* _USBMSC_HPP */
