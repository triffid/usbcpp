#include "USB.hpp"

#include "USBCDC.hpp"
#include "USBMSC.hpp"
#include "USBECM.hpp"

#include "uart.hpp"

#include "CLOCK.hpp"

USB u;
USBCDC uc;
USBMSC msc;
USBECM ecm;
clock sysclock;

UART dbg(0, 230400);

int main(void){
	dbg.send((uint8_t *) "start\n", 6);

	uc.attach(&u);
	msc.attach(&u);
	uint8_t mac[6] = { 0xAE, 0xF0, 0x28, 0x5D, 0x66, 0x21 };
	ecm.setMAC(mac);
	ecm.attach(&u);

	u.dumpDescriptors();

	u.init();

	sysclock.setTimeout(5 S);

	while (1) {
		if (sysclock.poll()) {
			printf("-------------------\n");
			printf("USBClkCtrl:   0x%8lX\n", LPC_USB->USBClkCtrl);
			printf("USBClkSt:     0x%8lX\n", LPC_USB->USBClkSt);
			printf("USBIntSt:     0x%8lX\n", LPC_SC->USBIntSt);
			printf("USBDevIntSt:  0x%8lX\n", LPC_USB->USBDevIntSt);
			printf("USBDevIntEn:  0x%8lX\n", LPC_USB->USBDevIntEn);
			printf("USBDevIntPri: 0x%8lX\n", LPC_USB->USBDevIntPri);
		}
	}
}

extern "C" {
	void __attribute__ ((interrupt)) USB_IRQHandler(void) {
		u.IRQ();
	}

	int _write(int fd, uint8_t *buf, size_t buflen) {
		dbg.send((uint8_t *) buf, buflen);
		return buflen;
	}
}
