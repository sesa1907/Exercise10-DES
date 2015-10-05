#include <stdio.h>
#include <stdlib.h> 
#include <sys/mman.h> 
#include <native/task.h>
#include <native/timer.h>

#define TASK_PERIOD 100000
#define ITER 10000
#define BASEPERIOD 0

RT_TASK periodTask;
RTIME global_time [ITER];
RTIME diff_time [ITER];
RTIME now;

static RT_INTR port_interr;

RT_TASK irq_server_task;

int isr_installation(void) {

  rt_intr_create(&port_interr,"port interrupt",7,I_PROPAGATE);
  // 7 is for port interrupt
  
  //rt_intr_enable(&port_interr);

  return 0; 
}
void write_RTIMES(char* filename, unsigned int number_of_values, RTIME* diff_time){
	printf("> writing...\n");
         unsigned int n=0;
         FILE *file;
         file = fopen(filename,"w");
         while (n<number_of_values) {
			 printf("> %u,%llu\n", n, diff_time[n]);
              fprintf(file,
			   "%u;%llu\n",
			   n,
			   diff_time[n]);  
              n++;
         }
         fclose(file);
		 
 }
void irq_server (void *cookie)
{
     static int interrupts = 0;
     int nr_interrupts;
	 rt_task_set_periodic(NULL, TM_NOW, TASK_PERIOD);
  for (i=0; i < ITER; i++) {
		global_time[i] = rt_timer_read();
        nr_interrupts = rt_intr_wait(&port_interr,TM_INFINITE);
        interrupts++;
        rt_printf("Iteration number %d, diff %d\n", i, (i > 0) ? global_time[i] - global_time[i-1] : 0);
		rt_task_wait_period(NULL);
		diff_time[i]	= (i > 0) ? global_time[i] - global_time[i-1] : 0;
  }
  write_RTIMES("time_diff_10d.csv",ITER,diff_time);
}

/*
void periodic_task (void *arg) {
	int i, write_time_diff;
	rt_task_set_periodic(NULL, TM_NOW, TASK_PERIOD); 
  	for(i=0; i < ITER; i++) {
		global_time[i] = rt_timer_read();
		printf("Iteration number %d, diff %d\n", i, (i > 0) ? global_time[i] - global_time[i-1] : 0);
		rt_task_wait_period(NULL);
		diff_time[i]	= (i > 0) ? global_time[i] - global_time[i-1] : 0;
	  }
	
  write_RTIMES("time_diff_10d.csv",ITER,diff_time);
}*/

void startup()
{
  rt_timer_set_mode(BASEPERIOD);
  /*
  rt_task_create(&periodTask, "period_task", 0, 0, 0);
  rt_task_start(&periodTask, &periodic_task, NULL);
  
  rt_printf("Your csv file is ready");
  */
  int retval;
  unsigned char byte;
  
  rt_printf("\nStart\n");

  retval=isr_installation();
  
	
  ioperm(0x37A,1,1);
  byte = inb(0x37A);
  byte = byte | 0x10;
  outb(inb(0x378) | 0x01, 0x378);
  //outb(byte, 0x37A);
  
  rt_task_create(&irq_server_task, "irq server task", 0, 0, 0);
  rt_task_start(&irq_server_task, &irq_server, NULL);
  
  byte = inb(0x37A);
  byte = byte & 0xEF;
  //outb(byte, 0x37A);
  outb(inb(0x378) & 0xfe, 0x378);
  
}

void init_xenomai() {
  mlockall(MCL_CURRENT|MCL_FUTURE);

  rt_print_auto_init(1);
}

int main (int argc, char *argv[]) 
{
  init_xenomai();
  startup();
  pause();
}
