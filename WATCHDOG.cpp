#include "WATCHDOG.hpp"

WATCHDOG::WATCHDOG(uint32_t timeout_us) {
	WDT_ClrTimeOutFlag();
	WDT_Init(WDT_CLKSRC_IRC, WDT_MODE_RESET);
	WDT_Start(timeout_us);
}

void WATCHDOG::feed() {
	WDT_Feed();
}
