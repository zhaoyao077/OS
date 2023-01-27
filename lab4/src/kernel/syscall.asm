
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_write_str		equ 1
_NR_sleep_ms		equ	2
_NR_P				equ	3
_NR_V				equ	4
INT_VECTOR_SYS_CALL equ 0x90

; 导出符号
global	get_ticks
global	write_str
global	sleep_ms
global	P
global	V

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	jmp	sys_call_block

write_str:
	mov	eax, _NR_write_str
	mov	ebx, [esp+4]	;参数1
	mov	ecx, [esp+8]	;参数2
	jmp	sys_call_block

sleep_ms:
	mov	eax, _NR_sleep_ms
	mov	ebx, [esp+4]
	jmp	sys_call_block

P:
	mov	eax, _NR_P  ; 对应系统调用p_process
	mov	ebx, [esp+4]
	int	INT_VECTOR_SYS_CALL
	ret

V:
	mov	eax, _NR_V ; 对应系统调用v_process
	mov	ebx, [esp+4]
	int	INT_VECTOR_SYS_CALL
	ret

sys_call_block:
  int     INT_VECTOR_SYS_CALL
  ret