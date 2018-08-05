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
 * blinkLED: blink LED and wait for signal to go to next LED
 *----------------------------------------------------------------------------*/
void blinkLED(void const *argument) 
{
  for (;;) 
	{
    XMC_GPIO_ToggleOutput(LED1);
    osSignalWait(0x0001, osWaitForever);
    XMC_GPIO_ToggleOutput(LED2);
    osSignalWait(0x0001, osWaitForever);
  }

}
osThreadId tid_blinkLED;
osThreadDef(blinkLED, osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 * mainThread: Switch on/off LEDs sequentially every 500ms
 *----------------------------------------------------------------------------*/
void mainThread(void const *argument) 
{
  for (;;) 
	{
    osDelay(500);
    osSignalSet(tid_blinkLED, 0x0001);
  }
}
osThreadId tid_mainThread;
osThreadDef(mainThread, osPriorityNormal, 1, 0);

int main(void)
{
	osKernelInitialize();
	
	/* Initialize peripherals */
	XMC_GPIO_SetMode(LED1, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
	XMC_GPIO_SetMode(LED2, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);
	
  /* create threads */
  tid_blinkLED = osThreadCreate(osThread(blinkLED), NULL);
	tid_mainThread = osThreadCreate(osThread(mainThread), NULL);
	
	osKernelStart();
}
