#include <stdint.h>
#include <stdio.h>
#include "reg.h"
#include "blink.h"
#include "usart.h"
#include "asm_func.h"

#define TASK_NUM 4 //initialize 3 tasks
#define PSTACK_SIZE_WORDS 1024 //user stack size = 4 kB (each word is 4 bytes)
# define DELAY_TIME 100 //Delay 100 ms

static uint32_t *psp_array[TASK_NUM]; //儲存指標的array


#define UNSIGN_INT_MAX 4294967295
long fiboLong;
uint32_t n_1;
uint32_t n_2;
uint32_t n_tmp;


static int n=0;
void setup_systick(uint32_t ticks);

void init_task(unsigned int task_id, uint32_t *task_addr, uint32_t *psp_init)
{
	//task id: 要初始化的task(0~2)
	//task_addr:
	//psp_init: 

	//? xPSR (bit 24, T bit, has to be 1 in Thumb state)
	*(psp_init-1) = UINT32_1 <<24;	  

	//? Return Address is being initialized to the task entry
	*(psp_init-2) = (uint32_t)task_addr;

	//?initialize psp_array (stack frame: 8 + r4 ~ r11: 8)
	psp_array[task_id] = psp_init-16;
}

//Task 0 function
void task0(void)
{
	printf("[Task0] Start in unprivileged thread mode.\r\n\n");
	printf("[Task0] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink(LED_BLUE); //should not return
}

void task1(void)
{
	printf("[Task1] Start in unprivileged thread mode.\r\n\n");
	printf("[Task1] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink(LED_GREEN); //should not return
}

void task2(void)
{
	printf("[Task2] Start in unprivileged thread mode.\r\n\n");
	printf("[Task2] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink(LED_ORANGE); //should not return
}

long getFib(uint32_t n){
	if (n==0)
		return 0;
	if (n==1)
		return 1;
	 return (getFib(n-1)+getFib(n-2));
}

long getFib2(uint32_t n_1, uint32_t n_2){
	 return (n_1+n_2);	
}
void task3(void){
	printf("[Task3] Started in unprevileged thread mode. \r\n\n");
	printf("[Task3] Control 0x%x \r\n", (unsigned int)read_ctrl());

	for (int i=0;;i++){
		
		//Determine which Fibonacci function to use
		if(n<3){
			fiboLong=getFib(i);
			n_1=1;
			n_2=1;
			n_tmp=0;
		}

		else //Calculate the Fibo with getFib2()
		{
			fiboLong=getFib2(n_1,n_2);
		}
		
		//Determine to print or to restart the counter
		if (fiboLong<=UNSIGN_INT_MAX){ //UNSIGN_INT_MAX
			//Unsign interger max
			printf("Fibo==%d\r\n",(unsigned int)fiboLong); //convert back to int
		}
		else{	
			i=0;
			continue;
		}
		n_tmp=n_2;
		n_1=n_tmp;
		n_2=(unsigned int)fiboLong;
		for(int j=0;j<100000;j++) //delay for a while
		;

	}


}

int main(void)
{
	init_usart1();

	//Initialize the user_stacks array
	uint32_t user_stacks[TASK_NUM][PSTACK_SIZE_WORDS];

	//init user tasks
	init_task(0, (uint32_t *)task0,user_stacks[0]+PSTACK_SIZE_WORDS);
	init_task(1, (uint32_t *)task1,user_stacks[1]+PSTACK_SIZE_WORDS);
	init_task(2, (uint32_t *)task2,user_stacks[2]+PSTACK_SIZE_WORDS);
	init_task(3, (uint32_t *)task3,user_stacks[3]+PSTACK_SIZE_WORDS);

	printf("[Kernel] Start in privileged thread mode.\r\n\n");

	printf("[Kernel] Setting systick...\r\n\n");
	//int32_t div=100;
	setup_systick(168e6 / 8 /(1000/DELAY_TIME)); //Change task every 10 ms
	/*
	/8  除頻器除頻倍率
	/100 =1000ms/10ms

	 */

	//start user task
	printf("[Kernel] Switch to unprivileged thread mode & start user task0 with psp.\r\n\n");
	start_user((uint32_t *)task0, user_stacks[0]);

	while (1) //should not go here
		;
}

void setup_systick(uint32_t ticks)
{
	// set reload value
	WRITE_BITS(SYST_BASE + SYST_RVR_OFFSET, SYST_RELOAD_23_BIT, SYST_RELOAD_0_BIT, ticks - 1);

	// uses external reference clock
	CLEAR_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_CLKSOURCE_BIT);

	// enable systick exception
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_TICKINT_BIT);

	// enable systick
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_ENABLE_BIT);
}


//Switch between tasks
uint32_t *sw_task(uint32_t *psp)
{
	static unsigned int curr_task_id = 0;

	psp_array[curr_task_id] = psp; //save current psp

	if (++curr_task_id > TASK_NUM - 1) //get next task id
		curr_task_id = 0;

	return psp_array[curr_task_id]; //?return next psp
}