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

void periodic_task (void *arg) {
	int i, write_time_diff;
	rt_task_set_periodic(NULL, TM_NOW, TASK_PERIOD); 
  	for(i=0; i < ITER; i++) {
		global_time[i] = rt_timer_read();
		printf("Iteration number %d, diff %d\n", i, (i > 0) ? global_time[i] - global_time[i-1] : 0);
		rt_task_wait_period(NULL);
		diff_time[i]	= (i > 0) ? global_time[i] - global_time[i-1] : 0;
	  }
	
  write_RTIMES("time_diff.csv",ITER,diff_time);
}

void startup()
{
  rt_timer_set_mode(BASEPERIOD);
  
  rt_task_create(&periodTask, "period_task", 0, 0, 0);
  rt_task_start(&periodTask, &periodic_task, NULL);

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
