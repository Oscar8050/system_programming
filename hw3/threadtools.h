#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

extern int timeslice, switchmode;

typedef struct TCB_NODE *TCB_ptr;
typedef struct TCB_NODE{
    jmp_buf  Environment;
    int      Thread_id;
    TCB_ptr  Next;
    TCB_ptr  Prev;
    int i, N;
    int w, x, y, z;
} TCB;

extern jmp_buf MAIN, SCHEDULER;
extern TCB_ptr Head;
extern TCB_ptr Current;
extern TCB_ptr Work;
extern sigset_t base_mask, waiting_mask, tstp_mask, alrm_mask;

void sighandler(int signo);
void scheduler();

// Call function in the argument that is passed in
#define ThreadCreate(function, thread_id, number)                                         \
{                                                                                         \
	/* Please fill this code section. */												  \
          /*printf("4444\n");      */                                                     \
if (setjmp(MAIN) == 0){                                                                   \
    /*printf("5555\n");   */                                                              \
    function(thread_id, number);                                                          \
}                                                                                         \
   /*printf("3333\n");    */                                                              \
                                                                                          \
}

// Build up TCB_NODE for each function, insert it into circular linked-list
#define ThreadInit(thread_id, number)                                                     \
{                                                                                         \
	/* Please fill this code section. */                                                  \
    		/*printf("thread_id: %d\n", thread_id);	*/									  \
    if (thread_id == 1){                                                                  \
        Head = (TCB*)malloc(sizeof(TCB));                                                 \
        Work = Head;                                                                      \
        Work->Thread_id = thread_id;                                                      \
        Work->N = number;                                                                 \
        Work->Next = Work;                                                                \
        Work->Prev = Work;                                                                \
    }else if (thread_id == 2){                                                            \
        Work = (TCB*)malloc(sizeof(TCB));                                                 \
        Work->Thread_id = thread_id;                                                      \
        Work->N = number;                                                                 \
        Work->Next = Head;                                                                \
        Work->Prev = Head;                                                                \
        Head->Next = Work;                                                                \
        Head->Prev = Work;                                                                \
    }else if (thread_id == 3)  {                                                          \
        Work = (TCB*)malloc(sizeof(TCB));                                                 \
        Work->Thread_id = thread_id;                                                      \
        Work->N = number;                                                                 \
        Work->Next = Head;                                                                \
        Head->Prev = Work;                                                                \
        Head->Next->Next = Work;                                                          \
        Work->Prev = Head->Next;                                                          \
    }                                                                                     \
    /*printf("thread_id: %d\n", thread_id);   */                                          \
    if (setjmp(Work->Environment) == 0){                                                  \
        longjmp(MAIN, 1);                                                                 \
    }                                                                                     \
}

// Call this while a thread is terminated
#define ThreadExit()                                                                      \
{                                                                                         \
	/* Please fill this code section. */												  \
    longjmp(SCHEDULER, 2);                                                                \
}

// Decided whether to "context switch" based on the switchmode argument passed in main.c
#define ThreadYield()                                                                     \
{                                                                                         \
	/* Please fill this code section. */												  \
                                                                                          \
if (setjmp(Current->Environment) == 0){                                                   \
    if (switchmode == 0){                                                                 \
        longjmp(SCHEDULER, 1);                                                            \
    }else if (switchmode == 1){                                                           \
        sigpending(&waiting_mask);                                                        \
        if (sigismember(&waiting_mask, SIGTSTP)){                                         \
            sigprocmask(SIG_UNBLOCK, &tstp_mask, NULL);                                   \
        }else if (sigismember(&waiting_mask, SIGALRM)){                                   \
            sigprocmask(SIG_UNBLOCK, &alrm_mask, NULL);                                   \
        }                                                                                 \
        sigprocmask(SIG_SETMASK, &base_mask, NULL);                                       \
        longjmp(Current->Environment, 1);                                                 \
    }                                                                                     \
}                                                                                         \
}
