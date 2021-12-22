
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");
    disp_pos = 0;
    for (int i = 0; i < 80 * 25; i++)
    {
        disp_str(" ");
    }
    disp_pos = 0;
	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	//定义在protect.h中
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */
        //初始化进程表，进程表定义在proc.h中
		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}
    //设置需要的时间片
	proc_table[0].ticks = proc_table[0].priority = 2;
	proc_table[1].ticks = proc_table[1].priority = 3;
	proc_table[2].ticks = proc_table[2].priority = 3;
    proc_table[3].ticks = proc_table[3].priority = 3;
    proc_table[4].ticks = proc_table[4].priority = 4;
    proc_table[5].ticks = proc_table[5].priority = 1;

    k_reenter = 0;
	ticks = 0;
    /*初始化信号量相关*/
    readNum = 1;
    readMutex.value = readNum;
    writeNum = 1;
    writeMutex.value = writeNum;
    nowStatus = 5;


    isReaderFirst = 0;
    //下面是readFirst相关设置
    countMutex.value = 1;
    writeMutexMutex.value = 1;
    //下面是writeFirst相关设置
    readCountMutex.value = 1;
    writeCountMutex.value = 1;
    readPermission.value = 1;
    readPermissionMutex.value = 1;
    readPreparedCount = 0;
    writeCount = 0;
    // 是否需要解决饿死
    solveHunger = 0;
	p_proc_ready	= proc_table;

        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	restart();

	while(1){}
}

/*======================================================================*
                               ReaderA
 *======================================================================*/
void ReaderA()
{
    reader('A');
}

/*======================================================================*
                               ReaderB
 *======================================================================*/
void ReaderB()
{
    reader('B');
}

/*======================================================================*
                               ReaderC
 *======================================================================*/
void ReaderC()
{
    reader('C');
}
void WriterD()
{
    writer('D');
}
void WriterE()
{
    writer('E');
}
void NormalF()
{
    while (1)
    {
        if (nowStatus <= 2)
        {
            my_print("<read==");
            char num = '0' + readCount - 0;
            char tem[4] = {num, '>', ' ', '\0'};
            my_print(tem);
        }
        else if (nowStatus <= 4)
        {
            my_print("<writing> ");
        }
        else
        {
            my_print("<<START>> ");
        }
        mills_sleep(10);
    }
}
void reader(char processName){
    char writeStr[] = " writing.";
    char readStr[] = " reading.";
    char endStr[] = " end.    ";
    char pname[2] = {processName, '\0'};
    if(isReaderFirst){
        mills_sleep(10);
        while(1){
            // 同时只有一个进程可以修改在读人数
            P(&countMutex);
            if (readPreparedCount == 0)
            {
                //夺取写的权限
                P(&writeMutex);
            }
            readPreparedCount++;
            V(&countMutex);

            P(&readMutex);
            //限制同时读的人数
            readCount++;
            my_print(pname);
            my_print(" start.  ");
            int j;
            //用于输出运行信息
            for (j = 0; j < p_proc_ready->priority; ++j)
            {
                my_print(pname);
                my_print(readStr);
                if (j == p_proc_ready->priority - 1)
                {
                    my_print(pname);
                    my_print(endStr);
                }
                else
                {
                    milli_delay(10);
                }
            }
            readCount--;
            V(&readMutex);

            P(&countMutex);
            // 同时只有一个进程可以修改在读人数
            readPreparedCount--;
            if (readPreparedCount == 0)
            {
                V(&writeMutex);
            }
            V(&countMutex);

            p_proc_ready->isDone = solveHunger;
            milli_delay(10); // 废弃当前时间片，至少等到下个时间片才能进入循环
        }
    } else{
        mills_sleep(10);
        while (1)
        {
            P(&readPermissionMutex); // 保证只有一个被卡在readPermission
            P(&readPermission);
            // 判断修改在读人数
            P(&readCountMutex);
            if (readPreparedCount == 0)
            {
                P(&writeMutex);
                //夺取写权限，或等待写完
            }
            readPreparedCount++;
            V(&readCountMutex);
            V(&readPermission);
            V(&readPermissionMutex);//保证优先写

            P(&readMutex);
            readCount++;
            my_print(pname);
            my_print(" start.  ");
            int j;
            for (j = 0; j < p_proc_ready->priority; ++j)
            {
                my_print(pname);
                my_print(readStr);
                if (j == p_proc_ready->priority- 1)
                {
                    my_print(pname);
                    my_print(endStr);
                }
                else
                {
                    milli_delay(10);
                }
            }
            readCount--;
            V(&readMutex);

            P(&readCountMutex);
            readPreparedCount--;
            if (readPreparedCount == 0)
            {
                V(&writeMutex);
            }
            V(&readCountMutex);

            p_proc_ready->isDone = solveHunger;
            milli_delay(10);
        }
    }
}
void writer(char processName){
    char pname[2] = {processName, '\0'};
    char writeStr[] = " writing.";
    char readStr[] = " reading.";
    char endStr[] = " end.    ";
    if(isReaderFirst){
        while (1){
            P(&writeMutexMutex); // 只允许一个写者进程在writeMutex排队，其他写者进程只能在writeMutexMutex排队
            P(&writeMutex);
            my_print(pname);
            my_print(" start.  ");
            int j;
            for (j = 0; j < p_proc_ready->priority; ++j)
            {
                my_print(pname);
                my_print(writeStr);
                if (j == p_proc_ready->priority - 1)
                {
                    my_print(pname);
                    my_print(endStr);
                }
                else
                {
                    milli_delay(10);
                }
            }
            V(&writeMutex);
            V(&writeMutexMutex);

            p_proc_ready->isDone = solveHunger;
            milli_delay(10);
        }
    } else{
        mills_sleep(20);
        while (1)
        {
            P(&writeCountMutex);
            //同时只允许一个写进程改变写进程数目
            if(writeCount==0){
                P(&readPermission);
                //夺取读的权限，或者等待读完
            }
            writeCount++;
            V(&writeCountMutex);
            P(&writeMutex);
            my_print(pname);
            my_print(" start.  ");
            int j;
            for (j = 0; j < p_proc_ready->priority; ++j)
            {
                my_print(pname);
                my_print(writeStr);
                if (j == p_proc_ready->priority - 1)
                {
                    my_print(pname);
                    my_print(endStr);
                }
                else
                {
                    milli_delay(10);
                }
            }
            V(&writeMutex);

            P(&writeCountMutex);
            writeCount--;
            if (writeCount == 0)
            { // 没有写进程了，才释放读进程的权限
                V(&readPermission);
            }
            V(&writeCountMutex);

            p_proc_ready->isDone = solveHunger;
            milli_delay(10);
        }
    }
}