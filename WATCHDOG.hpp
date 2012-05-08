#ifndef _WATCHDOG_HPP
#define	_WATCHDOG_HPP

#include "lpc17xx_wdt.h"

#define WATCHDOG_uS			* 1
#define WATCHDOG_mS			* 1000
#define	WATCHDOG_S			* 1000000
#define	WATCHDOG_SECONDS	* 1000000

class WATCHDOG {
public:
	WATCHDOG(uint32_t timeout_us);
	void feed(void);
};

#endif /* _WATCHDOG_HPP */
