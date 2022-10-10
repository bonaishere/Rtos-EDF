/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/


/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */

TaskHandle_t Button_1_Monitor_Handler = NULL;
TaskHandle_t Button_2_Monitor_Handler = NULL;
TaskHandle_t Periodic_Transmitter_Handler = NULL;
TaskHandle_t Uart_Receiver_Handler = NULL;
TaskHandle_t Load_1_Simulation_Handler = NULL;
TaskHandle_t Load_2_Simulation_Handler = NULL;

int Task1_in_time=0,Task2_in_time=0,Task3_in_time=0,Task4_in_time=0,Task5_in_time=0,Task6_in_time=0;
int Task1_out_time=0,Task2_out_time=0,Task3_out_time=0,Task4_out_time=0,Task5_out_time=0,Task6_out_time=0;
int Task1_total_time=0,Task2_total_time=0,Task3_total_time=0,Task4_total_time=0,Task5_total_time=0,Task6_total_time=0;
int Total_System_Time = 0;
float CPU_Load = 0;

unsigned char Tasks_Data[3][22]={0};

void string_copy(char *str_des, char *str_src)
{
	unsigned char i=0;
	while(str_src[i] != 0)
		{		
			str_des[i] = str_src[i];
			i++;
		}
		str_des[i] = '\0';
}


void Button_1_Monitor(void)
{
	static int flag1=0,flag2=0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	int i=0;
	vTaskSetApplicationTaskTag( NULL, ( void * ) 1 );
	while(1)
	{	
		if(GPIO_read(PORT_0,PIN1) == PIN_IS_HIGH && flag1 == 0)
		{
			string_copy(Tasks_Data[0],"Button1_Falling(ON) | ");
			flag1=1;
			flag2=0;
		}else if(GPIO_read(PORT_0,PIN1) == PIN_IS_LOW && flag2 == 0)
		{
			string_copy(Tasks_Data[0],"Button1_Rising(OFF) | ");
			flag2=1;
			flag1=0;
		}
		vTaskDelayUntil( &xLastWakeTime, 50);
	}
}

void Button_2_Monitor(void)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	int i=0;
	static int flag1=0,flag2=0;
	vTaskSetApplicationTaskTag( NULL, ( void * ) 2 );
	while(1)
	{		
		if(GPIO_read(PORT_0,PIN2) == PIN_IS_HIGH && flag1 == 0)
		{
			string_copy(Tasks_Data[1]," Button2_Falling(ON) |");
			flag1=1;
			flag2=0;
		}else if(GPIO_read(PORT_0,PIN2) == PIN_IS_LOW && flag2 == 0)
		{
			string_copy(Tasks_Data[1]," Button2_Rising(OFF) |");			
			flag2=1;
			flag1=0;
		}
		vTaskDelayUntil( &xLastWakeTime, 50);
	}
}

void Periodic_Transmitter(void)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * ) 3 );
	while(1)
	{		
		string_copy(Tasks_Data[2]," Periodic_String\n");
		vTaskDelayUntil(&xLastWakeTime, 100);
	}
}


void Uart_Receiver(void)
{
	static char flag_first=0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	int i=0;
	vTaskSetApplicationTaskTag( NULL, ( void * ) 4 );
	if(flag_first==0)
	{
		vTaskDelayUntil( &xLastWakeTime, 20);
	}
	while(1)
	{	
		vSerialPutString(*Tasks_Data, 62);	
		vTaskDelayUntil( &xLastWakeTime, 20);
	}
}

void Load_1_Simulation(void)
{
	int i=0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * ) 5 );
	while(1)
	{
		for(i=0;i<33000;i++)//Execution Time = 5ms
		{
			i=i;
		}
		vTaskDelayUntil(&xLastWakeTime, 10);
	}
}
void Load_2_Simulation(void)
{
	int i=0;
	TickType_t xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag( NULL, ( void * ) 6 );
	while(1)
	{
		for(i=0;i<78500;i++) //Execution Time = 12ms
		{
			i=i;
		}
		vTaskDelayUntil(&xLastWakeTime, 100);
	}
}
void vApplicationTickHook( void )
{
	GPIO_write(PORT_0, PIN0,PIN_IS_HIGH);
	GPIO_write(PORT_0, PIN0,PIN_IS_LOW);	
}


int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	
   /* Create Tasks here */
		xTaskPeriodicCreate((void *)Button_1_Monitor,"Button_1_Monitor",configMINIMAL_STACK_SIZE,NULL, 2,&Button_1_Monitor_Handler,50);
 
		xTaskPeriodicCreate((void *)Button_2_Monitor,"Button_2_Monitor",configMINIMAL_STACK_SIZE,NULL, 2,&Button_2_Monitor_Handler,50);
		
		xTaskPeriodicCreate((void *)Periodic_Transmitter,"Periodic_Transmitter",configMINIMAL_STACK_SIZE,NULL, 2,&Periodic_Transmitter_Handler,100);
	
		xTaskPeriodicCreate((void *)Uart_Receiver,"Uart_Receiver",configMINIMAL_STACK_SIZE,NULL, 2,&Uart_Receiver_Handler,20);
	
		xTaskPeriodicCreate((void *)Load_1_Simulation,"Load_1_Simulation",configMINIMAL_STACK_SIZE,NULL, 2,&Load_1_Simulation_Handler,10);
		xTaskPeriodicCreate((void *)Load_2_Simulation,"Load_2_Simulation",configMINIMAL_STACK_SIZE,NULL, 2,&Load_2_Simulation_Handler,100);
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


