#include "USB.hpp"

#include "USBCDC.hpp"
#include "USBMSC.hpp"
#include "USBECM.hpp"

#include "uart.hpp"

#include "CLOCK.hpp"
#include "WATCHDOG.hpp"

USB u;

USBCDC uc;
USBMSC msc;
USBECM ecm;

WATCHDOG w(5 WATCHDOG_SECONDS);

clock sysclock;

UART dbg(0, 230400);

uint8_t mac[6] = { 0xAE, 0xF0, 0x28, 0x5D, 0x66, 0x21 };

int main(void){
	printf("start\n");

	uc.attach(&u);
	msc.attach(&u);
	ecm.setMAC(mac);
	ecm.attach(&u);

	printf("u.dump\n");

	u.dumpDescriptors();

	printf("u.init\n");

	u.init();

	sysclock.setTimeout(5 S);

	while (1) {
		if (sysclock.poll()) {
// 			printf("-------------------\n");
// 			printf("USBClkCtrl:   0x%8lX\n", LPC_USB->USBClkCtrl);
// 			printf("USBClkSt:     0x%8lX\n", LPC_USB->USBClkSt);
// 			printf("USBIntSt:     0x%8lX\n", LPC_SC->USBIntSt);
// 			printf("USBDevIntSt:  0x%8lX\n", LPC_USB->USBDevIntSt);
// 			printf("USBDevIntEn:  0x%8lX\n", LPC_USB->USBDevIntEn);
// 			printf("USBDevIntPri: 0x%8lX\n", LPC_USB->USBDevIntPri);
		}
		w.feed();
	}
}

extern "C" {
	int _write(int fd, uint8_t *buf, size_t buflen) {
		dbg.send((uint8_t *) buf, buflen);
		return buflen;
	}
}
