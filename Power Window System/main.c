/*--------------------------------------------------------------
												Header Files
----------------------------------------------------------------*/
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include "TM4C123.h"

/*--------------------------------------------------------------
												Interrupt Definitions
----------------------------------------------------------------*/
#define PortF_IRQn 30
#define mainSW_INTURRUPT_PortF ((IRQn_Type)30)
#define mainSW_INTURRUPT_PortD ((IRQn_Type)3)
#define mainSW_INTURRUPT_PortB ((IRQn_Type)1)

/*--------------------------------------------------------------
												Function prototypes
----------------------------------------------------------------*/
void PortF_Init();
void PortA_Init();
void PortD_Init();
void PortB_Init();

void motor_up();
void motor_down();
void motor_stop();
//void driver_motor_down();
//void driver_motor_up();


/*--------------------------------------------------------------
									Semaphore Handle Definitions
----------------------------------------------------------------*/
//define a Semaphore handle
xSemaphoreHandle passenger_window_down_button; //passenger down
//semaphores for portB interrupt
xSemaphoreHandle on_off_switch_HIGH; //ON/OFF switch
xSemaphoreHandle on_off_switch_LOW; //ON/OFF switch
xSemaphoreHandle limit_switch_down; //Limit_switch_down
//semaphores for PortD interrupt
xSemaphoreHandle passenger_window_up_button; //passenger up
xSemaphoreHandle driver_window_up_button; //driver up
xSemaphoreHandle driver_window_down_button; //driver down
xSemaphoreHandle emergency_button; //Emergency Switch
xSemaphoreHandle limit_switch_up; //Limit_switch_up
xSemaphoreHandle lockMutex;

/*--------------------------------------------------------------
									Motor handler functions
----------------------------------------------------------------*/
void motor_up(){
	GPIOA -> DATA &= ~(0x0C);
	GPIOA -> DATA |= 0x08;//PA3
}
void motor_down(){
	GPIOA -> DATA &= ~(0x0C);
	GPIOA -> DATA |= 0x04;//PA2
}
void motor_stop(){
	GPIOA -> DATA &= ~(0x0C);
}
/*
void driver_motor_down(){
	GPIOA -> DATA &= ~(0x6C);
	GPIOA -> DATA |= 0x20;//PA5
}
void driver_motor_up(){
	GPIOA -> DATA &= ~(0x6C);
	GPIOA -> DATA |= 0x40;//PA6
}*/

