#include	<LPC17xx.h>

typedef void (*ISRHandler) (void);

struct ISRMap {
    void       *InitSP;
    ISRHandler  Reset_Handler;
    ISRHandler  NMI_Handler;
    ISRHandler  HardFault_Handler;
    ISRHandler  MemManage_Handler;
    ISRHandler  BusFault_Handler;
    ISRHandler  UsageFault_Handler;
    void       *reserved0[4];
    ISRHandler  SVCall_Handler;
    ISRHandler  Debug_Handler;
    void       *reserved1;
    ISRHandler  PendSV_Handler;
    ISRHandler  Systick_Handler;

    ISRHandler  WDT_IRQ;
    ISRHandler  TIMER0_IRQ;
    ISRHandler  TIMER1_IRQ;
    ISRHandler  TIMER2_IRQ;
    ISRHandler  TIMER3_IRQ;
    ISRHandler  UART0_IRQ;
    ISRHandler  UART1_IRQ;
    ISRHandler  UART2_IRQ;
    ISRHandler  UART3_IRQ;
    ISRHandler  PWM1_IRQ;
    ISRHandler  I2C0_IRQ;
    ISRHandler  I2C1_IRQ;
    ISRHandler  I2C2_IRQ;
    ISRHandler  SPI_IRQ;
    ISRHandler  SSP0_IRQ;
    ISRHandler  SSP1_IRQ;
    ISRHandler  PLL0_IRQ;
    ISRHandler  RTC_IRQ;
    ISRHandler  EINT0_IRQ;
    ISRHandler  EINT1_IRQ;
    ISRHandler  EINT2_IRQ;
    ISRHandler  EINT3_IRQ;
    ISRHandler  ADC_IRQ;
    ISRHandler  BOD_IRQ;
    ISRHandler  USB_IRQ;
    ISRHandler  CAN_IRQ;
    ISRHandler  DMA_IRQ;
    ISRHandler  I2S_IRQ;
    ISRHandler  ENET_IRQ;
    ISRHandler  RIT_IRQ;
    ISRHandler  MCPWM_IRQ;
    ISRHandler  QEI_IRQ;
    ISRHandler  PLL1_IRQ;
    ISRHandler  USBActivity_IRQ;
    ISRHandler  CANActivity_IRQ;
};

/* __Vectors is the ISR table written into start of flash and assembled by our linker
 * consider it read-only, any attempt to write will throw a memory manager fault
 *
 * __Vectors is defined in startup.S, it does not need to be created anywhere else
 */
extern struct ISRMap __Vectors;

/* if you intend to use soft-remapping,
 * define softmap somewhere in your application like this:
 * struct ISRMap	__attribute__ ((section(".ARM.__AT_0x10000000"))) softmap;
 * it MUST be on a 256-byte boundary, easist is to put it at start of ram
 *
 * You must call NVIC_SoftISR() before any use of NVIC_SetISR
 * otherwise the results will be overwritten
 */

extern struct ISRMap softmap;

#define	NVIC_SetISR(name, function) do { ((ISRHandler *) softmap)[(name + 16)] = function; } while (0)

#define NVIC_SoftISR() do { memcpy(softmap, __Vectors, sizeof(struct ISRMap)); NVIC_SetVTOR(softmap); } while (0);
