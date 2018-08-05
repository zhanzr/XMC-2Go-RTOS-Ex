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
#define BLINK_DELAY_TICK	500
#define DTS_SAMPLE_TICK	400

#define LED_SIGNAL_1	0x01
#define LED_SIGNAL_2	0x02

void led_Thread1 (void const *argument);
void led_Thread2 (void const *argument);

//note the raised priority for led_thread 1
osThreadDef(led_Thread1, osPriorityAboveNormal, 1, 0);
osThreadDef(led_Thread2, osPriorityNormal, 1, 0);

osThreadId T_led_ID1;
osThreadId T_led_ID2;

/*----------------------------------------------------------------------------
  Define the semaphore
 *---------------------------------------------------------------------------*/	
osSemaphoreId sem1;									
osSemaphoreDef(sem1);

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

void led_Thread1 (void const *argument) 
{
	for (;;) 
	{
		osSemaphoreAcquire(sem1, osWaitForever);
		LED_Off(1);                          
		osSemaphoreAcquire(sem1, osWaitForever);
		LED_On(1);                        
	}
}

void led_Thread2 (void const *argument) 
{
	for (;;) 
	{
		LED_On(2);		
		osSemaphoreRelease(sem1);
		osDelay(500);
		LED_Off(2);
		osDelay(500);   
	}
}

void DTS_Init(void)
{
	/* Enable DTS */
	XMC_SCU_StartTempMeasurement();
  
//	XMC_SCU_INTERRUPT_EnableEvent(XMC_SCU_INTERRUPT_EVENT_TSE_DONE);
	//limit Kelvin degree temperature higher compare limit in range [233,388]  	
	XMC_SCU_SetTempHighLimit(273 + 37);
	
	//limit Kelvin degree temperature lower compare limit in range [233,388]  
	XMC_SCU_SetTempLowLimit(273 + 36);
	
	XMC_SCU_INTERRUPT_EnableEvent(XMC_SCU_INTERRUPT_EVENT_TSE_HIGH);
  XMC_SCU_INTERRUPT_EnableEvent(XMC_SCU_INTERRUPT_EVENT_TSE_LOW);
  NVIC_SetPriority(SCU_1_IRQn, 3);
  NVIC_EnableIRQ(SCU_1_IRQn);
}

volatile uint32_t g_tmpU32;
volatile XMC_SCU_INTERRUPT_EVENT_t g_sch_event;
void SCU_1_IRQHandler(void)
{
	g_sch_event = XMC_SCU_INTERUPT_GetEventStatus();
	
//	XMC_SCU_INTERRUPT_ClearEventStatus(XMC_SCU_INTERRUPT_EVENT_TSE_DONE);	
	if(XMC_SCU_INTERRUPT_EVENT_TSE_HIGH == (g_sch_event&XMC_SCU_INTERRUPT_EVENT_TSE_HIGH))
	{
		XMC_SCU_INTERRUPT_ClearEventStatus(XMC_SCU_INTERRUPT_EVENT_TSE_HIGH);	
		osSignalSet	(T_led_ID1,LED_SIGNAL_1);	
	}
	
	if(XMC_SCU_INTERRUPT_EVENT_TSE_LOW == (g_sch_event&XMC_SCU_INTERRUPT_EVENT_TSE_LOW))
	{
		XMC_SCU_INTERRUPT_ClearEventStatus(XMC_SCU_INTERRUPT_EVENT_TSE_LOW);	
		osSignalSet	(T_led_ID2,LED_SIGNAL_2);
	}
	
}

void __svc(1) DTS_sample(void);
void __SVC_1(void)
{
//		g_tmpU32 = XMC_SCU_GetTemperature();
		g_tmpU32 = XMC_SCU_CalcTemperature();	
}


/*----------------------------------------------------------------------------
 Define the thread handles and thread parameters
 *---------------------------------------------------------------------------*/

int main(void)
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	LED_Initialize ();
	
//	DTS_Init();

	T_led_ID1 = osThreadCreate(osThread(led_Thread1), NULL);	
	T_led_ID2 = osThreadCreate(osThread(led_Thread2), NULL);
	sem1 = osSemaphoreNew(1, 0, NULL);	
	
	osKernelStart ();                         // start thread execution 
}