/*--------------------------------------------------------------
									Task Function Implementations
----------------------------------------------------------------*/
//this Task "Handler" is awakened when the semaphore is available
void passengerUp(){
	/*This task handles the interrupt triggered by the passenger up button
	by taking the semaphore from the respective ISR and performing the
	required motor function*/
	xSemaphoreTake(passenger_window_up_button,0);
	uint32_t no_of_interrupts = 1;
	TickType_t xLastAwakeTime = 0;
	TickType_t Time;
	for(;;)
	{
			if(xSemaphoreTake(passenger_window_up_button,portMAX_DELAY)==pdTRUE)
			{
				if(xSemaphoreTake(lockMutex,0) == pdTRUE)
				{
					if(no_of_interrupts % 2 == 0){
						//motor_stop();
						Time = xTaskGetTickCount() - xLastAwakeTime;
						if(Time >= pdMS_TO_TICKS(300)){
							motor_stop();
						}
					}else{
						xLastAwakeTime = xTaskGetTickCount();
						motor_up();
					}
					no_of_interrupts++;
					xSemaphoreGive(lockMutex);
				}
			}
	}
}
void passengerDown()
{
	/*This task handles the interrupt triggered by the passenger down button
	by taking the semaphore from the respective ISR and performing the
	required motor function*/
	xSemaphoreTake(passenger_window_down_button,0);
	uint32_t no_of_interrupts = 1;
	TickType_t xLastAwakeTime = 0;
	TickType_t Time;
	for(;;)
	{
		if(xSemaphoreTake(passenger_window_down_button,portMAX_DELAY)==pdTRUE)
		{
			if(xSemaphoreTake(lockMutex,0) == pdTRUE){
				if(no_of_interrupts % 2 == 0){
					//motor_stop();
					Time = xTaskGetTickCount() - xLastAwakeTime;
					if(Time >= pdMS_TO_TICKS(300)){
						motor_stop();
					}
				}else{
					xLastAwakeTime = xTaskGetTickCount();
					motor_down();
				}
				no_of_interrupts++;
				xSemaphoreGive(lockMutex);
			}
		}
	}
}
void driverUp()
{
	/*This task handles the interrupt triggered by the driver up button
	by taking the semaphore from the respective ISR and performing the
	required motor function*/
	xSemaphoreTake(driver_window_up_button,0);
	uint32_t no_of_interrupts = 1;
	TickType_t xLastAwakeTime = 0;
	TickType_t Time;
	for(;;)
	{
			if(xSemaphoreTake(driver_window_up_button,portMAX_DELAY)==pdTRUE)
			{
				if(no_of_interrupts % 2 == 0){
					Time = xTaskGetTickCount() - xLastAwakeTime;
					if(Time >= pdMS_TO_TICKS(300)){
						motor_stop();
					}
				}else{
					xLastAwakeTime = xTaskGetTickCount();
					motor_up();
				}
				no_of_interrupts++;
			}
	}
}
void driverDown()
{
	/*This task handles the interrupt triggered by the driver down button
	by taking the semaphore from the respective ISR and performing the
	required motor function*/
	xSemaphoreTake(driver_window_down_button,0);
	uint32_t no_of_interrupts = 1;
	TickType_t xLastAwakeTime = 0;
	TickType_t Time;
	for(;;)
	{
			if(xSemaphoreTake(driver_window_down_button,portMAX_DELAY)==pdTRUE)
			{				
				if(no_of_interrupts % 2 == 0){
					Time = xTaskGetTickCount() - xLastAwakeTime;
					if(Time >= pdMS_TO_TICKS(300)){
						motor_stop();
					}
				}else{
					xLastAwakeTime = xTaskGetTickCount();
					motor_down();
				}
				no_of_interrupts++;
			}
	}
}
void limitSwitchUp()
{
	/*This task handles the interrupt triggered when either the limit_switch_down
	or limit_switch_up is triggered*/

	xSemaphoreTake(limit_switch_up,0);

	for(;;)
	{
			if((xSemaphoreTake(limit_switch_up,portMAX_DELAY)==pdTRUE))
			{
			motor_stop();
				while((GPIOD -> DATA & 0x80) == 0){
					GPIOD -> DATA = 0x00;
					GPIOF -> DATA = 0x00;
				}
			motor_stop();
			GPIOF->DATA &= ~(0x06);
			}
	}
}
void limitSwitchDown(){
	xSemaphoreTake(limit_switch_down,0);
	for(;;)
	{
			if((xSemaphoreTake(limit_switch_down,portMAX_DELAY)==pdTRUE))
			{
			motor_stop();
				while((GPIOB -> DATA & 0x10) == 0){
					GPIOD -> DATA = 0x00;
					GPIOF -> DATA = 0x00;
				}
			motor_stop();
			GPIOF->DATA &= ~(0x06);
			}
	}
}
void lockSwitch()
{
		
	/*This task handles the ON/OFF interrupt edge triggering of the ON/OFF switch*/
		uint32_t flag =0;
		xSemaphoreTake(on_off_switch_LOW,0);
		for(;;)
		{
			if(xSemaphoreTake(on_off_switch_LOW,portMAX_DELAY)==pdTRUE){
				if(flag % 2 ==0){
					xSemaphoreTake(lockMutex,0);
					/*GPIOD -> ICR = 0xFF;
					GPIOF -> ICR = 0x01;*/
				}else{
					xSemaphoreGive(lockMutex);
				}
				flag++;	
			}
			GPIOD -> ICR = 0xFF;
			GPIOF -> ICR = 0x01;
		}
}
	
