#include "threadtools.h"

// Please complete this three functions. You may refer to the macro function defined in "threadtools.h"

// Mountain Climbing
// You are required to solve this function via iterative method, instead of recursion.
void MountainClimbing(int thread_id, int number){
	/* Please fill this code section. */
    //printf("6666\n");
    ThreadInit(thread_id, number);
    Current->i = 1;
    Current->w = 1;
    Current->x = 1;
    Current->y = 0;
    while (Current->i < Current->N){
        sleep(1);
        //do something...
        Current->y = Current->w + Current->x;
        printf("Mountain Climbing: %d\n", Current->y);
        Current->i += 1;
        Current->w = Current->x;
        Current->x = Current->y;
        /*if (!(Current->i < Current->N)){
            break;
        }*/
        ThreadYield();
    }
    if (Current->N == 0 || Current->N == 1){
        sleep(1);
        printf("Mountain Climbing: %d\n", 1);
        ThreadYield();
    }

    ThreadExit();
    
}

// Reduce Integer
// You are required to solve this function via iterative method, instead of recursion.
void ReduceInteger(int thread_id, int number){
	/* Please fill this code section. */
    ThreadInit(thread_id, number);
    Current->i = 0;
    Current->w = Current->N;
    //printf("number=%d\n", Current->N);
    while (Current->w > 1){
        sleep(1);
        //do something...
        //printf("in\n");
        if (Current->w % 2 == 0){
            Current->w = Current->w / 2;
        }else if (Current->w == 3 || Current->w % 4 == 1){
            Current->w -= 1;
        }else{
            Current->w += 1;
        }
        Current->i += 1;
        printf("Reduce Integer: %d\n", Current->i);
        /*if (!(Current->N > 1)){
            break;
        }*/
        ThreadYield();
    }
    if (Current->N == 1){
        sleep(1);
        printf("Reduce Integer: %d\n", Current->i);
        ThreadYield();
    }
    ThreadExit();
    
}

// Operation Count
// You are required to solve this function via iterative method, instead of recursion.
void OperationCount(int thread_id, int number){
	/* Please fill this code section. */
    ThreadInit(thread_id, number);
    Current->i = 0;
    if (Current->N %2 == 0)
        Current->w = 1;
    else
        Current->w = 2;
    Current->x = 0;
    while(Current->i < Current->N/2){
        sleep(1);
        //do something...
        Current->x += Current->w;
        Current->w += 2;
        Current->i += 1;
        printf("Operation Count: %d\n", Current->x);
        /*if (!(Current->i < Current->N/2)){
            break;
        }*/
        ThreadYield();
    }
    if (Current->N == 1){
        sleep(1);
        printf("Operation Count: %d\n", Current->x);
        ThreadYield();
    }
    ThreadExit();
    
}
