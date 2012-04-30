#include "CLOCK.hpp"

volatile uint32_t clock::ticks;

void clock::irq() {
	ticks++;
}

extern "C" {
	void SysTick_Handler() {
		clock::irq();
	}
}

clock::clock() {
	ticks = 0;
	SYSTICK_InternalInit(10);
	SYSTICK_Cmd(ENABLE);
	SYSTICK_IntCmd(ENABLE);
}
clock::~clock() {
	SYSTICK_Cmd(DISABLE);
	SYSTICK_IntCmd(DISABLE);
}

uint32_t clock::time() {
	return clock::ticks;
}

void clock::wait(uint32_t time) {
	time += ticks;
	while ((time - ticks) < 0x80000000);
}

void clock::setTimeout(uint32_t time) {
	timeout = time;
	nexttime = time + ticks;
}

uint8_t clock::poll() {
	if ((nexttime - ticks) >= 0x8000000) {
		nexttime += timeout;
		return 1;
	}
	return 0;
}
