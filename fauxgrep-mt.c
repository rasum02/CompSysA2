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

static struct job_queue job_queue;

static const char *global_needle;

int fauxgrep_file(char const *needle, char const *path) {
  FILE *f = fopen(path, "r");

  if (f == NULL) {
    warn("failed to open %s", path);
    return -1;
  }

  char *line = NULL;
  size_t linelen = 0;
  int lineno = 1;

  while (getline(&line, &linelen, f) != -1) {
    if (strstr(line, needle) != NULL) {
      printf("%s:%d: %s", path, lineno, line);
    }

    lineno++;
  }

  free(line);
  fclose(f);

  return 0;
}


//Made a build for the worker
void *worker_thread(void *arg){
  (void)arg;

  while (1) {

    char *path;

    //checks wether there is any jobs available 
    if (job_queue_pop(&job_queue, (void**)&path)==-1){
      break;
    }

    //reads file 
    fauxgrep_file(global_needle, path);

    //Frees saved memory for path
    free(path);

  }

  return NULL;
}


int main(int argc, char * const *argv) {
  if (argc < 2) {
    err(1, "usage: [-n INT] STRING paths...");
    exit(1);
  }

  int num_threads = 1;
  char const *needle = argv[1];
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

  global_needle = needle;

  //initialise the job queue
  if (job_queue_init(&job_queue, 64)!=0) {
    fprintf(stderr,"failed to initialize job queue");
    exit(1);
  }

  //initialise the workers
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

  free(threads);

  return 0;
}
