// Setting _DEFAULT_SOURCE is necessary to activate visibility of
// certain header file contents on GNU/Linux systems.
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

// err.h contains various nonstandard BSD extensions, but they are
// very handy.
#include <err.h>

#include "histogram.h"
#include "job_queue.h"

int global_histogram[8] = { 0 };
// Opretter en mutex (lås) og initialiserer den. Dette forhindrer konkurrerende
// tråde i at samtidig opdatere histogrammet, hvilket sikrer trådsikkerhed.
pthread_mutex_t global_histogram_mutex;


int fhistogram(char const *path) {
  FILE *f = fopen(path, "r");

  int local_histogram[8] = { 0 };

  if (f == NULL) {
    fflush(stdout);
    warn("failed to open %s", path);
    return -1;
  }

  int i = 0;

  char c;
  while (fread(&c, sizeof(c), 1, f) == 1) {
    i++;
    update_histogram(local_histogram, c);
    if ((i % 100000) == 0) {
      pthread_mutex_lock(&global_histogram_mutex);
      merge_histogram(local_histogram, global_histogram);
      print_histogram(global_histogram);
      pthread_mutex_unlock(&global_histogram_mutex);
    }
  }

  fclose(f);
  pthread_mutex_lock(&global_histogram_mutex);
  merge_histogram(local_histogram, global_histogram);
  print_histogram(global_histogram);
  pthread_mutex_unlock(&global_histogram_mutex);
  return 0;
}

void *worker(void *arg) {
  struct job_queue *queue = arg;
  char *path;
  while (1) {
    // Hent en opgave (Job) fra jobkøen. Hvis der ikke er flere opgaver, afslut tråden.
      job_queue_pop(queue,(void**)&path);
      if (path == NULL) {
          break;
      }
      fhistogram(path);
  }
  free(path);
  return NULL;
}

int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: paths...");
    exit(1);
  }
  //pegepind til filstierne
  char * const *paths = &argv[1];
  int num_threads = 2;


  //mutex (lås)
  // pthread_t threads[8];
    if (pthread_mutex_init(&global_histogram_mutex, NULL) != 0) {
        perror("Mutex initialization failed");
        exit(1);
    }

  // FTS_LOGICAL = follow symbolic links
  // FTS_NOCHDIR = do not change the working directory of the process
  //
  // (These are not particularly important distinctions for our simple
  // uses.)
  int fts_options = FTS_LOGICAL | FTS_NOCHDIR;
  FTS *ftsp;
  if ((ftsp = fts_open(paths, fts_options, NULL)) == NULL) {
    err(1, "fts_open() failed");
    return -1;
  }

  // Create job queue.
  struct job_queue queue;
  job_queue_init(&queue, 64);


  // Start up the worker threads.
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  for (int i = 0; i < num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &queue) != 0) {
      err(1, "pthread_create() failed");
    }
  }
  FTSENT *p;
  //Gennemgå alle filer
  while ((p = fts_read(ftsp)) != NULL) { // per fil
    switch (p->fts_info) {
    case FTS_D:
      break;
    case FTS_F:
      job_queue_push(&queue,(void*)strdup(p->fts_path)); // lig i queue
      break;
    default:
      break;
    }
  }

  // // Wait for all threads to finish.  This is important, as some may
  // // still be working on their job.
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            err(1, "pthread_join() failed");
        }
    }
  fts_close(ftsp);

  move_lines(9);

  pthread_mutex_destroy(&global_histogram_mutex);

  return 0;
}

