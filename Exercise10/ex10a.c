#include <stdio.h>
#include <stdlib.h> 
#include <sys/mman.h> 
#include <native/task.h>
//#include <native/timer.h>

#define TASK_PERIOD 1e5
#define ITER 10000
#define BASEPERIOD 0

RT_TASK periodTask;
RTIME write_time;
RTIME global_time [ITER];
RTIME diff_time [ITER];


void write_RTIMES(char * filename, unsigned int number_of_values,
                  RTIME *time_values){
         unsigned int n=0;
         FILE *file;
         file = fopen(filename,"w");
         while (n<number_of_values-1) {
              fprintf(file,"%u;%llu\n",n,time_values[n+1]-time_values[n]);  
              n++;
         }
		 
         fclose(file);
 }

void periodic_task (void *arg) {
	int i,j;
	rt_task_set_periodic(NULL, TM_NOW, TASK_PERIOD); 
	
  	for(i=0, j=0; i < ITER; i++,j++) {
		global_time[i] = rt_timer_read();
		rt_task_wait_period(NULL);
	  }
	for(j=0;j<ITER-1;j++)
	{
		rt_printf("diff time :%d\n",global_time[j+1]-global_time[j]);
	}
	
  write_RTIMES("time_diff.csv",ITER,global_time);
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
