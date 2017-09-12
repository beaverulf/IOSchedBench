#include <stdio.h>
#include "lib/thpool.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define NR_PROC 1

typedef struct ioparam{
	int op;
	int mbyte;
	int nr_times;
	char* scheduler; 
	pid_t pid;
} ioparam;

long long current_timestamp();

int io_operation(struct ioparam *iop);

int main() {
	struct ioparam *iop = malloc(sizeof(struct ioparam));
	iop->mbyte = 5;
	iop->nr_times = 100;

	char *schedulers[3] = {"cfq","noop","deadline"};

	char sched_type[256];
	pid_t pidArr[NR_PROC];
	for(int k = 0; k<3; k++){
		sprintf(sched_type,"echo %s > /sys/block/sda/queue/scheduler",schedulers[k]);
		iop->scheduler = schedulers[k];

		int ret = system(sched_type);
		system("cat /sys/block/sda/queue/scheduler");

		for(int i=0; i < NR_PROC; i++){
			if ((pidArr[i] = fork()) < 0){
				perror("Could not fork");
				exit(1);
			} else if (pidArr[i]==0){
				iop->pid = pidArr[i];
				io_operation(iop);
				return 0;
			} else {
				iop->pid = pidArr[i];
				io_operation(iop);
			}
		}

	}

	printf("Killing children.\n");
	for(int i=0; i < NR_PROC; i++){
		int cstatus;
		waitpid(pidArr[i],&cstatus,0);
		printf("Returning from proc #%d\n",pidArr[i]);
		pidArr[i]=-1;
	}

	return 0;
}

int io_operation(struct ioparam *iop){
	char cmd_string[1024];
	sprintf(cmd_string, "dd if=/dev/zero of=data/trashfile%d count=%d bs=%d status=none", (int)iop->pid,iop->nr_times, (iop->mbyte*1024000));

	long t1 = current_timestamp();
	int ret = system(cmd_string);
	long t2 = current_timestamp();

	printf("Time: %f sec Pid: %d Scheduler: %s\n",((float)(t2-t1)/1000),(int)iop->pid,iop->scheduler);
	return ret;
}

long long current_timestamp() {
	struct timeval te;
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
	// printf("milliseconds: %lld\n", milliseconds);
	return milliseconds;
}

