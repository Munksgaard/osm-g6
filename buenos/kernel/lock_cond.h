#ifndef BUENOS_PROC_LOCK_COND
#define BUENOS_PROC_LOCK_COND

#include "kernel/thread.h"
#include "kernel/spinlock.h"

typedef struct {
  int taken;
  spinlock_t spinlock;
} lock_t;

typedef struct {
  int unused; // We only have the field to make conditions adressable.
} cond_t;


int lock_reset(lock_t *lock);
void lock_acquire(lock_t *lock);
void lock_release(lock_t *lock);

int cond_reset(cond_t *cond);
void cond_wait(cond_t *cond, lock_t *condition_lock);
void cond_signal(cond_t *cond, lock_t *condition_lock);
void cond_broadcast(cond_t *cond, lock_t *condition_lock);

#endif
