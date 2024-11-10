#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "synch.h"
#include "filesys/file.h"
//#define USERPROG

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
   /* Owned by thread.c. */
   tid_t tid;                          /* Thread identifier. */
   enum thread_status status;          /* Thread state. */
   char name[16];                      /* Name (for debugging purposes). */
   uint8_t *stack;                     /* Saved stack pointer. */
   int priority;                       /* Priority. */
   struct list_elem allelem;           /* List element for all threads list. */
   
   /*Alam Clock*/
   int64_t alarm;
   
   /*Priority Scheduling*/  
   int init_priority;                  /* priority가 바뀔 것을 대비해 초기 priority 저장*/
   struct lock* request_lock;          /* 현재 이 thread가 acquire한 lock*/
   struct list donation_list;          /* 이 thread에게 donate해준 thread들 list*/
   struct list_elem donaelem;          /* donate한 thread의 donation_list에 연결되는 list element*/

   /*Advanced Scheduling*/
   int nice;
   int recent_cpu;
   
   /* Shared between thread.c and synch.c. */
   struct list_elem elem;              /* List element. */

#ifdef USERPROG
   /* Owned by userprog/process.c. */
   uint32_t *pagedir;                  /* Page directory. */

   /*system call*/
   struct list child_list; 
   struct list_elem child_elem;
   int exit_code;
   struct semaphore sema_wait;
   struct semaphore sema_exit;

   /*system call-file*/
   struct file* fdt[128];
   int fd_count;

   /*file system*/
   bool is_file_valid;
   struct semaphore sema_file1;
   struct semaphore sema_file2;
   struct file* myfile;
#endif
    
    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

bool is_idle(struct thread*);
struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

/*compare function*/
bool cmp_alarm(const struct list_elem *a, const struct list_elem *b, void *aux UNUSED);
bool cmp_priority (const struct list_elem* elem_a, const struct list_elem* elem_b, void * aux UNUSED);

/*alarm clock*/
void thread_alarm(int64_t alarm);
void thread_wakeup(int64_t ticks);

/*Advanced Scheduler*/
void calc_priority(struct thread *t);
void calc_recent_cpu(struct thread *t);
void calc_load_avg(void);
void update_priority(void);
void update_recent_cpu(void);

#define F (1<<14)

inline int n_to_fp(int n){
   return n * F;
}
inline int fp_to_int(int x){
   return x / F;
}
inline int fp_to_round_int(int x){
   return (x >= 0 ? ((x + F / 2) / F) : ((x - F / 2) / F));
}
inline int fp_add(int x, int y){
   return x+y;
}
inline int fp_sub(int x, int y){
   return x-y;
}
inline int fp_int_add(int x, int n){
   return x + n * F;
}
inline int int_fp_add(int n, int x){
   return x + n * F;
}
inline int fp_int_sub(int x, int n){
   return x - n * F;
}
inline int int_fp_sub(int n, int x){
   return n * F - x;
}
inline int fp_multiply(int x, int y){
   return ((int64_t) x) * y /F;
}
inline int fp_int_multiply(int x, int n){
   return x*n;
}
inline int fp_divide(int x, int y){
   return ((int64_t) x) * F / y;
}
inline int fp_int_divide(int x, int n){
   return x / n;
}

/*Project 2*/
#ifdef USERPROG
struct thread* get_thread_with_tid(tid_t tid);
#endif

#endif /* threads/thread.h */
