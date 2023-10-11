#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

#include <err.h>

#include "job_queue.h"
#include "job_queue.c"


int main(int argc, char * const *argv) {
    if(argc != 1) {
        err(1, "invalid number of arguments: %s", argv[1]);
    } else {
        //Run tests
        printf("Running tests...\n");
        printf("Testing job_queue_init()\n");
        struct job_queue *jq = NULL;
        printf("Size of job queue before job_queue_init: %lu\n",sizeof(jq));
        job_queue_init(jq,64);
        printf("Size of job queue after job_queue_init: %lu\n",sizeof(jq));
    }
}
