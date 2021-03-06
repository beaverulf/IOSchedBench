#include <stdio.h>
#include "lib/thpool.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct ioparam{
	int op;
	int mbyte;
	int nr_times;
	char *output_file;
	char *input_file;
} ioparam;
int sequential_ops(threadpool tpool,struct ioparam *iop);

long long current_timestamp();

int io_operation(struct ioparam *iop);

int main(int argc, char *argv[]) {

	struct ioparam *iop = malloc(sizeof(struct ioparam));
	iop->output_file = "trashfile.trash";
	iop->input_file = "randomfile";
	threadpool tpool;
	tpool = thpool_init(100);

	char *schedulers[3] = {"cfq","noop","deadline"};

	char sched_type[256];
	
	//Runs the same tests on the 3 different schedulers.
	for(int k = 0; k<3; k++){
		sprintf(sched_type,"echo %s > /sys/block/sda/queue/scheduler",schedulers[k]);
		int ret = system(sched_type);
		system("cat /sys/block/sda/queue/scheduler >> data/results.txt");
		
		//Read/Write 2MB 300 times  
		system("echo \"Operation: 200*1MB\" >> data/results.txt");
		iop->mbyte = 2;
		iop->nr_times = 300;
		sequential_ops(tpool,iop);
	}

	return 0;
}

/*
Function for executing and logging an I/O operation.

Param:
 -struct ioparam *iop - A struct containing the operation parameters for the I/O request.
*/
int io_operation(struct ioparam *iop){
	char cmd_string[1024];
	sprintf(cmd_string, "dd if=%s of=%s count=%d bs=%d status=none; echo \"\">trashfile.trash",iop->input_file, iop->output_file ,iop->nr_times, (iop->mbyte*1024000));

	long t1 = current_timestamp();
	int ret = system(cmd_string);
	long t2 = current_timestamp();
	char result[128];
	sprintf(result,"echo \"%0.3f %d \">>data/results.txt",((float)(t2-t1)/1000),iop->mbyte*iop->nr_times/*,(int)pthread_self()*/);
	system(result);
	return ret;
}

/*
Adding work to the thread pool work queue.

Params:
 - threadpool tpool - the threadpool for which to add the workload
 - struct ioparam *iop - A struct containing the operation parameters for the I/O request.
*/
int sequential_ops(threadpool tpool,struct ioparam *iop){
	for(int i=0; i < 100; i++){
		thpool_add_work(tpool,(void*)io_operation,(void*)iop);
	}
	thpool_wait(tpool);
	return 0;
}

/*
Function for getting the current timestamp.
*/
long long current_timestamp() {
	struct timeval te;
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
	return milliseconds;
}

