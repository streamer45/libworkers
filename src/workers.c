#include <math.h>
#include <pthread.h>
#include "workers.h"

#define N_THREADS_MAX 64

typedef struct {
  size_t id;
  pthread_t thread;

  int started;

  workers_t *ctx;
} worker_t;

struct workers {
  worker_t workers[N_THREADS_MAX];
  size_t n;

  pthread_cond_t conds[3];
  pthread_mutex_t mutex;

  int locked;

  uint64_t nidle;

  job_t job;
  void *ctx;
};

static void *worker(void *ctx) {

  worker_t *w;
  w = (worker_t *)ctx;

  for (;;) {

    pthread_mutex_lock(&w->ctx->mutex);

    if (!w->started) {
      fprintf(stderr, "%zu started\n", w->id);
      w->started = 1;
    }

    fprintf(stderr, "%zu going idle\n", w->id);

    ++w->ctx->nidle;

    if ( w->ctx->nidle >= w->ctx->n && (w->ctx->nidle % w->ctx->n) == 0 ) {
      pthread_cond_signal(&w->ctx->conds[1]);
      fprintf(stderr, "roar\n");
    }

    pthread_cond_wait(&w->ctx->conds[0], &w->ctx->mutex);

    fprintf(stderr, "%zu awake\n", w->id);

    pthread_mutex_unlock(&w->ctx->mutex);

    w->ctx->job(w->ctx->ctx, w->id);
  }

  return NULL;
}

workers_t *workers_create(size_t n_workers, job_t job, void *ctx) {
  int ret;
  workers_t *w;
  if (!n_workers) return NULL;
  if (n_workers > N_THREADS_MAX) return NULL;
  if (!ctx) return NULL;
  if (!job) return NULL;
  w = calloc(1, sizeof(*w));

  w->n = n_workers;
  w->ctx = ctx;
  w->job = job;

  pthread_cond_init(&w->conds[0], NULL);
  pthread_cond_init(&w->conds[1], NULL);
  pthread_cond_init(&w->conds[2], NULL);
  pthread_mutex_init(&w->mutex, NULL);

  pthread_mutex_lock(&w->mutex);

  w->locked = 1;
  //w->cycles = 1;

  for (int i = 0; i < w->n; ++i) {
    w->workers[i].ctx = w;
    w->workers[i].id = i;
    ret = pthread_create(&w->workers[i].thread, NULL, worker, &w->workers[i]);
    if (ret != 0) return NULL;
  }

  return w;
}

int workers_wait(workers_t *w) {
  //int i;
  if (!w) return -1;

  if (!w->locked) pthread_mutex_lock(&w->mutex);


  if (w->nidle <= w->n || (w->nidle % w->n) != 0) {
    fprintf(stderr, "workers_wait: waiting\n");
    pthread_cond_wait(&w->conds[1], &w->mutex);
  } else {
    fprintf(stderr, "workers_wait: not waiting %zu\n", w->nidle);
  }

  w->locked = 0;
  pthread_mutex_unlock(&w->mutex);
  return 0;
}

int workers_wake(workers_t *w) {
  if (!w) return -1;
  if (!w->locked) pthread_mutex_lock(&w->mutex);
  pthread_cond_broadcast(&w->conds[0]);


  // fprintf(stderr, "workers_wake: waiting\n");
  // pthread_cond_wait(&w->conds[2], &w->mutex);


  // fprintf(stderr, "workers_wake: woken!\n");
  w->locked = 0;
  pthread_mutex_unlock(&w->mutex);
  return 0;
}

int workers_destroy(workers_t *w) {
  if (!w) return -1;
  if (!w->locked) pthread_mutex_lock(&w->mutex);
  // for (int i = 0; i < w->n; ++i) {
  //   w->workers[i].done = 1;
  // }
  // fprintf(stderr, "waiting \n");
  // pthread_cond_wait(&w->conds[2], &w->mutex);
  // pthread_mutex_unlock(&w->mutex);
  // free(w);
  return 0;
}
