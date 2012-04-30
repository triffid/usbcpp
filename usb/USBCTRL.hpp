#ifndef	_USBCTRL_HPP
#define _USBCTRL_HPP

#include "USBHW.hpp"

class USBCTRL {
	USBHW *hw;
public:
	USBCTRL();
	void init(USBHW *);

};

#endif /* _USBCTRL_HPP */
