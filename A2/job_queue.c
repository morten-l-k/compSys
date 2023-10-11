#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue = calloc(capacity, sizeof(struct job_queue));
  job_queue->capacity = capacity;
  if(job_queue == NULL) {
    return -1;
  } else {
    return 0;
  }
}

int job_queue_destroy(struct job_queue *job_queue) {
  assert(0);
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  for (int i = 0; i < job_queue->capacity; i++) {
    if(job_queue[i].data == NULL) {
        job_queue[i].data = data;
        break;
    } else {
      job_queue->next = &(job_queue[i+1]);
    }
  }
  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) { 
  assert(0);
}
