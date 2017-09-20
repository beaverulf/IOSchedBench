#include <stdio.h>
#include "lib/thpool.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread/pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#define NR_PROC 2

typedef struct ioparam{
    int op;
    int mbyte;
    int nr_times;
    int results[3][100];
    char *output_file;
} ioparam;

long long current_timestamp();

float io_operation(struct ioparam *iop);

int main(int argc, char *argv[]) {

    struct ioparam *iop = malloc(sizeof(struct ioparam));
    iop->mbyte = 100;
    iop->nr_times = 10;
    iop->output_file = "/dev/null";

    threadpool tpool;
    tpool = thpool_init(20);

    char *schedulers[3] = {"cfq","noop","deadline"};

    char sched_type[256];
    for(int k = 0; k<3; k++){
        sprintf(sched_type,"echo %s > /sys/block/sda/queue/scheduler",schedulers[k]);
        int ret = system(sched_type);
        system("cat /sys/block/sda/queue/scheduler");
        sequential_reads(tpool,iop);

    }

    return 0;
}

float io_operation(struct ioparam *iop){
    char cmd_string[1024];
    sprintf(cmd_string, "dd if=/dev/zero of=%s count=%d bs=%d",iop->output_file ,iop->nr_times, (iop->mbyte*1024000));

    long t1 = current_timestamp();
    int ret = system(cmd_string);
    long t2 = current_timestamp();

    printf("T:%0.3f mb:%d %u\n",((float)(t2-t1)/1000),iop->mbyte*iop->nr_times,(int)pthread_self());
    return ((float)(t2-t1)/1000);
}

int sequential_reads(threadpool tpool,struct ioparam *iop){
    for(int i=0; i < 100; i++){
        thpool_add_work(tpool,(void*)io_operation,(void*)iop);
    }
    thpool_wait(tpool);
    return 0;
}



long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

