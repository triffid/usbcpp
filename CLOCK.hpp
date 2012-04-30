#ifndef	_CLOCK_HPP
#define	_CLOCK_HPP

#include "LPC17xx.h"
#include "lpc17xx_systick.h"

#include <stdint.h>

#define MS /10
#define S  * 100

class clock {
	static volatile uint32_t ticks;
	uint32_t timeout;
	uint32_t nexttime;
public:
	static void irq(void);
	clock();
	~clock();
	uint32_t time();
	static void wait(uint32_t time);
	void setTimeout(uint32_t time);
	uint8_t poll();
};

#endif /* _CLOCK_HPP */
