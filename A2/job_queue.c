#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"



int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->data = malloc(sizeof(void*)*capacity);
  job_queue->capacity = capacity;
  job_queue->prod_pointer = 0;
  job_queue->cons_pointer = 0;
  job_queue->queue_count = 0;
  assert(pthread_mutex_init(&job_queue->queue_access, NULL) == 0);
  assert(pthread_cond_init(&job_queue->queue_condition, NULL) == 0);
  job_queue->queue_destroyed = 0; 

  // Ensuring malloc was successful 
  if (&(job_queue->data)== NULL) {
    return EXIT_FAILURE;
  }
  else { 
    return EXIT_SUCCESS;
  }
}

int job_queue_destroy(struct job_queue *job_queue) {
  assert(pthread_mutex_lock(&job_queue->queue_access) == 0);
  while (job_queue->queue_count != 0) {
    assert(pthread_cond_wait(&job_queue->queue_condition, &job_queue->queue_access) == 0);
  }
  job_queue->queue_destroyed = 1;

  // Destroy mutex and variable condition 
  pthread_mutex_destroy(&job_queue->queue_access);
  pthread_cond_destroy(&job_queue->queue_condition);

  // Free memory allocated for the job queue
  free(job_queue->data);
  return EXIT_SUCCESS;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  assert(pthread_mutex_lock(&job_queue->queue_access) == 0);

  if(job_queue->queue_destroyed) {
    assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
    return -1; 
  }

  while (job_queue->capacity <= job_queue->queue_count) {
    assert(pthread_cond_wait(&job_queue->queue_condition, &job_queue->queue_access) == 0);
  }

  //#1 step: Push data to queue and update queue count
  job_queue->data[job_queue->prod_pointer] = data; 
  job_queue->queue_count++;

  //#2 step: Move producer pointer
  if (job_queue->prod_pointer + 1 >= job_queue->capacity){
      job_queue->prod_pointer = 0;
    } 
  else {
      job_queue->prod_pointer++;
    }

  assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
  assert(pthread_cond_signal(&job_queue->queue_condition) == 0);
  return EXIT_SUCCESS;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  assert(pthread_mutex_lock(&job_queue->queue_access) == 0);
  while (job_queue->queue_count <= 0) {
    assert(pthread_cond_wait(&job_queue->queue_condition, &job_queue->queue_access) == 0);
    // Return -1 if the queue has been destroyed while the thread has been waiting
    if(job_queue->queue_destroyed) {
      assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
      return -1; 
    }
  }
  
  //Step 1: Write pointer to data and decrease queue count
  *data = job_queue->data[job_queue->cons_pointer];
  job_queue->queue_count--;
    
  //Step 2: Move consumer pointer
  if (job_queue->cons_pointer + 1 >= job_queue->capacity) {
    job_queue->cons_pointer = 0;
  } else {
    job_queue->cons_pointer++;
  }

  assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
  assert(pthread_cond_signal(&job_queue->queue_condition) == 0);
  return EXIT_SUCCESS; 
}
