
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_
#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */
#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80
#define TAB_WIDTH 4
#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */

typedef struct i_stack{
    unsigned int idx;//当前的下标
    unsigned int len;//数组的长度
    unsigned int array[SCREEN_SIZE];//储存用数组
}STACK;
typedef struct c_stack{
    int idx;//当前的下标
    int seperator;//记录/r的下标
    char ch[SCREEN_SIZE];//储存每一步的字符
}CHARSTACK;
/* CONSOLE */
typedef struct s_console
{
    unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
    unsigned int	original_addr;		/* 当前控制台对应显存位置 */
    unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
    unsigned int	cursor;			/* 当前光标位置 */
    STACK *cursorStack; //用于记录每一个光标位置
    CHARSTACK charStack;//用以记录每一步进行了什么操作
    unsigned int    endOfNormalCursor;//记录进入查找模式前的光标位置
}CONSOLE;

#endif /* _ORANGES_CONSOLE_H_ */
