#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
    printf("Program name: %s\n", argv[0]);
    int player_id;
    while ((argc > 1) && (argv[1][0] == '-')){
        switch (argv[1][1]){
            case 'n':
                //printf("%s\n", argv[2]);
                player_id = atoi(argv[2]);
                break;
            default:
                printf("Wrong Argument: %s\n", argv[1]);
        }
        argv+=2;
        argc-=2;
    }
    
    //printf("player_id=%d\n", player_id);

    char buf[100] = {'\0'};
    
    for (int round = 1; round <= 10; round++){
        int guess;
        srand ((player_id + round) * 323);
        guess = rand() % 1001;
        sprintf(buf, "%d %d\n", player_id, guess);
        write(STDOUT_FILENO, buf, strlen(buf));
    }
    
    
    return 0;
}
