#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct workers workers_t;
typedef void (*job_t) (void *ctx, size_t id);

workers_t *workers_create(size_t n_workers, job_t job, void *ctx);
int workers_wait(workers_t *w);
int workers_wake(workers_t *w);
int workers_destroy(workers_t *w);
