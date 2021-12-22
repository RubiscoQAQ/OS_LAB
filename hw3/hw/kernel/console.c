
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PRIVATE void push(CONSOLE* p_con,unsigned int pos);
PRIVATE unsigned int pop(CONSOLE* p_con);
PRIVATE void searchString(CONSOLE *p_con);
/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;
    p_tty->p_console->endOfNormalCursor = p_tty->p_console->cursor;
    p_tty->p_console->cursorStack->idx = 0;
    p_tty->p_console->charStack.idx = 0;
    p_tty->p_console->charStack.seperator = 0;
    for(int i=0;i<SCREEN_SIZE;i++){
        p_tty->p_console->charStack.ch[i]=' ';
    }
    p_tty->p_console->cursorStack->len = SCREEN_SIZE;
	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    if(mode==2){
        if(ch=='\r'){
            mode = 0;
            p_con->charStack.idx = p_con->charStack.seperator;
        } else{
            return;
        }
    }
	switch(ch) {
	case '\n':
	    if(mode==0){
            if (p_con->cursor < p_con->original_addr +
                                p_con->v_mem_limit - SCREEN_WIDTH) {
                push(p_con,p_con->cursor);
                p_con->cursor = p_con->original_addr + SCREEN_WIDTH *
                                                       ((p_con->cursor - p_con->original_addr) /
                                                        SCREEN_WIDTH + 1);
            }
	    }
		else{
		    //处理匹配
		    mode = 2;
            searchString(p_con);
		}
		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr&&p_con->cursorStack->idx!=0) {
		    unsigned int idx = pop(p_con);
            if(mode==1){
                if(idx<p_con->endOfNormalCursor){
                    push(p_con,idx);
                    break;
                }
                //这里是为了让在查找模式中不要删除掉原文，一旦要删除的地方在原文范围内，就再push进去
            }
		    if(idx!=(p_con->cursorStack->len+1)){
		        int i=0;
                while(p_con->cursor>idx){
                    p_con->cursor--;
                    *(p_vmem-2-2*i) = ' ';
                    *(p_vmem-1-2*i) = DEFAULT_CHAR_COLOR;
                    i++;
                }
		    }
		}
		break;
    case '\t':
        if(p_con->cursor < p_con->original_addr + p_con->v_mem_limit - TAB_WIDTH){
            push(p_con,p_con->cursor);
            for(int i=0;i<TAB_WIDTH;i++){
                *p_vmem++ = ' ';
                *p_vmem++ = BLUE;
                p_con->cursor++;
            }
        }
        break;
    case '\r':
        //作为ESC的代表字符
        set_charStackSeperator(p_con);
        if(mode==1){
            p_con->endOfNormalCursor = p_con->cursor;
        } else{
            if (p_con->cursor > p_con->original_addr){
                int i=0;
                while(p_con->cursor>p_con->endOfNormalCursor){
                    unsigned int idx = pop(p_con);
                    //将光标栈出栈到合适的位置
                    while(p_con->cursor>idx){
                        p_con->cursor--;
                        *(p_vmem-2-2*i) = ' ';
                        *(p_vmem-1-2*i) = DEFAULT_CHAR_COLOR;
                        i++;
                    }
                }
            }
            for(int i=0;i<p_con->cursor;i++){
                if(*(u8*)(V_MEM_BASE + i * 2+1)==RED){
                    *(u8*)(V_MEM_BASE + i * 2+1)=DEFAULT_CHAR_COLOR;
                }
            }
        }
        break;
    case 'Z':
    case 'z':
        if(ctrl){
            doCtrlZ(p_con);
            return;
        }
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
            push(p_con,p_con->cursor); //在操作之前保存，可以在需要时候pop出来
            if(mode==0&&p_con->cursor>p_con->endOfNormalCursor){
                p_con->endOfNormalCursor = p_con->cursor;
            }
			*p_vmem++ = ch;
			if(mode==0||ch==' '){
                *p_vmem++ = DEFAULT_CHAR_COLOR;
			} else{
                *p_vmem++ = RED;
			}
			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

/*======================================================================*
			   对光标位置记录栈的操作
 *======================================================================*/
PRIVATE void push(CONSOLE* p_con,unsigned int pos){
    if(p_con->cursorStack->idx<p_con->cursorStack->len){
        p_con->cursorStack->array[p_con->cursorStack->idx] = pos;
        p_con->cursorStack->idx++;
    } else{
        disp_str("stackOverFlow");
    }
}
PRIVATE unsigned int pop(CONSOLE* p_con){
    if(p_con->cursorStack->idx-1>=0){
        unsigned int res = p_con->cursorStack->array[p_con->cursorStack->idx-1];
        p_con->cursorStack->idx--;
        return res;
    } else{
        disp_str("stackOverFlow");
        return p_con->cursorStack->len+1;
    }
}
/*======================================================================*
			   对字符串的匹配
 *======================================================================*/
PRIVATE void searchString(CONSOLE *p_con){
    int len = p_con->cursor-p_con->endOfNormalCursor;//待匹配字符串长度
    if(len==0){
        return;
    }
    u8* p_vmem;
    u8* tar_vmem;
    u8* p_color;
    u8* tar_color;
    for(int i=0;i<p_con->endOfNormalCursor;i++){
        int found = 1;
        for(int j=0;j<len;j++){
            p_vmem = (u8*)(V_MEM_BASE + i * 2 + j*2);
            p_color = (u8*)(V_MEM_BASE + i * 2 + j*2+1);
            tar_vmem = (u8*)(V_MEM_BASE + p_con->endOfNormalCursor * 2 + j*2);
            tar_color = (u8*)(V_MEM_BASE + p_con->endOfNormalCursor * 2 + j*2+1);
            if(*p_vmem!=*tar_vmem||(*p_vmem==' '&&*p_color!=*tar_color)){
                found = 0;
                break;
            }
        }
        if(len>p_con->endOfNormalCursor){
            found = 0;
            //排除匹配字符比原文长的情况
        }
        if(found==1){
            for(int k=0;k<len;k++){
                if(*(u8*)(V_MEM_BASE + i * 2+k*2)!=' ')
                *(u8*)(V_MEM_BASE + i * 2+k*2+1)=RED;
            }
        }
    }
}
/*======================================================================*
			   撤销操作
 *======================================================================*/
PUBLIC void doCtrlZ(CONSOLE *p_con){
    if(mode==0){
        cleanScreen();
        p_con->cursorStack->idx=0;
        p_con->cursor = disp_pos / 2;
        //初始化指针
        flush(p_con);
        redo(p_con);
    }
    if(mode==1){
        p_con->cursorStack->idx=p_con->charStack.seperator;
        disp_pos = p_con->endOfNormalCursor * 2;
        for (int i = 0 ; i < SCREEN_SIZE; ++i){
            disp_str(" ");
        }
        disp_pos = p_con->endOfNormalCursor * 2;
        p_con->cursor = disp_pos / 2;
        //初始化指针
        flush(p_con);
        redo(p_con);
    }
}
PUBLIC void redo(CONSOLE *p_con){
    int start = 0;
    if(mode==1){
        start = p_con->charStack.seperator;
    }
    p_con->charStack.idx-=2;
    if(p_con->charStack.idx<=0){
        p_con->charStack.idx=0;
        return;
        //已经清空
    }
    for(int i=start;i<p_con->charStack.idx;i++){
        out_char(p_con,p_con->charStack.ch[i]);
    }
}
PUBLIC void set_charStackSeperator(CONSOLE* p_con){
    if(mode==1){
        p_con->charStack.seperator=p_con->charStack.idx;
    }
}