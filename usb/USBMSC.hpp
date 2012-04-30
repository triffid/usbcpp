#ifndef	_USBMSC_HPP
#define	_USBMSC_HPP

#include "USB.hpp"
#include "descriptor_msc.h"

class USBMSC {
	usbdesc_interface mscint;
	usbdesc_endpoint epIN;
	usbdesc_endpoint epOUT;
public:
	USBMSC();
	~USBMSC();
	void attach(USB*);
};

#endif /* _USBMSC_HPP */