void EmergencySwitch(){
	
	/*This task handles the interrupt triggered by the emergency button
	by taking the semaphore from the respective ISR and performing the
	required motor function*/
	xSemaphoreTake(emergency_button,0);
	/*xSemaphoreTake(passenger_window_up_button,0);
	xSemaphoreTake(driver_window_up_button,0);*/
	for(;;){
			if(xSemaphoreTake(emergency_button,portMAX_DELAY) == pdTRUE){
				/*if((GPIOD -> MIS & 0x01) == 0x01){   //xSemaphoreTake(passenger_window_up_button,portMAX_DELAY) == pdTRUE){
				motor_stop();
				vTaskDelay(5000/portTICK_RATE_MS);
				motor_down();
				}
				if((GPIOD -> MIS & 0x04) == 0x04){   //xSemaphoreTake(driver_window_up_button,portMAX_DELAY) == pdTRUE){
				motor_stop();
				vTaskDelay(5000/portTICK_RATE_MS);
				driver_motor_down();
				}*/
				motor_stop();
				vTaskDelay(500/portTICK_RATE_MS);
				motor_down();
			}
	}
}
//This Periodic task is preempted by the task "Handler"
void InitialTask(){
	for(;;){
	
		vTaskDelay(500/portTICK_RATE_MS);
	}

}
/*--------------------------------------------------------------
												Main Function
----------------------------------------------------------------*/
int main( void )
{
			PortF_Init();
			PortA_Init();
			PortD_Init();
			PortB_Init();

		__ASM("CPSIE i");
	
			/* Create the 'handler' task. This is the task that will be synchronized
			with the interrupt. The handler task is created with a high priority to
			ensure it runs immediately after the interrupt exits. In this case a
			priority of 3 is chosen. */
			xTaskCreate(passengerUp, "PassengerUp", 140, NULL, 2, NULL );
			xTaskCreate(passengerDown, "PassengerDown", 140, NULL, 2, NULL);
			xTaskCreate(driverUp, "driverUp", 140, NULL, 3, NULL );
			xTaskCreate(driverDown, "driverDown", 140, NULL, 3, NULL);
			xTaskCreate(limitSwitchUp, "limitUp", 140, NULL, 3, NULL);
			xTaskCreate(limitSwitchDown,"limitDown",140,NULL,3,NULL);
			xTaskCreate(EmergencySwitch,"Emergency",140,NULL,4,NULL);
			xTaskCreate(lockSwitch, "lock", 140, NULL, 4, NULL);
			/* Create the task that will periodically generate a software interrupt.
			This is created with a priority below the handler task to ensure it will
			get preempted each time the handler task exits the Blocked state. */
			xTaskCreate( InitialTask, "Initial", 140, NULL, 1, NULL );
			/* Start the scheduler so the created tasks start executing. */
			vTaskStartScheduler();
		

    /* If all is well we will never reach here as the scheduler will now be
    running the tasks.  If we do reach here then it is likely that there was
    insufficient heap memory available for a resource to be created. */
    for( ;; );
}

/*--------------------------------------------------------------
										Port Initializations
----------------------------------------------------------------*/
//Initialize the hardware of Port-F
void PortF_Init(void){ 
	vSemaphoreCreateBinary(passenger_window_down_button);
  SYSCTL->RCGCGPIO |= 0x00000020;    // 1) F clock
  GPIOF->LOCK = 0x4C4F434B;  				 // 2) unlock PortF PF0  
  GPIOF->CR = 0x0F;          				 // allow changes to PF4-0       
  GPIOF->AMSEL= 0x00;       				 // 3) disable analog function
  GPIOF->PCTL = 0x00000000;  				 // 4) GPIO clear bit PCTL  
  GPIOF->DIR = 0x0E;         				 // 5) PF4,PF0 input, PF3,PF2,PF1 output   
  GPIOF->AFSEL = 0x00;      				 // 6) no alternate function
  GPIOF->PUR = 0x01;       				   // enable pullup resistors on PF4,PF0       
  GPIOF->DEN = 0x0F;       				   // 7) enable digital pins PF4-PF0
	GPIOF->DATA = 0x00;
	
	// Setup the interrupt on PortF
	GPIOF->ICR = 0x01;     // Clear any Previous Interrupt 
	GPIOF->IM |=0x01;     // Unmask the interrupts for PF0 and PF4
	GPIOF->IS |= 0x00;     // Make bits PF0 and PF4 level sensitive
	GPIOF->IEV &= ~0x01;
	GPIOF -> IBE |= 0x01;// Sense on Low Level
	NVIC_SetPriority(mainSW_INTURRUPT_PortF,5);
	NVIC_EnableIRQ(PortF_IRQn);       // Enable the Interrupt for PortF in NVIC
}
void PortA_Init(void)
{
	SYSCTL->RCGCGPIO |= 0x00000001;   
  GPIOA->LOCK = 0x4C4F434B;  			
  GPIOA->CR = 0x6C;          			      
  GPIOA->AMSEL= 0x00;       				 
  GPIOA->PCTL = 0x00000000;  				 
  GPIOA->DIR = 0x6C; 
  GPIOA->AFSEL = 0x00;      			
  GPIOA->DEN = 0x6C;       				  
	GPIOA->DATA = 0x00;
	
}
void PortD_Init(void)
{
	vSemaphoreCreateBinary(passenger_window_up_button);
	vSemaphoreCreateBinary(driver_window_up_button);
	vSemaphoreCreateBinary(driver_window_down_button);
	vSemaphoreCreateBinary(emergency_button);
	vSemaphoreCreateBinary(limit_switch_up);
	lockMutex = xSemaphoreCreateMutex();
	SYSCTL->RCGCGPIO |= 0x00000008;  
  GPIOD->LOCK = 0x4C4F434B;  				  
  GPIOD->CR = 0xCD;          				 
  GPIOD->AMSEL= 0x00;       				 
  GPIOD->PCTL = 0x00000000;  				 
  GPIOD->DIR =0x00;         				    
  GPIOD->AFSEL = 0x00;      				 
  GPIOD->PUR = 0xCD;       				        
  GPIOD->DEN = 0xCD;
	GPIOD->DATA = 0x00;
	
	GPIOD->ICR = 0xFF;
	GPIOD->IM |= 0xFD;	
	GPIOD->IS |= 0x02; //1100 0010
	GPIOD->IEV &= ~0xCD;  
	GPIOD->IBE |= 0xCD; //D0,D2,D3,D6,D7 1100 1101
	GPIOD->ICR = 0xFF;
	NVIC_SetPriority(mainSW_INTURRUPT_PortD,5);
	NVIC_EnableIRQ(mainSW_INTURRUPT_PortD);        
}

