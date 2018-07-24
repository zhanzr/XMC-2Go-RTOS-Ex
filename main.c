/*
 * Copyright (C) 2016 Infineon Technologies AG. All rights reserved.
 *
 * Infineon Technologies AG (Infineon) is supplying this software for use with
 * Infineon's microcontrollers.
 * This file can be freely distributed within development tools that are
 * supporting such microcontrollers.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 */

/**
 * @file
 * @date 02 May, 2016
 * @version 1.0.0
 *
 * @brief XMC1100 XMC_2Go RTX Blinky example
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
