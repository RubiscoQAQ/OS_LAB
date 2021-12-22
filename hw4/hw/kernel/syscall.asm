
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！、
_NR_mills_sleep     equ 1
_NR_my_print        equ 2
_NR_P               equ 3
_NR_V               equ 4

;设置系统调用
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global  mills_sleep
global  my_print
global  P
global  V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

mills_sleep:
	mov	eax, _NR_mills_sleep
	mov ecx, [esp+4];由于需要参数
	int	INT_VECTOR_SYS_CALL
	ret

my_print:
    mov	eax, _NR_my_print
    mov ecx, [esp+4];由于需要参数
    int	INT_VECTOR_SYS_CALL
    ret

P:
	mov eax,_NR_P
	mov ecx,[esp+4];
	int INT_VECTOR_SYS_CALL
	ret

V:
	mov eax,_NR_V
	mov ecx,[esp+4]
	int INT_VECTOR_SYS_CALL
	ret
