#ifndef	_USBCDC_HPP
#define	_USBCDC_HPP

#include "USB.hpp"

class USBCDC : public USB_EP_Receiver {
	usbdesc_interface   usbif;
	usbcdc_header       cdcheader;
	usbcdc_callmgmt     cmgmt;
	usbcdc_acm          acm;
	usbcdc_union        cdcunion;

	usbdesc_interface   slaveif;

	usbdesc_endpoint	intep;
	usbdesc_endpoint	outep;
	usbdesc_endpoint	inep;

	uint8_t IfAddr;
	uint8_t EpIntAddr;
	uint8_t slaveIfAddr;
	uint8_t EpOutAddr;
	uint8_t EpInAddr;

	int EpCallback(uint8_t, uint8_t);

public:
	USBCDC();
	~USBCDC();

	void attach(USB *usb);
};

#endif /* _USBCDC_HPP */
