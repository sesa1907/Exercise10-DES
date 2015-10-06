#include <stdio.h>
#include <stdlib.h> 
#include <sys/mman.h> 
#include <native/task.h>
#include <native/timer.h>
#include <native/intr.h>
#include <rtdk.h>
#include <sys/io.h>

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
unsigned char byte;
	rt_intr_create(&port_interr,"port interrupt",7,I_PROPAGATE);
    ioperm(0x37A, 1, 1); //enable interrupt port
	byte = inb(0x37A);
	byte = byte | 0x10; /* hex 10 = binary 00010000 */
	outb(byte, 0x37A);

	ioperm(0x378, 1, 1); //enable interrupt port
	byte = inb(0x378);
	byte = byte | 0x01; /* hex 10 = binary 00010000 */
	outb(byte, 0x378);
  // 7 is for port interrupt

 // rt_intr_enable(&port_interr);

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
     //static int interrupts = 0;
     int nr_interrupts, i;
	 rt_task_set_periodic(NULL, TM_NOW, TASK_PERIOD);
  for (i=0; i < ITER; i++) {
		global_time[i] = rt_timer_read();
        //nr_interrupts = rt_intr_wait(&port_interr,TM_INFINITE);
        //interrupts++;
		outb(inb(0x378) & 0xFE, 0x378);
		outb(inb(0x378) | 0x01, 0x378); /* enable interrupt */
		rt_intr_wait(&port_interr,TM_INFINITE);
        rt_printf("Iteration number %d, diff %d\n", i, (i > 0) ? global_time[i] - global_time[i-1] : 0);
		//rt_task_wait_period(NULL);
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
  
  rt_task_create(&irq_server_task, "irq server task", 0, 0, 0);
  rt_task_start(&irq_server_task, &irq_server, NULL);
 
  
}

void init_xenomai() {
  mlockall(MCL_CURRENT|MCL_FUTURE);

  rt_print_auto_init(1);
}

int main (int argc, char *argv[]) 
{
  unsigned char byte;
  init_xenomai();
  startup();
  pause();
  
  byte = inb(0x37A); //disabled
  byte = byte & 0xEF; /* hex EF = binary 11101111 */
  outb(byte, 0x37A);
}
