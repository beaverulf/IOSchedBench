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
} ioparam;

long long current_timestamp();

int io_operation(struct ioparam *iop);

void thread_identify(){
    printf("Thread #%u working\n", (int)pthread_self());
}

int main() {

    struct ioparam *iop = malloc(sizeof(struct ioparam));
    iop->mbyte = 1000;
    iop->nr_times = 10;


    //thpool_add_work(tpool,(void*)io_operation,(void*)iop);
    //thpool_wait(tpool);



    char *schedulers[3] = {"cfq","noop","deadline"};

    char sched_type[256];
    pid_t pidArr[NR_PROC];
    for(int k = 0; k<3; k++){
        sprintf(sched_type,"echo %s > /dev/block/sda/queue/scheduler",schedulers[k]);
        int ret = system(sched_type);
        system("cat /dev/block/sda/queue/scheduler");

        for(int i=0; i < NR_PROC; i++){
            if ((pidArr[i] = fork()) < 0){
                perror("Could not fork");
                exit(1);
            } else if (pidArr[i]==0){
                threadpool tpool;
                tpool = thpool_init(2);
                printf("I am a child.\n");
                //thpool_add_work(tpool,(void*)io_operation,(void*)iop);
                //thpool_wait(tpool);
                return 0;
            } else {
                printf("I am a parent.\n");
                threadpool tpool;
                tpool = thpool_init(2);
                thpool_add_work(tpool,(void*)io_operation,(void*)iop);
                thpool_wait(tpool);
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
    sprintf(cmd_string, "dd if=/dev/zero of=/dev/null count=%d bs=%d", iop->nr_times, (iop->mbyte*1024000));

    long t1 = current_timestamp();
    int ret = system(cmd_string);
    long t2 = current_timestamp();

    printf("Time: %li sec for %u\n",((t2-t1)/1000),(int)pthread_self());
    return ret;
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

