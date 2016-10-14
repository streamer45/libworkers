#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "workers.h"

#define N_THREADS 4

int check = 0;

#define ERROR(fmt, ...) \
  { \
    fprintf(stderr, "E: %s@%s:%d ", __func__, __FILE__, __LINE__); \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
    fprintf(stderr, "\n"); \
  }

static void worker(void *ctx, size_t id) {
  fprintf(stderr, "%d: doing some work! %s\n", id, (char *)ctx);
  __atomic_add_fetch(&check, 1, __ATOMIC_SEQ_CST);
}

int test() {
  int ret;
  workers_t *w;

  w = workers_create(N_THREADS, worker, "context");

  if (!w) return -1;

  fprintf(stderr, "workers created!\n");

  fprintf(stderr, "waiting...\n");

  ret = workers_wait(w);
  if (ret != 0) return -2;

  fprintf(stderr, "started!\n");

  ret = workers_wake(w);
  if (ret != 0) return -3;

  //fprintf(stderr, "woken!\n");

  ret = workers_wait(w);
  if (ret != 0) return -4;

  if (check != N_THREADS) {
    fprintf(stderr, "this should never happen %d!\n", check);
    return -5;
  }

  fprintf(stderr, "done! %d\n", check);

  ret = workers_destroy(w);
  if (ret != 0) return -6;

  fprintf(stderr, "workers destroyed!\n");

  return 0;
}

int main(int argc, char const *argv[]) {
  int ret;
  ret = test();
  if (ret != 0) {
    ERROR("Failure! %d", ret);
    return ret;
  }
  return 0;
}
