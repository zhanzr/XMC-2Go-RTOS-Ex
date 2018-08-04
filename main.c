#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <XMC1100.h>
#include <xmc_scu.h>
#include <xmc_rtc.h>
#include <xmc_uart.h>
#include <xmc_gpio.h>
#include <xmc_flash.h>

#include "cmsis_os.h"

#include "Driver_USART.h"

#define LED1 P1_0
#define LED2 P1_1
#define BLINK_DELAY_N	100000
#define BLINK_DELAY_MS	500

/* Initialize peripherals */
void LED_Initialize(void)
{
	/* Initialize peripherals */
	XMC_GPIO_SetMode(LED1, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
	XMC_GPIO_SetMode(LED2, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
}

void LED_On(uint8_t n)
{
	switch(n)
	{
		case 1:
			XMC_GPIO_SetOutputHigh(LED1);
			break;
		
		case 2:
			XMC_GPIO_SetOutputHigh(LED2);
			break;
		
		default:
			break;
	}
}

void LED_Off(uint8_t n)
{
	switch(n)
	{
		case 1:
			XMC_GPIO_SetOutputLow(LED1);
			break;
		
		case 2:
			XMC_GPIO_SetOutputLow(LED2);
			break;
		
		default:
			break;
	}
}

void LED_Toggle(uint8_t n)
{
	switch(n)
	{
		case 1:
			XMC_GPIO_ToggleOutput(LED1);
			break;
		
		case 2:
			XMC_GPIO_ToggleOutput(LED2);
			break;
		
		default:
			break;
	}
}

int stdout_putchar(int ch)
{
	XMC_UART_CH_Transmit(XMC_UART0_CH0, ch);
	for(uint32_t i=0; i<1000; ++i)
	{
		__NOP();
	}
	return ch;
}

/*----------------------------------------------------------------------------
  Simple delay loop 
 *---------------------------------------------------------------------------*/

void delay(uint32_t count)
{
	for(uint32_t index =0; index<count; index++)
	{
		__NOP();
	}
}

void delay_wfe(uint32_t count)
{
	for(uint32_t index =0; index<count; index++)
	{
		__wfe();
	}
}

/*--------------------------- os_idle_demon ---------------------------------*/

/// \brief The idle demon is running when no other thread is ready to run
void os_idle_demon (void) {
 
  for (;;) {
    /* HERE: include optional user code to be executed when no thread runs.*/
		LED_Toggle(1);
		delay_wfe(BLINK_DELAY_MS);
		LED_Toggle(2);
		delay_wfe(BLINK_DELAY_MS*3);
	}
}

/*----------------------------------------------------------------------------
 Define the thread handles and thread parameters
 *---------------------------------------------------------------------------*/

int main(void)
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	LED_Initialize ();
		
	osKernelStart ();                         // start thread execution 
//	while(1)
//	{
//		;
//	}
}
