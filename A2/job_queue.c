#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
	job_queue->data = calloc(capacity, sizeof(void*));
	job_queue->prod_pointer = 0;
	job_queue->cons_pointer = 0;
  job_queue->capacity = capacity;
  job_queue->queue_count = 0;
  assert(pthread_mutex_init(&job_queue->queue_access,NULL) == 0);
  assert(pthread_cond_init(&job_queue->queue_condition,NULL) == 0);

  
  //Check if calloc was successful
  if(&(job_queue->data) == NULL) {
		return -1;
	} else {
		return 0;
	}
}

int job_queue_destroy(struct job_queue *job_queue) {
  free(job_queue);
  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  pthread_mutex_lock(&job_queue->queue_access);
  pthread_cond_wait(&job_queue->queue_condition,&job_queue->queue_access);
  while (job_queue->queue_count != job_queue->capacity) {
    //#1 step: Push data to queue
    job_queue->data[job_queue->prod_pointer] = data;
    job_queue->queue_count++;

    //#2 step: Move producer pointer
    if (job_queue->prod_pointer + 1 == job_queue->capacity){
      job_queue->prod_pointer = 0;
    } else {
      job_queue->prod_pointer++;
    }
  }
  pthread_mutex_unlock(&job_queue->queue_access);
  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) { 
  pthread_mutex_lock(&job_queue->queue_access);
  while (job_queue->queue_count != 0) {
    //Step 1: Write pointer to data
    *data = job_queue->data[job_queue->cons_pointer];
    job_queue->queue_count--;
    
     //Step 2: Move consumer pointer
    if (job_queue->cons_pointer + 1 == job_queue->capacity) {
      job_queue->cons_pointer = 0;
    } else {
      job_queue->cons_pointer++;
    }
  }
  pthread_mutex_unlock(&job_queue->queue_access);
  return 0;
}