void PortB_Init(void) 
{
	vSemaphoreCreateBinary(on_off_switch_HIGH);
	vSemaphoreCreateBinary(on_off_switch_LOW);
	vSemaphoreCreateBinary(limit_switch_down);
	lockMutex = xSemaphoreCreateMutex();
	SYSCTL->RCGCGPIO |= 0x00000002;  
  GPIOB->LOCK = 0x4C4F434B;  				  
  GPIOB->CR = 0x1C;          				 
  GPIOB->AMSEL= 0x00;       				 
  GPIOB->PCTL = 0x00000000;  				 
  GPIOB->DIR =0x00;         				    
  GPIOB->AFSEL = 0x00;      				 
  GPIOB->PUR = 0x1C;       				        
  GPIOB->DEN = 0x1C;
	GPIOB->DATA = 0x00;
	
	GPIOB->ICR = 0x1C; 
	GPIOB->IS &= ~(0x1C); //Check tmrw 
	GPIOB->IEV &= ~0xEF;  
	GPIOB->IBE |= 0x1C;
	GPIOB->IM |= 0x1C;
	//GPIOB->ICR = 0x1C;
	NVIC_SetPriority(mainSW_INTURRUPT_PortB,5);
	NVIC_EnableIRQ(mainSW_INTURRUPT_PortB);         
}
/*--------------------------------------------------------------
										Interrupt Handlers
----------------------------------------------------------------*/
//Port-F handler
void GPIOF_Handler(void){

	uint32_t i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if((GPIOF->RIS & (0x01))== 1)
	{
		for(uint32_t j = 0;j < 300000;j++);
		xSemaphoreGiveFromISR(passenger_window_down_button,&xHigherPriorityTaskWoken);//Passenger Down
	}
	
	GPIOF->ICR = 0x11;        // clear the interrupt flag of PORTF
  i= GPIOF->ICR ;           // Reading the register to force the flag to be cleared
}

void GPIOD_Handler(void){

	uint32_t i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if((GPIOD->MIS & (0x01)) == 0x01)
	{		
		for(uint32_t j = 0;j < 300000;j++);
		xSemaphoreGiveFromISR(passenger_window_up_button,&xHigherPriorityTaskWoken); //Passenger up
	}
	else if(((GPIOD->MIS & (0x80)) == 0x80))
	{
		xSemaphoreGiveFromISR(limit_switch_up,&xHigherPriorityTaskWoken); //Limit_switch_up
	}
	else if((GPIOD->MIS & (0x04)) == 0x04)
	{		
		for(uint32_t j = 0;j < 300000;j++);
		xSemaphoreGiveFromISR(driver_window_up_button,&xHigherPriorityTaskWoken); //Driver Up
	}
	else if((GPIOD->MIS & (0x08)) == 0x08)
	{		
		for(uint32_t j = 0;j < 300000;j++);
		xSemaphoreGiveFromISR(driver_window_down_button,&xHigherPriorityTaskWoken); //Driver Down
	}
	else if((GPIOD->MIS & (0x40))== 0x40)
	{		
		xSemaphoreGiveFromISR(emergency_button,&xHigherPriorityTaskWoken); //Emergency
	}
	GPIOD->ICR = 0xFF;        // clear the interrupt flag of PORTF
  i= GPIOD->ICR ;           // Reading the register to force the flag to be cleared

	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
void GPIOB_Handler(){
	uint32_t i;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if((GPIOB->RIS & (0x08))== 0x08)
	{		
		xSemaphoreGiveFromISR(on_off_switch_LOW,&xHigherPriorityTaskWoken);//Lock switch
	}else if((GPIOB -> RIS & (0x10)) == 0x10){
		xSemaphoreGiveFromISR(limit_switch_down,&xHigherPriorityTaskWoken);//Limit_switch_down
	}
	
	GPIOB->ICR = 0xFF;        // clear the interrupt flag of PORTF
  i= GPIOB->ICR ;           // Reading the register to force the flag to be cleared

	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	
}