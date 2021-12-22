
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	isAllDone();//如果所有进程都完成了，就重启
    PROCESS* p = proc_table+5;//F进程
    if(isRunnable(p)){
        p_proc_ready = p;
    }
    else{
        while(!isRunnable(ptr_schedule)){
            ptr_schedule++;
            if(ptr_schedule==p){
                ptr_schedule = proc_table;
            }
        }
        p_proc_ready = ptr_schedule;
        ptr_schedule++;
        if(ptr_schedule==p){
            ptr_schedule = proc_table;
        }
    }
    if(p_proc_ready-proc_table<=4){
        nowStatus = p_proc_ready-proc_table;
    }
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}
/*======================================================================*
                           sys_mills_sleep
 *======================================================================*/
PUBLIC void sys_mills_sleep(int milli_seconds){
    p_proc_ready->wake_tick = get_ticks() + milli_seconds / (1000 / HZ);
    schedule();
}

PUBLIC int isRunnable(PROCESS* p){
    if(p->wake_tick <= get_ticks()&&p->isBlock == 0&&p->isDone ==0){
        return 1;
    }else{
        return 0;
    }
}
/*======================================================================*
                           sys_my_print
 *======================================================================*/
PUBLIC void sys_my_print(char* str){
    if (disp_pos > 80 * 25 * 2){
        //disp_pos = 0;
        return;
    }
    //这是因为输出超过显存会报错，当前环境下还不能提供超过显存的输出，这是有待解决的问题
    switch(p_proc_ready - proc_table){
        case 0:
            disp_color_str(str, BRIGHT | MAKE_COLOR(BLACK, RED));
            break;
        case 1:
            disp_color_str(str, BRIGHT | MAKE_COLOR(BLACK, GREEN));
            break;
        case 2:
            disp_color_str(str, BRIGHT | MAKE_COLOR(BLACK, BLUE));
            break;
        case 5:
            disp_str(str);
            break;
        case 4:
            disp_color_str(str, BRIGHT | MAKE_COLOR(BLACK, PURPLE));
            break;
        case 3:
            disp_color_str(str, BRIGHT | MAKE_COLOR(BLACK, YELLO));
            break;
        default:
            disp_str(str);
            break;
    }
}
/*======================================================================*
                           sys_P
 *======================================================================*/
PUBLIC void sys_P(void *mutex){
    disable_irq(CLOCK_IRQ);//PV原语要求不能被中断
    Semaphore *semaphore = (Semaphore *)mutex;
    semaphore->value--;
    if (semaphore->value < 0){
        block(semaphore);
        //如果小于0，就要陷入阻塞
    }
    enable_irq(CLOCK_IRQ);
}

PUBLIC void block(Semaphore *mutex){
    mutex->queue[-mutex->value - 1] = p_proc_ready;
    //-mutex->value - 1正好保证了任务按顺序进入队列
    p_proc_ready->isBlock = 1; // 阻塞
    schedule();
}
/*======================================================================*
                           sys_V
 *======================================================================*/
PUBLIC void sys_V(void *mutex){
    disable_irq(CLOCK_IRQ);//PV原语要求不能被中断
    Semaphore *semaphore = (Semaphore *)mutex;
    semaphore->value++;
    if (semaphore->value <= 0){
        wake(semaphore);
        //如果小于0，就要唤醒一个被阻塞的进程
    }
    enable_irq(CLOCK_IRQ);
}

PUBLIC void wake(Semaphore *mutex){
    mutex->queue[0]->isBlock = 0;
    for(int i=0;i<(-mutex->value);i++){
        mutex->queue[i] = mutex->queue[i+1];
    }
}

PUBLIC void isAllDone(){
    PROCESS *p;
    int allDone = 1;
    for (p = proc_table; p < proc_table + NR_TASKS-1; p++){
        //-1是为了不要F进程
        if(p->isDone==0){
            allDone = 0;
            return;
        }
    }
    if(allDone==1){
        //如果全部做完了，任务重启
        for (p = proc_table; p < proc_table + NR_TASKS-1; p++){
            p->isDone = 0;
        }
        disp_str("<RESTART> ");
    }
}
PUBLIC void my_print_int(int i){
    if(i==0){
        my_print("0");
        return;
    }
    int len = 0;
    int copy = i;
    int temp = 0;
    while(copy!=0){
        copy/=10;
        len++;
    }
    char res[len+1];
    res[len] = '\0';
    while(i!=0){
        temp = i%10;
        res[len-1] = temp-0+'0';
        len--;
        i/=10;
    }
    my_print(res);
}