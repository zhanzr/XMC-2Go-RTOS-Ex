/**
 * @file
 * @date 1:27 AM 7/25/2018
 * @version 1.0.0
 *
 * @brief RTOS example
 *
 * Two threads are defined to switch on/off the LEDs in the board periodically
 *
 * History
 *
 * Version 1.0.0 
 * - Initial
 *
 */

#include "cmsis_os.h"
#include "xmc_gpio.h"

#define LED1 P1_0
#define LED2 P1_1
#define BLINK_DELAY_N	100000
#define BLINK_DELAY_MS	500

void timer_cb(void const *param);

osTimerDef(timer0_handle, timer_cb);
osTimerDef(timer1_handle, timer_cb);

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
/*----------------------------------------------------------------------------
  Timer callback function. Toggle the LED associated with the timer
 *---------------------------------------------------------------------------*/
void timer_cb(void const *param)
{
	switch( (uint32_t) param)
	{
		case 1:
			LED_Toggle(1);
		break;

		case 2:
			LED_Toggle(2);
		break;

		default:
		break;
	}
}

int main(void)
{
	osKernelInitialize ();                    // initialize CMSIS-RTOS
		
	LED_Initialize ();
	
	osTimerId timer1 = osTimerCreate(osTimer(timer0_handle), osTimerPeriodic, (void *)1);	
	osTimerId timer2 = osTimerCreate(osTimer(timer1_handle), osTimerPeriodic, (void *)2);	

	osTimerStart(timer1, BLINK_DELAY_MS);	
	osTimerStart(timer2, BLINK_DELAY_MS*2);	
	
	osKernelStart ();                         // start thread execution 
	while(1)
	{
		;
	}
}
