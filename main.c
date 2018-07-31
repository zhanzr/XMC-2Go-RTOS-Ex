/**
 * @file
 * @date 1:27 AM 7/25/2018
 * @version 1.0.0
 *
 * @brief RTOS example
 *
 * RTOS Demo, see READ.md for document
 *
 * History
 *
 * Version 1.0.0 
 * - Initial
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
#define BLINK_DELAY_TICK	100
#define DTS_SAMPLE_TICK	400

#define LED_SIGNAL_1	0x01
#define LED_SIGNAL_2	0x02

void test_Thread1 (void const *argument);
void test_Thread2 (void const *argument);
osThreadDef(test_Thread1, osPriorityNormal, 1, 0);
osThreadDef(test_Thread2, osPriorityNormal, 1, 0);

osThreadId T_test_Thread1;
osThreadId T_test_Thread2;
	
osSemaphoreId semArrived1;									//define the semaphores
osSemaphoreDef(semArrived1);
osSemaphoreId semArrived2;
osSemaphoreDef(semArrived2);

extern ARM_DRIVER_USART Driver_USART0;
static ARM_DRIVER_USART *UARTdrv = &Driver_USART0; 

void UART_cb(uint32_t event)
{
    switch (event)
    {
    case ARM_USART_EVENT_RECEIVE_COMPLETE:  
     break;
     
    case ARM_USART_EVENT_TRANSFER_COMPLETE:
    case ARM_USART_EVENT_SEND_COMPLETE:
    case ARM_USART_EVENT_TX_COMPLETE:
        break;
 
    case ARM_USART_EVENT_RX_TIMEOUT:
		/* Error: Call debugger or replace with custom error handling */
        break;
 
    case ARM_USART_EVENT_RX_OVERFLOW:
    case ARM_USART_EVENT_TX_UNDERFLOW:
		default:
		/* Error: Call debugger or replace with custom error handling */
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

static uint32_t old_tick1;
static uint32_t old_tick2;
static uint32_t delta_tick1;
static uint32_t delta_tick2;

void test_Thread1 (void const *argument) 
{
	for (;;)
	{
		LED_Off(1);
		osDelay(BLINK_DELAY_TICK);
		
		osSemaphoreRelease(semArrived2);					//The semaphores ensure both tasks arrive here
		osSemaphoreWait(semArrived1,osWaitForever);		//before continuing
		
		delta_tick1 = osKernelSysTick() - old_tick1;		
		old_tick1 = osKernelSysTick();
		
		LED_On(1);
		osDelay(BLINK_DELAY_TICK);
		
		osSemaphoreRelease(semArrived2);					//The semaphores ensure both tasks arrive here
		osSemaphoreWait(semArrived1,osWaitForever);		//before continuing
  }
}

void test_Thread2 (void const *argument) 
{
	uint32_t para = (uint32_t)argument;
		
	for (;;) 
	{		
		osSemaphoreRelease(semArrived1);					//The semaphores ensure both tasks arrive here
		osSemaphoreWait(semArrived2,osWaitForever);		//before continuing
		
		delta_tick2 = osKernelSysTick() - old_tick2;
		old_tick2 = osKernelSysTick();
		printf("%u %u\n", delta_tick1, delta_tick2);
		
		osDelay(BLINK_DELAY_TICK);

		osSemaphoreRelease(semArrived1);					//The semaphores ensure both tasks arrive here
		osSemaphoreWait(semArrived2,osWaitForever);		//before continuing
	}
}


void UART_Init(void)
{
	/*Initialize the UART driver */
  UARTdrv->Initialize(UART_cb);
  UARTdrv->PowerControl(ARM_POWER_FULL);
  UARTdrv->Control(ARM_USART_MODE_ASYNCHRONOUS |
                   ARM_USART_DATA_BITS_8 |
                   ARM_USART_PARITY_NONE |
                   ARM_USART_STOP_BITS_1 , 256000);
   
  /* Enable the Transmitter/Receiver line */
  UARTdrv->Control (ARM_USART_CONTROL_TX, 1);
	UARTdrv->Control (ARM_USART_CONTROL_RX, 1);
}
/*----------------------------------------------------------------------------
 Define the thread handles and thread parameters
 *---------------------------------------------------------------------------*/

int main(void)
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	LED_Initialize ();
	
	UART_Init();

	printf("2Go Rendezvous @ %u Hz %s\n", 
	SystemCoreClock, 
	osKernelSystemId);
	
	#ifdef __MICROLIB
	printf("Microlib\n");
	#else
	printf("StandardLib\n");
#endif
	
	semArrived1 = osSemaphoreCreate(osSemaphore(semArrived1), 0);				
	semArrived2 = osSemaphoreCreate(osSemaphore(semArrived2), 0);		
	T_test_Thread1 =	osThreadCreate(osThread(test_Thread1), NULL);
	T_test_Thread2 =	osThreadCreate(osThread(test_Thread2), NULL);

	old_tick1 = old_tick2 = osKernelSysTick();

	osKernelStart ();                         // start thread execution 
}
