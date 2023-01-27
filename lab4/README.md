# OS-lab4

## 实验内容

【读者写者问题】

- 现有6个⼀直存在的进程执⾏循环读（写）操作，其中A是普通进程，B、C、D是读者进程，E、F是写者进程。
- B阅读消耗2个时间片，C、D阅读消耗3个时间片。
- E写消耗3个时间片，F写消耗4个时间片。

- 允许n个读者同时读⼀本书，有读者时写者不能写

- 只允许1个写者写，此时读者不能读。

- 读（写）完后休息t个时间片。



## 代码设计

- 读写策略的修改

  ```c
  PRIVATE void init_tasks()
  {
  	init_screen(tty_table);
  	clean(console_table);
  
  	int prior[7] = {1, 1, 1, 1, 1, 1, 1};
  	for (int i = 0; i < 7; ++i) {
          proc_table[i].ticks    = prior[i];
          proc_table[i].priority = prior[i];
  	}
  
  	// initialization
  	k_reenter = 0;
  	ticks = 0;
  	readers = 0;
  	writers = 0;
  	writing = 0;
  
  
  	strategy = 0; //读写公平0、读优先1、写优先2
  
  	p_proc_ready = proc_table;
  }
  ```

- 最多同时读的数量修改

  ```c
  /* system call */
  #define NR_SYS_CALL     5
  
  #define TIME_SLICE      1000
  #define MAX_READERS     3 //最多同时读的数量
  
  #define BRIGHT_RED      0x0C
  #define BRIGHT_GREEN    0x0A
  #define BRIGHT_BLUE    	0x09
  ```

- P操作

  ```c
  /*======================================================================*
                                  P操作
   *======================================================================*/
  PUBLIC void p_process(SEMAPHORE* s){
  	//屏蔽中断
  	disable_int();
  
      s->value--;
      if (s->value < 0){//申请资源失败，进入阻塞队列
          p_proc_ready->blocked = 1;
          s->p_list[s->tail] = p_proc_ready;
          s->tail = (s->tail + 1) % NR_PROCS;
          schedule();
      }
  	
  	//开启中断
  	enable_int();
  }
  ```

- V操作

  ```c
  
  /*======================================================================*
                                  V操作
   *======================================================================*/
  PUBLIC void v_process(SEMAPHORE* s){
  	//屏蔽中断
  	disable_int();
  
      s->value++;
      if (s->value <= 0){//存在阻塞进程，将其唤醒
          s->p_list[s->head]->blocked = 0;//唤醒队头进程
          s->head = (s->head + 1) % NR_PROCS;
      }
  	
  	//开启中断	
  	enable_int();
  }
  ```

- 打印输出

  ```c
  void A()
  {
      char color = '\06';
      int time = 0;
      while (1) {
  				if(time < 20){
  					if(time == 19){
  						printf("%c%c%c ",'\06', '2','0');
  					}else if(time >= 9 && time < 19){
  						printf("%c%c%c ",'\06', '1','0' + time - 9);
  					}else if(time < 9){
  						printf("%c%c  ", '\06', '1' + time);
  					}
  					time++;
  
  					for(int table_ptr = 0; table_ptr < 5; table_ptr++){
  						if(print_table[table_ptr] == 'Z'){
  							printf("%c%c ", '\03', print_table[table_ptr]);
  						} else if(print_table[table_ptr] == 'O'){
  							printf("%c%c ", '\02', print_table[table_ptr]);
  						} else if(print_table[table_ptr] == 'X'){
  							printf("%c%c ", '\01', print_table[table_ptr]);
  						}
  					}
  					printf("\n");
  				}
  
          sleep_ms(TIME_SLICE);
      }
  }
  ```

  