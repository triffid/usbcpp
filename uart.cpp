#include "uart.hpp"

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_clkpwr.h"


UART::UART(uint8_t port, uint32_t baud) {
	UART_CFG_Type UARTConfigStruct;
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	PINSEL_CFG_Type PinCfg;

	if (port == 0) {
		PinCfg.Funcnum = 1;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 0;

		PinCfg.Pinnum = 2;
		PINSEL_ConfigPin(&PinCfg);

		PinCfg.Pinnum = 3;
		PINSEL_ConfigPin(&PinCfg);

		u = (LPC_UART_TypeDef *) LPC_UART0;
	}
	else if (port == 1) {
		PinCfg.Funcnum = 2;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Portnum = 2;

		PinCfg.Pinnum = 0;
		PINSEL_ConfigPin(&PinCfg);

		PinCfg.Pinnum = 1;
		PINSEL_ConfigPin(&PinCfg);

		u = (LPC_UART_TypeDef *) LPC_UART1;
	}
	else {
		return;
	}

	UART_ConfigStructInit(&UARTConfigStruct);

	UARTConfigStruct.Baud_rate = baud;

	UART_Init(u, &UARTConfigStruct);

	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
	UART_FIFOConfig(u, &UARTFIFOConfigStruct);

	UART_TxCmd(u, ENABLE);
}

// UART::~UART() {
// 	while (UART_CheckBusy(u) == SET);
// 	UART_DeInit(u);
// }

void UART::send(uint8_t *buf, uint32_t buflen) {
	UART_Send(u, buf, buflen, BLOCKING);
}

uint32_t UART::recv(uint8_t *buf, uint32_t buflen) {
	return UART_Receive(u, buf, buflen, NONE_BLOCKING);
}

uint8_t UART::cansend() {
	if (u->LSR & UART_LSR_THRE)
		return 1;
	return 0;
}

uint8_t UART::canrecv() {
	if (u->LSR & UART_LSR_RDR)
		return 1;
	return 0;
}

