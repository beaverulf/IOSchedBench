#include <stdio.h>
#include "lib/thpool.h"
#include <sys/types.h>
#include <unistd.h>
#include <pthread/pthread.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct ioparam{
    int op;
    int mbyte;
    int nr_times;
} ioparam;

long long current_timestamp();

int io_operation(struct ioparam *iop);

void task1(int k){
    printf("Thread #%u working on %d\n", (int)pthread_self(),k);
}

int main() {
    threadpool tpool;
    tpool = thpool_init(4);

    struct ioparam *iop = malloc(sizeof(struct ioparam));
    iop->mbyte = 1000;
    iop->nr_times = 100;

    thpool_add_work(tpool,(void*)io_operation,(void*)iop);
    thpool_wait(tpool);
    return 0;
}

int io_operation(struct ioparam *iop){
    char cmd_string[1024];
    sprintf(cmd_string, "dd if=/dev/zero of=/dev/null count=%d bs=%d", iop->nr_times, (iop->mbyte*1024000));

    long t1 = current_timestamp();
    int ret = system(cmd_string);
    long t2 = current_timestamp();

    printf("Time: %li sec\n",((t2-t1)/1000));
    return ret;
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

