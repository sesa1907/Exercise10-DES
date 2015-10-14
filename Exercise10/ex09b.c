#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/io.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/intr.h>

#include <rtdk.h>

#define LPT1IRQ 7

RT_TASK task;

void lpt1_handler(void *arg)
{
	RT_INTR intr;
	rt_intr_create(&intr, "lpt1 handler", LPT1IRQ, 0);
	int c = 0;
	while(1)
	{
		rt_printf("lpt1 interrupt number: %d\n",
        c += rt_intr_wait(&intr, TM_INFINITE));
	}
}

int main(int argc, char* argv[])
{
	rt_print_auto_init(1);

    /* setup lpt1 */
    ioperm(0x37A, 1, 1);
    unsigned char byte = inb(0x37A);
    byte |= 0x10;
    outb(byte, 0x37A);

	mlockall(MCL_CURRENT | MCL_FUTURE);

	rt_task_create(&task, "task", 0, 50, 0);
	rt_task_start(&task, &lpt1_handler, 0);

	rt_printf("CRTL+C to stop\n");
	pause();

    /* break down lpt1 */
    byte = inb(0x37A);
    byte &= 0xEF;
    outb(byte, 0x37A);
}
