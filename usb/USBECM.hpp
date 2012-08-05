#ifndef _USBECM_HPP
#define _USBECM_HPP

#include "USB.hpp"

typedef usbdesc_string_l(12) macstr_t;

class USBECM : public USB_EP_Receiver {
	USB *myusb;

	usbdesc_interface		if0;
	usbcdc_header			cdcheader;
	usbcdc_union			cdcunion;
	usbcdc_ether			cdcether;
	usbdesc_endpoint		notifyEP;
	usbdesc_interface		ifnop;
	usbdesc_interface		ifdata;
	usbdesc_endpoint		OutEP;
	usbdesc_endpoint		InEP;
	macstr_t				macaddr;

	int EpCallback(uint8_t, uint8_t);

public:
	USBECM();
	~USBECM();
	void setMAC(uint8_t mac[6]);
	void attach(USB *);
};

#endif /* _USBECM_HPP */
