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
  assert(pthread_cond_init(&job_queue->prod_cond, NULL) == 0);
  assert(pthread_cond_init(&job_queue->cons_cond, NULL) == 0);
  assert(pthread_cond_init(&job_queue->cond_destroy, NULL) == 0);
  job_queue->queue_destroyed = 0; 
  job_queue->cons_amount = 0;
  job_queue->queue_is_destroyed = 0; 

  

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
  job_queue->queue_destroyed = 1;
  // Wait for queue to be emptied and to rest of consumers to die
  while ((job_queue->queue_count > 0) || (job_queue->cons_amount > 0)) {
    // Wait for consumer pre-destruction to exit with -1
    printf("Here, 38. Count: %d, cons amount: %d \n", job_queue->queue_count, job_queue->cons_amount);
    assert(pthread_cond_wait(&job_queue->cond_destroy, &job_queue->queue_access) == 0);
    assert(pthread_cond_signal(&job_queue->cons_cond) == 0);
  }
  printf("Count %d, Cons: %d \n", job_queue->queue_count, job_queue->cons_amount);
  // Destroy mutex and variable condition
  while(job_queue->cons_amount > 0){
    assert(pthread_cond_wait(&job_queue->cond_destroy, &job_queue->queue_access) == 0);
    assert(pthread_cond_broadcast(&job_queue->cons_cond)==0);
  }
  printf("Here, 43\n");
  job_queue->queue_is_destroyed = 1; 
  assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
  assert(pthread_mutex_destroy(&job_queue->queue_access) == 0);
  assert(pthread_cond_destroy(&job_queue->prod_cond) == 0);
  assert(pthread_cond_destroy(&job_queue->cons_cond) == 0);
  assert(pthread_cond_destroy(&job_queue->cond_destroy) == 0);

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
    assert(pthread_cond_wait(&job_queue->prod_cond, &job_queue->queue_access) == 0);
  }
  printf("56\n");
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

  assert(pthread_cond_signal(&job_queue->cons_cond) == 0);
  assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
  return EXIT_SUCCESS;
}



int job_queue_pop(struct job_queue *job_queue, void **data) {
  if(job_queue->queue_is_destroyed == 1){
    return -1;
  }
  else {
    printf("Here, 86, cons: %d \n", job_queue->cons_amount);
    if (job_queue->cons_amount == 0 && job_queue->queue_destroyed == 1 && job_queue->queue_count == 0) {
      printf("101\n");
      pthread_cond_signal(&job_queue->cond_destroy);
      return -1;
    }
    assert(pthread_mutex_lock(&job_queue->queue_access) == 0);
    job_queue->cons_amount++;
    // If queue is empty, but it has not been destroyed yet
    while ((job_queue->queue_count <= 0) && (job_queue->queue_destroyed == 0)) {
      assert(pthread_cond_wait(&job_queue->cons_cond, &job_queue->queue_access) == 0);
    }
    // If queue is empty, and it has been destroyed
    // Return -1 if the queue has been destroyed while the thread has been waiting
    if ((job_queue->queue_count <= 0) && (job_queue->queue_destroyed == 1)) {
      // If more consumers have been trying to pop a destroyed queue, make them terminate before queue destruction
      if(job_queue->cons_amount > 1) {
        job_queue->cons_amount--;
        assert(pthread_cond_signal(&job_queue->cons_cond) == 0);
        assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
        return -1;
      }
      // Last consumer well go here (now the queue can be destroyed)
      else {
        printf("Here, 105, cosnumers: %d \n", job_queue->cons_amount);
        job_queue->cons_amount--;
        assert(pthread_cond_signal(&job_queue->cond_destroy) == 0);
        assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
        return -1; 
      }
    }


    // If the queue is not empty, but has been destroyed
    // Continue to work on input (but don't call producer)
    if((job_queue->queue_count > 0) && (job_queue->queue_destroyed == 1)) {
      //Step 1: Write pointer to data and decrease queue count
      *data = job_queue->data[job_queue->cons_pointer];
      job_queue->queue_count--;
        
      //Step 2: Move consumer pointer
      if (job_queue->cons_pointer + 1 >= job_queue->capacity) {
        job_queue->cons_pointer = 0;
      } else {
        job_queue->cons_pointer++;
      }

      // Decrease the amount of consumers, since this one is about to die
      job_queue->cons_amount--;

      assert(pthread_cond_signal(&job_queue->cons_cond) == 0);
      assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
      return EXIT_SUCCESS; 

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

    // Decrease the amount of consumers, since this one is about to die
    job_queue->cons_amount--;

    assert(pthread_cond_signal(&job_queue->prod_cond) == 0);
    assert(pthread_mutex_unlock(&job_queue->queue_access) == 0);
    return EXIT_SUCCESS; 
  }
}
