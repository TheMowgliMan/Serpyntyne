#ifndef ARCHPROC_TASK_SWITCH_H_
#define ARCHPROC_TASK_SWITCH_H_

extern void switch_task(struct process *prev, struct process *next);

#endif
