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

#include "job_queue.h"

#include <err.h>

#include "histogram.h"

static struct job_queue job_queue;
static long treeshold = 10000;

int global_histogram[8] = { 0 };

int global_bytes = 0;
pthread_mutex_t histogram_mutex = PTHREAD_MUTEX_INITIALIZER;


//Made a build for the worker
void *worker_thread(void *arg) {
  (void)arg;  //for no warning
  while (1) {
      char *path;

      //checks if there is any job available
      if (job_queue_pop(&job_queue, (void**)&path) == -1) break;

      int local_histogram[8] = {0};

      //var to count local-bytes
      long local_bytes = 0;

      //checks if F is possible to read
      FILE *f = fopen(path, "r");
      if (!f) { warn("failed to open %s", path); free(path); continue; }

      //counts bytes and saves how many bytes are counted
      char c;
      while (fread(&c, sizeof(c), 1, f) == 1) {
          update_histogram(local_histogram, c);
          local_bytes++;
      }
      fclose(f);


      //locks to update a global variable in a thread-safe way
      pthread_mutex_lock(&histogram_mutex);
      merge_histogram(local_histogram, global_histogram);
      global_bytes += local_bytes;

      while (global_bytes >= treeshold) {
          print_histogram(global_histogram);
          global_bytes -= treeshold;
      }
      pthread_mutex_unlock(&histogram_mutex);

      free(path);
  }
  return NULL;
}



int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: paths...");
    exit(1);
  }

  int num_threads = 1;
  char * const *paths = &argv[1];

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

    paths = &argv[3];
  } else {
    paths = &argv[1];
  }


  //initialise the job queue
  if (job_queue_init(&job_queue, 64)!=0) {
    fprintf(stderr,"failed to initialize job queue");
    exit(1);
  }

  pthread_t *threads = malloc(num_threads * sizeof(pthread_t));

  for (int i = 0; i < num_threads; i++) {
      pthread_create(&threads[i], NULL, worker_thread, NULL);
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
  while ((p = fts_read(ftsp)) != NULL) {
    switch (p->fts_info) {
    case FTS_D:
      break;
    case FTS_F: {
      //saves the path, so it doesnt change when threading
      char *path_copy = strdup(p->fts_path);

      //pushes the job into the queue, for the workers to grab
      job_queue_push(&job_queue,path_copy);
      break;
    }
    default:
      break;
    }
  }

  fts_close(ftsp);

  job_queue_destroy(&job_queue);

  for (int i = 0; i <num_threads; i++){
    pthread_join(threads[i], NULL);
  }
  print_histogram(global_histogram);

  free(threads);

  move_lines(9);

  return 0;
}
