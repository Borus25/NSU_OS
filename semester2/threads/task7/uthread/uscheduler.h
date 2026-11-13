#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "uthread.h"

int scheduler_add(uthread_t* thread);
uthread_t* scheduler_current(void);
ucontext_t* scheduler_context(void);

void scheduler_run(void);

#endif