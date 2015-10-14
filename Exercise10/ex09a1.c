#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/intr.h>

#include <rtdk.h>

#define KBDIRQ 1

RT_TASK task;

void keyboard_handler(void *arg)
{
	RT_INTR intr;
	rt_intr_create(&intr, "keyboard handler", KBDIRQ, I_PROPAGATE);
	int c = 0;
	while(1)
	{
		rt_printf("#keyboard interrupt number %d\n",
                c += rt_intr_wait(&intr, TM_INFINITE));
	}
}

int main(int argc, char* argv[])
{
	/* Perform auto-init of rt_print buffers if the task doesn't do so */
	rt_print_auto_init(1);

	/* Avoids memory swapping for this program */
	mlockall(MCL_CURRENT | MCL_FUTURE);

	rt_task_create(&task, "task", 0, 50, 0);
	rt_task_start(&task, &keyboard_handler, 0);

	rt_printf("CRTL+C to stop\n");
	pause();
}
