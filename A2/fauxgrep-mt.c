// Setting _DEFAULT_SOURCE is necessary to activate visibility of
// certain header file contents on GNU/Linux systems.
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>

// err.h contains various nonstandard BSD extensions, but they are
// very handy.
#include <err.h>

#include <pthread.h>

#include "job_queue.h"

struct worker_args {
  char const *needle;
  struct job_queue *jq;
};

void *worker(void *args) {
  //Unpacking arguments
  struct worker_args *worker_args = (struct worker_args*) args;
  char const *needle = worker_args->needle;
  struct job_queue *jq = worker_args->jq;
  
  char const *path = NULL;
    while (1) {
      if (job_queue_pop(jq,(void**)&path) == 0) {
      
        FILE *f = fopen(path, "r");

      if (f == NULL) {
        warn("failed to open %s", path);
        return NULL;
      }

      char *line = NULL;
      size_t linelen = 0;
      int lineno = 1; //Line number
  
      while (getline(&line, &linelen, f) != -1) {
        if (strstr(line, needle) != NULL) {
          printf("%s:%d: %s", path, lineno, line);
        }
        lineno++;
      }

      free(line);
      fclose(f);
      } else {
        break;
      }
  }

  return NULL;
}

int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: [-n INT] STRING paths...");
    exit(1);
  }

  int num_threads = 1;
  char const *needle = argv[1]; //Recall: const qualifier makes variable read-only
  char * const *paths = &argv[2];


  if (argc > 3 && strcmp(argv[1], "-n") == 0) {
    // Since atoi() simply returns zero on syntax errors, we cannot
    // distinguish between the user entering a zero, or some
    // non-numeric garbage.  In fact, we cannot even tell whether the
    // given option is suffixed by garbage, i.e. '123foo' returns
    // '123'.  A more robust solution would use strtol(), but its
    // interface is more complicated, so here we are.
    num_threads = atoi(argv[2]);

    if (num_threads < 1) {
      err(1, "invalid thread count: %s", argv[2]);
    }

    needle = argv[3];
    paths = &argv[4];

  } else {
    needle = argv[1];
    paths = &argv[2];
  }

  // Initialising the job queue 
  struct job_queue jq;
  assert(job_queue_init(&jq,64) == 0);

  //Initilising worker threads
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  struct worker_args worker_args;
  worker_args.jq = &jq;
  worker_args.needle = needle;

  for (size_t i = 0; i < (size_t)num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &worker_args) != 0) {
      err(1, "pthread_create() failed");
    }
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

  FTSENT *p;
  while ((p = fts_read(ftsp)) != NULL) { //fts_read() reads a node in the file-tree-structure (FTS)
    switch (p->fts_info) {
    case FTS_D: //Directory entry
      break;
    case FTS_F: //File entry
      assert(job_queue_push(&jq,strdup(p->fts_path)) == 0); // Process the file p->fts_path, somehow.
      break;
    default:
      break;
    }
  }

  fts_close(ftsp);
  job_queue_destroy(&jq);// Shut down the job queue and the worker threads here.

  // Wait for all threads to finish.  This is important, as some may
  // still be working on their job.
  for (int i = 0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() failed");
    }
  }
  free(threads);

  return 0;
}
