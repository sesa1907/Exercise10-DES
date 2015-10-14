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
RT_TASK dummy_task;

void dummy_task_function(void *arg)
{
	int i;
	while(1)
	{
		rt_task_sleep(1e7); //let linux system have time to kill the program
		rt_printf("Dummy task spinning\n");
		rt_timer_spin(1e9);
		rt_printf("Dummy task stopped\n");
	}
}

void keyboard_handler(void *arg)
{
	RT_INTR intr;
	RT_TASK *ctask;
	RT_TASK_INFO ctaskinfo;
	ctask = rt_task_self();
	rt_intr_create(&intr, "keyboard handler", KBDIRQ, I_PROPAGATE);
	int c = 0;
	while(1)
	{
		rt_printf("Keybord interrupts number: %d\n", c += rt_intr_wait(&intr, TM_INFINITE));
		rt_task_inquire(ctask, &ctaskinfo);
		rt_printf("Prio: %d\n", ctaskinfo.cprio);
	}
}

int main(int argc, char* argv[])
{
	rt_print_auto_init(1);

	mlockall(MCL_CURRENT | MCL_FUTURE);

	rt_task_create(&task, "task", 0, 50, 0);
	rt_task_create(&dummy_task, "dummy_task", 0, 51, 0);

	rt_task_start(&task, &keyboard_handler, 0);
	rt_task_start(&dummy_task, &dummy_task_function, 0);

	rt_printf("CRTL+C to stop\n");
	pause();
}
