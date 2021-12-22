
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"


PUBLIC	PROCESS			proc_table[NR_TASKS];
PUBLIC  PROCESS* ptr_schedule = proc_table;

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

PUBLIC	TASK	task_table[NR_TASKS] = {{ReaderA, STACK_SIZE_TESTA, "ReaderA"},
					{ReaderB, STACK_SIZE_TESTB, "ReaderB"},
					{ReaderC, STACK_SIZE_TESTC, "ReaderC"},
					{WriterD, STACK_SIZE_TESTD, "WriterD"},
                    {WriterE, STACK_SIZE_TESTE, "WriterE"},
                    {NormalF, STACK_SIZE_TESTF, "NormalF"}};

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks,sys_mills_sleep,sys_my_print,sys_P,sys_V};

