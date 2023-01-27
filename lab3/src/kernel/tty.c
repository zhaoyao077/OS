
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

static  void init_tty(TTY* p_tty);
static  void tty_do_read(TTY* p_tty);
static  void tty_do_write(TTY* p_tty);
static  void put_key(TTY* p_tty, u32 key);

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}


/*======================================================================*
				      ³õÊ¼»¯ËùÓÐttyµÄscreen
 *======================================================================*/

PUBLIC void init_all_screen() {
	TTY* p_tty;
	for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++)
	{
		init_screen(p_tty);
	}
	select_console(0);
}

/*======================================================================*
			   init_tty
 *======================================================================*/
static  void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
        char output[2] = {'\0', '\0'};
        /**
         * ??mode?????????????
         */
		if (mode == 2)
		{
			if ((key & MASK_RAW) == ESC)
			{
				mode = 0;
				exit_esc(p_tty->p_console);
			}
			return;
		}
        if (!(key & FLAG_EXT)) {
		put_key(p_tty, key);
        }
        else {
			int raw_code = key & MASK_RAW;
            switch(raw_code) {
                /**
                 * ??ESC?????????/????
                 */
				case ESC:
					if (mode == 0) {
						mode = 1;
						p_tty->p_console->start_cursor = p_tty->p_console->cursor;
						p_tty->p_console->pos_stk.start = p_tty->p_console->pos_stk.ptr;
					}
					else {
					    // ???????
						mode = 0;
					}
					break;
                case ENTER:
					if (mode == 0) {
						put_key(p_tty, '\n');
					}
					else{
						search(p_tty->p_console);
						// ???????ESC??????
						mode = 2;
					}
					break;
                case BACKSPACE:
					put_key(p_tty, '\b');
					break;
				case TAB:
					put_key(p_tty, '\t');
					break;
                case UP:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_DN);
                        }
					break;
				case DOWN:
					if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
						scroll_screen(p_tty->p_console, SCR_UP);
					}
					break;
				case F1:
				case F2:
				case F3:
				case F4:
				case F5:
				case F6:
				case F7:
				case F8:
				case F9:
				case F10:
				case F11:
				case F12:
					/* Alt + F1~F12 */
					if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
						select_console(raw_code - F1);
					}
					break;
                default:
                        break;
                }
        }
}

/*======================================================================*
			      put_key
*======================================================================*/
static  void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
static  void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


//======================================================================
//			      tty_do_write
 //======================================================================
 static void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}


