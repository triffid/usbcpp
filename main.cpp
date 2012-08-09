#include "USB.hpp"

#include "USBCDC.hpp"
#include "USBMSC.hpp"
#include "USBECM.hpp"

#include "uart.hpp"
#include "gpio.hpp"

#include "CLOCK.hpp"
#include "WATCHDOG.hpp"

USB u;

USBCDC uc;
USBMSC msc;
USBECM ecm;

WATCHDOG w(5 WATCHDOG_SECONDS);

clock sysclock;

UART dbg(0, 115200);

uint8_t mac[6] = { 0xAE, 0xF0, 0x28, 0x5D, 0x66, 0x21 };

#define N_LEDS 5

GPIO leds[N_LEDS] = {
	GPIO(1, 18, GPIO_DIR_OUTPUT),
	GPIO(1, 19, GPIO_DIR_OUTPUT),
	GPIO(1, 20, GPIO_DIR_OUTPUT),
	GPIO(1, 21, GPIO_DIR_OUTPUT),
	GPIO(4, 28, GPIO_DIR_OUTPUT),
};

uint8_t mystery[16] = { 0x02, 0x48 , 0x4f , 0xf3 , 0xc0 , 0x04 , 0x04 , 0x05  , 0x81 , 0xc1 , 0x1c , 0x00 , 0x01 , 0x03 , 0x00 , 0x3d };

void dbgled(int l) {
// 	iprintf("l%X:", l & ((1 << N_LEDS) - 1));
	for (int i = 0; i < N_LEDS; i++) {
		leds[i].write(l & 1);
// 		iprintf("[%d:%d=%d]", leds[i].port, leds[i].pin, l & 1);
		l >>= 1;
	}
// 	iprintf("\n");
}

int main(void) {
	dbgled(0);

	iprintf("start\n");

	uc.attach(&u);
	msc.attach(&u);
	ecm.setMAC(mac);
	ecm.attach(&u);

	iprintf("u.dump\n");

	u.dumpDescriptors();

	iprintf("u.init\n");

	u.init();

	sysclock.setTimeout(5 S);

	// because leds[0] is connected to USB_UP_LED signal and usb hardware takes it over
	leds[0].setup();

// 	dbgled(15);

// 	int l = 0;

	while (1) {
// 		u.USBHW::HwISR();
		if (sysclock.poll()) {
// 			iprintf("-------------------\n");
// 			iprintf("USBClkCtrl:   0x%8lX\n", LPC_USB->USBClkCtrl);
// 			iprintf("USBClkSt:     0x%8lX\n", LPC_USB->USBClkSt);
// 			iprintf("USBIntSt:     0x%8lX\n", LPC_SC->USBIntSt);
// 			iprintf("USBDevIntSt:  0x%8lX\n", LPC_USB->USBDevIntSt);
// 			iprintf("USBDevIntEn:  0x%8lX\n", LPC_USB->USBDevIntEn);
// 			iprintf("USBDevIntPri: 0x%8lX\n", LPC_USB->USBDevIntPri);
			iprintf("Fr: %d\n", u.USBCTRL::lastFrame());
		}
		w.feed();
// 		l++;
// 		if ((l & ((1UL << 19) - 1UL)) == 0)
// 			dbgled(l >> 19);
	}
}

extern "C" {
	int _write(int fd, uint8_t *buf, size_t buflen) {
		dbg.send((uint8_t *) buf, buflen);
		return buflen;
	}

	void NMI_Handler() {
		iprintf("NMI\n");
		for (;;);
	}
	void HardFault_Handler() {
		iprintf("HardFault\n");
		for (;;);
	}
	void MemManage_Handler() {
		iprintf("MemManage\n");
		for (;;);
	}
	void BusFault_Handler() {
		iprintf("BusFault\n");
		for (;;);
	}
	void UsageFault_Handler() {
		iprintf("UsageFault\n");
		for (;;);
	}
}
