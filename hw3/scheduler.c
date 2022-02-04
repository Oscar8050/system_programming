#include "threadtools.h"

/*
1) You should state the signal you received by: printf('TSTP signal caught!\n') or printf('ALRM signal caught!\n')
2) If you receive SIGALRM, you should reset alarm() by timeslice argument passed in ./main
3) You should longjmp(SCHEDULER,1) once you're done.
*/
void sighandler(int signo){
	/* Please fill this code section. */
	if (signo == SIGTSTP){
		printf("TSTP signal caught!\n");
		sigprocmask(SIG_SETMASK, &base_mask, NULL);
		longjmp(SCHEDULER, 1);
	}else if (signo == SIGALRM){
		printf("ALRM signal caught!\n");
		sigprocmask(SIG_SETMASK, &base_mask, NULL);
		alarm(timeslice);
		longjmp(SCHEDULER, 1);
	}
}

/*
1) You are stronly adviced to make 
	setjmp(SCHEDULER) = 1 for ThreadYield() case
	setjmp(SCHEDULER) = 2 for ThreadExit() case
2) Please point the Current TCB_ptr to correct TCB_NODE
3) Please maintain the circular linked-list here
*/
void scheduler(){
	/* Please fill this code section. */
	//printf("Scheduler\n");
    Current = Head;
	int scheduleState = setjmp(SCHEDULER);
    if (scheduleState == 0){
		longjmp(Current->Environment, 1);
	}else if (scheduleState == 1){
		//printf("back\n");
		Current = Current->Next;
		longjmp(Current->Environment, 1);
	}else if (scheduleState == 2){
		//printf("freeing\n");
		Work = Current;
		if (Current->Prev == Current){
			free(Current);
			//printf("last\n");
			longjmp(MAIN, 1);
		}else if (Current->Next != Current->Prev){
			//printf("first\n");
			Current->Prev->Next = Current->Next;
			Current->Next->Prev = Current->Prev;
		}else if (Current->Next == Current->Prev){
			//printf("second\n");
			Current->Prev->Next = Current->Prev;
			Current->Prev->Prev = Current->Prev;
		}
		Current = Current->Next;
		free(Work);
		//printf("current jmp thred=%d\n", Current->Thread_id);
		longjmp(Current->Environment, 1);
	}
	
    
}
