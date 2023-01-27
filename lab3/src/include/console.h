
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_

/**
 * 记忆化栈：用来存储光标走过的每一个位置
 */
typedef struct stack{
	int ptr;
	int pos[10000];
	int start;   /* esc模式开始时，ptr的位置 */
}STK;

/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
	unsigned int	start_cursor;			/* esc模式开始时，ptr的位置 */
    STK pos_stk;					/* 记忆化栈 */
}CONSOLE;

PUBLIC void push(CONSOLE* p_con, unsigned int pos) {
    p_con->pos_stk.pos[p_con->pos_stk.ptr++] = pos;
}


PUBLIC unsigned int pop1(CONSOLE* p_con) {
    if (p_con->pos_stk.ptr == 0) return 0;
    else {
        (p_con->pos_stk.ptr)--;
        return p_con->pos_stk.pos[p_con->pos_stk.ptr];
    }
}

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define _CHAR_COLOR  0x0e	/* 0000 0010 黑底绿字 */
#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */
#define RED_CHAR_COLOR	0x04	/* 0000 0100 黑底红字 */
#define GREEN_CHAR_COLOR  0x02	/* 0000 0010 黑底绿字 */
#define BLUE_CHAR_COLOR	0x01	/* 0000 0001 黑底蓝字 */
#define TAB_WIDTH  4        /*TAB的长度是4*/

#endif /* _ORANGES_CONSOLE_H_ */
