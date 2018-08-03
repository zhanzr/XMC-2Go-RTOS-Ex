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
#define BLINK_DELAY_TICK	500

void tx_Thread (void const *argument);
void rx_Thread (void const *argument);

osThreadDef(tx_Thread, osPriorityNormal, 1, 0);
osThreadDef(rx_Thread, osPriorityNormal, 1, 0);

osThreadId T_thread_ID_1;																			
osThreadId T_thread_ID_2;
/*----------------------------------------------------------------------------
  Define the mutex
 *---------------------------------------------------------------------------*/	
osMutexId uart_mutex;
osMutexDef (uart_mutex);

osMessageQId Q_U16;																		//define the message queue
osMessageQDef (Q_U16, 10, uint16_t);
osEvent  result;

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

int mutex_printf (const char *format, ...)
{
	osMutexWait(uart_mutex, osWaitForever);
	
  va_list args;
  int retval;

  va_start (args, format);
	
  retval = vfprintf (stdout, format, args);	
	
  va_end (args);
	
	osMutexRelease(uart_mutex);
	
  return retval;
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


void tx_Thread (void const *argument) 
{
	for (;;) 
	{
		uint16_t tmpU16 = rand();
		mutex_printf("sending to queue: %4X\n", tmpU16); 
		osMessagePut(Q_U16, tmpU16, osWaitForever);			
		osDelay(BLINK_DELAY_TICK*2);
	}
}

void rx_Thread (void const *argument) 
{
	for(;;)
	{
		result = 	osMessageGet(Q_U16,osWaitForever);				//wait for a message to arrive
		mutex_printf("receiving: %4X\n\n", result.value.v);  // write the data to the STDOUT
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

	printf("2Go Message Queue @ %u Hz %s\n", 
	SystemCoreClock, 
	osKernelSystemId);
	
	#ifdef __MICROLIB
	printf("Microlib\n");
	#else
	printf("StandardLib\n");
#endif

	uart_mutex = osMutexCreate(osMutex(uart_mutex));
		
	Q_U16 = osMessageCreate(osMessageQ(Q_U16),NULL);					//create the message queue

	T_thread_ID_1 = osThreadCreate(osThread(tx_Thread), (void*)1);	
	T_thread_ID_2 = osThreadCreate(osThread(rx_Thread), (void*)2);
	
	srand((uint32_t)__TIME__);
	
	osKernelStart ();                         // start thread execution 
}
