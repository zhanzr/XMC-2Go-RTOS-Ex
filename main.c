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
#define BLINK_DELAY_N	50000

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

/*----------------------------------------------------------------------------
  Flash LED 1
 *---------------------------------------------------------------------------*/
void led_thread1 (void const *argument) 
{
	while(1)
	{
		LED_On(1);                          
		delay(BLINK_DELAY_N);
		LED_Off(1);
		delay(BLINK_DELAY_N);
	}
}

/*----------------------------------------------------------------------------
 Flash LED 2
 *---------------------------------------------------------------------------*/
void led_thread2 (void const *argument) 
{
	while(1)
	{
		LED_On(2);                          
		delay(BLINK_DELAY_N);
		LED_Off(2);
		delay(BLINK_DELAY_N);
	}
}

/*----------------------------------------------------------------------------
 Define the thread handles and thread parameters
 *---------------------------------------------------------------------------*/

osThreadId main_ID,led_ID1,led_ID2;	
osThreadDef(led_thread2, osPriorityNormal, 1, 0);
osThreadDef(led_thread1, osPriorityNormal, 1, 0);

int main(void)
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	LED_Initialize ();
	
	led_ID2 = osThreadCreate(osThread(led_thread2), NULL);
	led_ID1 = osThreadCreate(osThread(led_thread1), NULL);

	osKernelStart ();                         // start thread execution 
	while(1)
	{
		;
	}
}
