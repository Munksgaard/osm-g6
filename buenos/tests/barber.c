#include "tests/lib.h"

#define BARBERS 3
#define CUSTOMERS 20

#define MAX_SITTING 10
#define MAX_WAITING 20

typedef struct barber_data {
    int id;
    usr_cond_t cond;
} barber_data;

typedef struct customer_data {
    int id;
    usr_cond_t cond;
} customer_data;

usr_lock_t barber_lock;

barber_data barber[BARBERS];
customer_data customers[CUSTOMERS];

usr_cond_t standing_cond;
usr_cond_t sitting_cond;
usr_cond_t barber_cond;

customer_data *next_customer = NULL;

int sitting = 0;
int standing = 0;

void simulatedwait(int duration) {
    int loc;
    for (loc=0; loc<duration; loc++) {
        // no-op
    }
}

void barber_function(void *arg) {
    barber_data *barber = (barber_data *)arg;
    customer_data *customer;

    syscall_lock_acquire(&barber_lock);

    while (1) {
        printf("Barber #%d looking for new customers\n", barber->id);

        if (next_customer == NULL) {
            syscall_condition_signal(&sitting_cond, &barber_lock);
            syscall_condition_signal(&standing_cond, &barber_lock);

            printf("Barber #%d sleeping\n", barber->id);
            syscall_condition_wait(&barber_cond, &barber_lock);
            printf("Barber #%d got woken up!\n", barber->id);
        } 
        
        customer = next_customer;
        next_customer = NULL;
        printf("Barber #%d servicing customer #%d\n", barber->id, customer->id);
        syscall_lock_release(&barber_lock);
        simulatedwait(100000);
        syscall_lock_acquire(&barber_lock);
        
        printf("Barber #%d done servicing customer #%d\n", barber->id, customer->id);
        syscall_condition_signal(&customer->cond, &barber_lock);
    }
}

void customer_function(void *arg) {
    customer_data *customer = (customer_data *)arg;

    syscall_lock_acquire(&barber_lock);

    printf("Customer #%d arrives\n", customer->id);

    if (sitting + standing >= MAX_WAITING) {
        printf("Barbershop full. Customer #%d leaving\n", customer->id);
        syscall_lock_release(&barber_lock);
        syscall_exit(0);
    }

    if (sitting >= MAX_SITTING) {
        printf("Chairs occupied. Customer #%d standing in line\n", customer->id);
        standing++;
        syscall_condition_wait(&standing_cond, &barber_lock);
        standing--;
    }

    if (sitting <= MAX_SITTING && next_customer != NULL) {
        printf("Customer #%d takes a seat\n", customer->id);
        sitting++;
        syscall_condition_wait(&sitting_cond, &barber_lock);
        sitting--;
    }

    printf("Customer #%d is being serviced\n", customer->id);

    next_customer = customer;
    syscall_condition_signal(&barber_cond, &barber_lock);
    syscall_condition_wait(&customer->cond, &barber_lock);
    syscall_lock_release(&barber_lock);
    syscall_exit(0);
}

int main() {
    int i;
    syscall_lock_create(&barber_lock);
    syscall_lock_acquire(&barber_lock);

    syscall_condition_create(&barber_cond);
    syscall_condition_create(&standing_cond);
    syscall_condition_create(&sitting_cond);

    for(i=0; i<(BARBERS); i++) {
        barber[i].id = i;
        syscall_condition_create(&barber[i].cond);
        syscall_fork((void (*)(int))(&barber_function), (int)&barber[i]);
    }

    for(i=0; i<(CUSTOMERS); i++) {
        customers[i].id = i;
        syscall_condition_create(&customers[i].cond);
        syscall_fork((void (*)(int))(&customer_function), (int)&customers[i]);
    }

    syscall_lock_release(&barber_lock);
    syscall_exit(0);

    return 0;
}
