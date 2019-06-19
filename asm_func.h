#ifndef ASM_FUNC_H
#define ASM_FUNC_H

#include <stdint.h>

//return the value of control
uint32_t read_ctrl(void); //Put control into r0 and bx lr

//start user task at task_addr in unprivileged mode and set sp to psp with value psp_init
void start_user(uint32_t *task_addr, uint32_t *psp_init); 

#endif