#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/io.h>
#include <native/task.h>
#include <native/timer.h>
#include <native/intr.h>
#include <rtdk.h>

#define TASK_PERIOD 100000
#define ITER 10000

RTIME global_time [ITER];
RTIME diff_time [ITER];

static RT_INTR port_interr;

RT_TASK irq_server_task;

int isr_installation(void) {
unsigned char byte;
  rt_intr_create(&port_interr,"port interrupt",7,I_PROPAGATE);
  // 7 is for port interrupt
    ioperm(0x37A, 1, 1); //enable interrupt port
	byte = inb(0x37A);
	byte = byte | 0x10; /* hex 10 = binary 00010000 */
	outb(byte, 0x37A);

	ioperm(0x378, 1, 1); //enable interrupt port
	byte = inb(0x378);
	byte = byte | 0x01; /* hex 10 = binary 00010000 */
	outb(byte, 0x378);
  //rt_intr_enable(&port_interr);

  return 0; 
}

void irq_server (void *cookie)
{
     //static int interrupts = 0;
     int nr_interrupts, i;
	 //rt_task_set_periodic(NULL, TM_NOW, TASK_PERIOD);
  for (i=0; i < ITER; i++) {
		global_time[i] = rt_timer_read();
        nr_interrupts = rt_intr_wait(&port_interr,TM_INFINITE);
        //interrupts++;
        //rt_printf("Iteration number %d, diff %d\n", i, (i > 0) ? global_time[i] - global_time[i-1] : 0);
		//rt_task_wait_period(NULL);
		//diff_time[i]	= (i > 0) ? global_time[i] - global_time[i-1] : 0;
		outb(inb(0x378) & 0xFE, 0x378);
		outb(inb(0x378) | 0x01, 0x378); /* enable interrupt */
  }
}

void startup()
{
  int retval;
  unsigned char byte;
  
  rt_printf("\nStart\n");

  retval=isr_installation();
	
  ioperm(0x37A,1,1);
  byte = inb(0x37A);
  byte = byte | 0x10;
  outb(byte, 0x37A);
  
  rt_task_create(&irq_server_task, "irq server task", 0, 0, 0);
  rt_task_start(&irq_server_task, &irq_server, NULL);
  
  byte = inb(0x37A);
  byte = byte & 0xEF;
  outb(byte, 0x37A);
}

void init_xenomai() {
  mlockall(MCL_CURRENT|MCL_FUTURE);

  rt_print_auto_init(1);
}

int main(int argc, char* argv[])
{
unsigned char byte;
  init_xenomai();
  startup();
  pause();
  byte = inb(0x37A); //disabled
  byte = byte & 0xEF; /* hex EF = binary 11101111 */
  outb(byte, 0x37A);
}
