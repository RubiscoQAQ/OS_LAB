
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		ticks;

EXTERN	int		disp_pos;
EXTERN	u8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8		idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];
PUBLIC  PROCESS* ptr_schedule;
EXTERN	u32		k_reenter;

EXTERN	TSS		tss;
EXTERN	PROCESS*	p_proc_ready;

extern	PROCESS		proc_table[];
extern	char		task_stack[];
extern  TASK            task_table[];
extern	irq_handler	irq_table[];

// 新增Semaphore部分
typedef struct semaphore{
    int value;
    PROCESS* queue[NR_TASKS];
}Semaphore;


EXTERN Semaphore readMutex;
EXTERN Semaphore writeMutex;
EXTERN Semaphore readCountMutex;
EXTERN Semaphore writeCountMutex;
EXTERN Semaphore readPermission;
EXTERN Semaphore readPermissionMutex;
EXTERN Semaphore countMutex;
EXTERN Semaphore writeMutexMutex;
EXTERN int readNum;     // 允许同时读的个数
EXTERN int writeNum;    // 允许同时写的个数，默认为1
EXTERN int readPreparedCount;
EXTERN int readCount;   // 正在读的个数
EXTERN int solveHunger; // 是否解决饿死
EXTERN int writeCount;   // 正在写的个数
EXTERN int waiting;
EXTERN int nowStatus;
EXTERN int isReaderFirst;

EXTERN char writeStr[];
EXTERN char readStr[];
EXTERN char endStr[];