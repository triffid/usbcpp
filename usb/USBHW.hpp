#ifndef _USBHW_HPP
#define _USBHW_HPP

class USBHW {
public:
	USBHW();
	void init(void);
	void connect(void);
	void disconnect(void);
};

#endif /* _USBHW_HPP */
