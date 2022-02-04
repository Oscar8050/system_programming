#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

void err_sys(const char* x)
{
    perror(x);
    exit(1);
}

int main(int argc, char *argv[]){

    //fprintf(stderr, "1232jkjf\n");
    
    int host_id, depth, lucky_number;
    while ((argc > 1) && argv[1][0] == '-'){
        switch (argv[1][1]){
            case 'm':
                host_id = atoi(argv[2]);
                break;
            case 'd':
                depth = atoi(argv[2]);
                break;
            case 'l':
                lucky_number = atoi(argv[2]);
                break;
            default:
                
        }
        argc -= 2;
        argv += 2;
    }

    //fprintf(stderr, "depth=%d\n", depth);
    
    char fifo[20] = {'\0'};
    sprintf(fifo, "fifo_%d.tmp", host_id);
    //printf("%s\n", fifo);
    int fd_read, fd_write;
    if (depth == 0){
        fd_read = open(fifo, O_RDONLY);
        fd_write = open("fifo_0.tmp", O_WRONLY);
        dup2(fd_read, STDIN_FILENO);
        dup2(fd_write, STDOUT_FILENO);
    }
    if (depth != 2){
        int fd1_r[2], fd1_w[2], fd2_r[2], fd2_w[2];
        if (pipe(fd1_r) < 0){
            err_sys("pipe error");
        }
        if (pipe(fd1_w) < 0){
            err_sys("pipe error");
        }
        if (pipe(fd2_r) < 0){
            err_sys("pipe error");
        }
        if (pipe(fd2_w) < 0){
            err_sys("pipe error");
        }

        if (fork() == 0){
            close(fd2_r[0]);
            close(fd2_r[1]);
            close(fd2_w[0]);
            close(fd2_w[1]);
            close(fd1_w[1]);
            close(fd1_r[0]);

            dup2(fd1_r[1], STDOUT_FILENO);
            dup2(fd1_w[0], STDIN_FILENO);
            
            char str_host_id[10] = {'\0'}, str_depth[10] = {'\0'}, str_lucky_number[10] = {'\0'};
            sprintf(str_host_id, "%d", host_id);
            sprintf(str_depth, "%d", depth+1);
            sprintf(str_lucky_number, "%d", lucky_number);
            //fprintf(stderr, "before exec\n");
            //char *arg[] = {strdup("-m"), strdup(str_host_id), strdup("-d"), strdup(str_depth), strdup("-l"), strdup(str_lucky_number), NULL};
            //fprintf(stderr, "after exec\n");
            //int ret = execvp("host", arg);
            int ret = execl("host", "host", "-m", str_host_id, "-d", str_depth, "-l", str_lucky_number, NULL);
            if (ret == -1)
                perror("execvp error");

            exit(0);//?
        }
        

        if (fork() == 0){
            close(fd1_r[0]);
            close(fd1_r[1]);
            close(fd1_w[0]);
            close(fd1_w[1]);
            close(fd2_w[1]);
            close(fd2_r[0]);

            dup2(fd2_r[1], STDOUT_FILENO);
            dup2(fd2_w[0], STDIN_FILENO);

            char str_host_id[10] = {'\0'}, str_depth[10] = {'\0'}, str_lucky_number[10] = {'\0'};
            sprintf(str_host_id, "%d", host_id);
            sprintf(str_depth, "%d", depth+1);
            sprintf(str_lucky_number, "%d", lucky_number);
            //char *arg[] = {"-m", str_host_id, "-d", str_depth, "-l", str_lucky_number, NULL};
            //int ret = execvp("host", arg);
            int ret = execl("host", "host", "-m", str_host_id, "-d", str_depth, "-l", str_lucky_number, NULL);
            if (ret == -1)
                perror("execvp error");

            exit(0);//?
        }

        FILE *fp1_r = fdopen(fd1_r[0], "r");
        FILE *fp1_w = fdopen(fd1_w[1], "a");

        FILE *fp2_r = fdopen(fd2_r[0], "r");
        FILE *fp2_w = fdopen(fd2_w[1], "a");

        if (depth == 0){
            //fprintf(stderr, "in host\n");
            int p1, p2, p3, p4, p5, p6, p7, p8;
            //fprintf(stderr, "12121212\n");
            scanf(" %d %d %d %d %d %d %d %d", &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8);
            fflush(stdin);
            //fprintf(stderr, "scan%d %d %d %d %d %d %d %d\n", p1, p2, p3, p4, p5, p6, p7, p8);
            //fprintf(stderr, "12121212\n");
            while (1){
                fprintf(fp1_w, "%d %d %d %d\n", p1, p2, p3, p4);
                fflush(fp1_w);
                fprintf(fp2_w, "%d %d %d %d\n", p5, p6, p7, p8);
                fflush(fp2_w);

                int score[8] = {0};
                int pl1, guess1, pl2, guess2;
                int die = 0;
                for (int i = 0; i < 10; i++){

                    //fprintf(stderr, "depth000000\n");
                    fscanf(fp1_r, "%d %d", &pl1, &guess1);
                    fflush(fp1_r);
                    fscanf(fp2_r, "%d %d", &pl2, &guess2);
                    fflush(fp2_r);
                    //fprintf(stderr, "after depth0000000\n");

                    //fprintf(stderr, "top pl1=%d guess1=%d pl2=%d guess2=%d\n", pl1, guess1, pl2, guess2);

                    if (guess1 == -1 && guess2 == -1){
                        die = 1;
                        break;
                    }

                    int win_id, win_guess;

                    if (abs(guess1 - lucky_number) < abs(guess2 - lucky_number)){
                        win_id = pl1;
                        win_guess = guess1;
                    }else if (abs(guess1 - lucky_number) > abs(guess2 - lucky_number)){
                        win_id = pl2;
                        win_guess = guess2;
                    }else if (guess1 < guess2){
                        win_id = pl1;
                        win_guess = guess1;
                    }else if (guess1 > guess2){
                        win_id = pl2;
                        win_guess = guess2;
                    }


                    //fprintf(stderr, "lucky_number=%d winid=%d winguess=%d\n", lucky_number, win_id, win_guess);
                    
                    if (win_id == p1)
                        score[0] += 10;
                    else if(win_id == p2)
                        score[1] += 10;
                    else if (win_id == p3)
                        score[2] += 10;
                    else if (win_id == p4)
                        score[3] += 10;
                    else if (win_id == p5)
                        score[4] += 10;
                    else if (win_id == p6)
                        score[5] += 10;
                    else if (win_id == p7)
                        score[6] += 10;
                    else if (win_id == p8)
                        score[7] += 10;
                }

                if (die == 1)
                    break;

                //fprintf(stderr, "%d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n", host_id, p1, score[0], p2, score[1], p3, score[2], p4, score[3], p5, score[4], p6, score[5], p7, score[6], p8, score[7]);
                printf("%d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n", host_id, p1, score[0], p2, score[1], p3, score[2], p4, score[3], p5, score[4], p6, score[5], p7, score[6], p8, score[7]);
                fflush(stdout);
                //fprintf(stderr, "endhost\n");

                scanf(" %d %d %d %d %d %d %d %d", &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8);
                fflush(stdin);
                //fprintf(stderr, "scan%d %d %d %d %d %d %d %d\n", p1, p2, p3, p4, p5, p6, p7, p8);
            }
            wait(NULL);
            wait(NULL);
            

        }else if (depth == 1){
            int p1, p2, p3, p4;
            scanf("%d %d %d %d", &p1, &p2, &p3, &p4);
            fflush(stdin);
            while (1){
                fprintf(fp1_w, "%d %d\n", p1, p2);
                fflush(fp1_w);
                fprintf(fp2_w, "%d %d\n", p3, p4);
                fflush(fp2_w);

                int pl1, guess1, pl2, guess2;
                int die = 0;
                for (int i = 0; i < 10; i++){
                    //fprintf(stderr, "depth1111111111\n");
                    fscanf(fp1_r, "%d %d", &pl1, &guess1);
                    fflush(fp1_r);
                    fscanf(fp2_r, "%d %d", &pl2, &guess2);
                    fflush(fp2_r);
                    //fprintf(stderr, "after depth1111111111\n");

                    //fprintf(stderr, "pl1=%d guess1=%d pl2=%d guess2=%d\n", pl1, guess1, pl2, guess2);

                    if (guess1 == -1 && guess2 == -1){
                        printf("%d %d\n", pl1, guess1);
                        fflush(stdout);
                        die = 1;
                        break;
                    }

                    if (abs(guess1 - lucky_number) < abs(guess2 - lucky_number)){
                        printf("%d %d\n", pl1, guess1);
                        fflush(stdout);
                    }else if (abs(guess1 - lucky_number) > abs(guess2 - lucky_number)){
                        printf("%d %d\n", pl2, guess2);
                        fflush(stdout);
                    }else if (guess1 < guess2){
                        printf("%d %d\n", pl1, guess1);
                        fflush(stdout);
                    }else if (guess1 > guess2){
                        printf("%d %d\n", pl2, guess2);
                        fflush(stdout);
                    }
                }

                if (die == 1)
                    break;

                scanf("%d %d %d %d", &p1, &p2, &p3, &p4);
                fflush(stdin);
            }
            wait(NULL);
            wait(NULL);
            

        }

        


    }else{
        //depth == 2

        int p1, p2;
        scanf("%d %d", &p1, &p2);
        fflush(stdin);
        while (1){

            if (p1 == -1 && p2 == -1){
                printf("%d %d\n", p1, p1);
                fflush(stdout);
                break;
            }

            int fd1[2], fd2[2];
            if (pipe(fd1) < 0){
                err_sys("pipe error");
            }
            if (pipe(fd2) < 0){
                err_sys("pipe error");
            }
            
            
            if (fork() == 0){
                close(fd1[0]);
                dup2(fd1[1], STDOUT_FILENO);
                char buf[10] = {'\0'};
                sprintf(buf, "%d", p1);
                //char *arg[] = {"-n", buf, NULL};
                //int ret = execvp("player", arg);
                //fprintf(stderr, "player1=%d\n", p1);
                int ret = execl("player", "player", "-n", buf, NULL);
                if (ret == -1)
                    perror("execvp error");
                exit(0); //?
            }

            if (fork() == 0){
                close(fd2[0]);
                dup2(fd2[1], STDOUT_FILENO);
                char buf[10] = {'\0'};
                sprintf(buf, "%d", p2);
                //char *arg[] = {"-n", buf, NULL};
                //int ret = execvp("player", arg);
                //fprintf(stderr, "player2=%d\n", p2);
                int ret = execl("player", "player", "-n", buf, NULL);
                if (ret == -1)
                    perror("execvp error");
                    
                exit(0); //?
            }

            FILE *fp1_r = fdopen(fd1[0], "r");
            FILE *fp2_r = fdopen(fd2[0], "r");
            int pl1, guess1, pl2, guess2;

            for (int i = 0; i < 10; i++){
                //fprintf(stderr, "before scan player i=%d\n", i);
                fscanf(fp1_r, "%d %d", &pl1, &guess1);
                fflush(fp1_r);
                fscanf(fp2_r, "%d %d", &pl2, &guess2);
                fflush(fp2_r);
                //fprintf(stderr, "after scan player\n");

                //fprintf(stderr, "pl1=%d guess1=%d pl2=%d guess2=%d\n", pl1, guess1, pl2, guess2);

                if (abs(guess1 - lucky_number) < abs(guess2 - lucky_number)){
                    printf("%d %d\n", pl1, guess1);
                }else if (abs(guess1 - lucky_number) > abs(guess2 - lucky_number)){
                    printf("%d %d\n", pl2, guess2);
                }else if (guess1 < guess2){
                    printf("%d %d\n", pl1, guess1);
                }else if (guess1 > guess2){
                    printf("%d %d\n", pl2, guess2);
                }
                fflush(stdout);
            }

            wait(NULL);
            wait(NULL);
            scanf("%d %d", &p1, &p2);
            fflush(stdin);
        }
        

    }
    
    //fprintf(stderr, "kill host\n");
    return 0;
}
