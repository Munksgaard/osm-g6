#include "kernel/sleepq.h"
#include "kernel/thread.h"
#include "kernel/spinlock.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"

#include "kernel/lock_cond.h"

/* Initialize a lock */
int lock_reset(lock_t *lock) {
  lock->taken = 0;
  spinlock_reset(&lock->spinlock);
  return 0;
}

void lock_acquire(lock_t *lock) {
  interrupt_status_t intr_status = _interrupt_disable();

  spinlock_acquire(&lock->spinlock);
  while(lock->taken) {
    spinlock_release(&lock->spinlock);

    /* Sleep until we can get the lock */
    sleepq_add(lock);
    thread_yield();

    /* Our turn, obtain the spinlock and check if we can take it now */
    spinlock_acquire(&lock->spinlock);
  }

  lock->taken = 1;
  spinlock_release(&lock->spinlock);
  _interrupt_set_state(intr_status);
}

void lock_release(lock_t *lock) {
  interrupt_status_t intr_status =_interrupt_disable();
  spinlock_acquire(&lock->spinlock);
  lock->taken = 0;
  spinlock_release(&lock->spinlock);
  _interrupt_set_state(intr_status);
  sleepq_wake(lock);
}

/* Initialize a condition variable */
int cond_reset(cond_t *cond) {
  cond = cond;
  return 1;
}

/* Wait for the condition to occur
 * Pre-condition: The process must hold the lock prior to calling wait */
void cond_wait(cond_t *cond, lock_t *condition_lock) {
  /* Sleep until signaled */
  interrupt_status_t intr_status =_interrupt_disable();
  sleepq_add(cond);
  lock_release(condition_lock);
  thread_yield();

  /* Woken up, now re-acquire the lock */
  lock_acquire(condition_lock);
  _interrupt_set_state(intr_status);
}

/* Signal one waiting process that the condition have occured */
void cond_signal(cond_t *cond, lock_t *condition_lock) {
  condition_lock = condition_lock;
  sleepq_wake(cond);
}

/* Signal all waiting processes that the condition have occured */
void cond_broadcast(cond_t *cond, lock_t *condition_lock) {
  condition_lock = condition_lock;
  sleepq_wake_all(cond);
}
