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

#define LED_SIGNAL_1	0x01
#define LED_SIGNAL_2	0x02

void led_Thread1 (void const *argument);
void led_Thread2 (void const *argument);
void signal_Thread (void const *argument);

osThreadDef(led_Thread1, osPriorityNormal, 1, 0);
osThreadDef(led_Thread2, osPriorityNormal, 1, 0);
osThreadDef(signal_Thread, osPriorityNormal, 1, 0);

osThreadId T_led_ID1;
osThreadId T_led_ID2;
osThreadId T_signal;

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
  Flash LED 1 when signaled by the other thread
 *---------------------------------------------------------------------------*/
void led_Thread1 (void const *argument) 
{
	for (;;) 
	{
		osSignalWait (LED_SIGNAL_1,osWaitForever);
		LED_Toggle(1);                          
	}
}

/*----------------------------------------------------------------------------
  Flash LED 2 when signaled by the other thread
 *---------------------------------------------------------------------------*/
void led_Thread2 (void const *argument) 
{
	for (;;) 
	{
		osSignalWait (LED_SIGNAL_2,osWaitForever);
		LED_Toggle(2);                          
	}
}

/*----------------------------------------------------------------------------
  Synchronise the flashing of LEDs by setting a signal flag
 *---------------------------------------------------------------------------*/
void signal_Thread (void const *argument) 
{
	for (;;) 
	{
		osSignalSet	(T_led_ID1,LED_SIGNAL_1);
		osDelay(BLINK_DELAY_MS);

		osSignalSet	(T_led_ID2,LED_SIGNAL_2);
		osDelay(BLINK_DELAY_MS);
	}
}

/*----------------------------------------------------------------------------
 Define the thread handles and thread parameters
 *---------------------------------------------------------------------------*/

int main(void)
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	LED_Initialize ();
	
	T_led_ID1 = osThreadCreate(osThread(led_Thread1), NULL);	
	T_led_ID2 = osThreadCreate(osThread(led_Thread2), NULL);
	T_signal = osThreadCreate(osThread(signal_Thread), NULL);
	
	osKernelStart ();                         // start thread execution 
}
