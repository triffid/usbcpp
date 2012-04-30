#include "USBHW.hpp"

#include "LPC17xx.h"
#include "lpc17xx_clkpwr.h"



USBHW::USBHW() {}

void USBHW::init() {
	// P2.9 -> USB_CONNECT
	LPC_PINCON->PINSEL4 &= ~0x000C0000;
	LPC_PINCON->PINSEL4 |=  0x00040000;

	// P1.18 -> USB_UP_LED
	// P1.30 -> VBUS
	LPC_PINCON->PINSEL3 &= ~0x30000030;
	LPC_PINCON->PINSEL3 |=  0x20000010;

	// P0.29 -> USB_D+
	// P0.30 -> USB_D-
	LPC_PINCON->PINSEL1 &= ~0x3C000000;
	LPC_PINCON->PINSEL1 |=  0x14000000;

	// enable PUSB
	LPC_SC->PCONP |= CLKPWR_PCONP_PCUSB;

	LPC_USB->USBClkCtrl = 0x12;	                  /* Dev clock, AHB clock enable  */
	while ((LPC_USB->USBClkSt & 0x12) != 0x12);

	// disable/clear all interrupts for now
	LPC_USB->USBDevIntEn = 0;
	LPC_USB->USBDevIntClr = 0xFFFFFFFF;
	LPC_USB->USBDevIntPri = 0;

	LPC_USB->USBEpIntEn = 0;
	LPC_USB->USBEpIntClr = 0xFFFFFFFF;
	LPC_USB->USBEpIntPri = 0;

	// by default, only ACKs generate interrupts
// 	HwNakIntEnable(0);

// 	NVIC_EnableIRQ(USB_IRQn);

// 	LPC_USB->USBDevIntEn |= FRAME | DEV_STAT | EP_SLOW;
}

void USBHW::connect() {
}

void USBHW::disconnect() {
}

