#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>

bool **dots, **next_dots;
bool transform[9][2] = {{0, 0}, {0, 0}, {0, 1}, {1, 1}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}};

typedef struct job{
    int col_s, col_e, row_s, row_e, r_size, c_size;
}Job;

typedef struct {
    pthread_mutex_t mlock;
    bool **buf;
}buftype;

void *survive(void *arg){
    Job *job = (Job *) arg;
    //printf("thread\n");
    //printf("%d %d %d %d\n", job->col_s, job->col_e, job->row_s, job->row_e);
    if (job->col_e != -1 && job->row_e != -1){
        int lives;
        for (int x = job->row_s; x <= job->row_e; x++){
            for (int y = job->col_s; y <= job->col_e; y++){
                lives = 0;
                if (x-1 >= 0 && dots[x-1][y] == 1)
                    lives += 1;
                if (x+1 < job->r_size && dots[x+1][y] == 1)
                    lives += 1;
                if (y-1 >= 0 && dots[x][y-1] == 1)
                    lives += 1;
                if (y+1 < job->c_size && dots[x][y+1] == 1)
                    lives += 1;
                if (x-1 >= 0 && y-1 >= 0 && dots[x-1][y-1] == 1)
                    lives += 1;
                if (x-1 >= 0 && y+1 < job->c_size && dots[x-1][y+1] == 1)
                    lives += 1;
                if (x+1 < job->r_size && y-1 >= 0 && dots[x+1][y-1] == 1)
                    lives += 1;
                if (x+1 < job->r_size && y+1 < job->c_size && dots[x+1][y+1] == 1)
                    lives += 1;
                
                if (dots[x][y] == 0)
                    next_dots[x][y] = transform[lives][0];
                else
                    next_dots[x][y] = transform[lives][1];
            }
        }
        
    }
    pthread_exit(NULL);
    
}


int main(int argc, char *argv[]) {
    
    int p = 0, t = 0;
    char in[50], out[50];
    while ((argc > 3) && argv[1][0] == '-'){
        switch (argv[1][1]){
            case 'p':
                p = atoi(argv[2]);
                break;
            case 't':
                t = atoi(argv[2]);
                break;
            default:
                break;
        }
        argc -= 2;
        argv += 2;
    }
    
    strcpy(in, argv[1]);
    strcpy(out, argv[2]);
    
    //printf("%d %d %s %s\n", p, t, in, out);
    
    FILE *fp, *fpout;
    //printf("1111\n");
    fp = fopen(in, "r+");
    fpout = fopen(out, "w");
    //printf("23122\n");
    int row, col, epoch;
    fscanf(fp, "%d %d %d", &row, &col, &epoch);
    getc(fp);
    //printf("%d %d %d\n", row, col, epoch);
    
    
    /*for (int i = 0; i < row; i++){
        for (int j = 0; j < col; j++){
            printf("%d", dots[i][j]);
        }
        printf("\n");
    }*/
    
    Job jobs[t];
    
    if (t > 0){
        
        dots = (bool **)malloc(sizeof(bool *) * row);
        next_dots = (bool **)malloc(sizeof(bool *) * row);
        bool **tmp;
        for (int i = 0; i < row; i++){
            dots[i] = (bool *)malloc(sizeof(bool) * col);
            next_dots[i] = (bool *)malloc(sizeof(bool) * col);
        }
        
        char element;
        for (int i = 0; i < row; i++){
            for (int j = 0; j < col+1; j++){
                element = getc(fp);
                if (element == '\n')
                    continue;
                if (element == 'O')
                    dots[i][j] = 1;
                else
                    dots[i][j] = 0;
            }
        }
        
        int cut;
        if (row > col){
            cut = row / t;
            for (int i = 0; i < t-1; i++){
                jobs[i].row_s = cut * i;
                jobs[i].row_e = cut * (i+1) - 1;
                jobs[i].col_s = 0;
                jobs[i].col_e = col - 1;
            }
            jobs[t-1].row_s = cut * (t-1);
            jobs[t-1].row_e = row-1;
            jobs[t-1].col_s = 0;
            jobs[t-1].col_e = col - 1;
        }else{
            cut = col / t;
            for (int i = 0; i < t-1; i++){
                jobs[i].col_s = cut * i;
                jobs[i].col_e = cut * (i+1) - 1;
                jobs[i].row_s = 0;
                jobs[i].row_e = row - 1;
            }
            jobs[t-1].col_s = cut * (t-1);
            jobs[t-1].col_e = col-1;
            jobs[t-1].row_s = 0;
            jobs[t-1].row_e = row - 1;
        }
        for (int i = 0; i < t; i++){
            jobs[i].r_size = row;
            jobs[i].c_size = col;
        }
        
        pthread_t tp[t];
        //printf("t=%d\n", t);
        for (int k = 0; k < epoch; k++){
            for (int i = 0; i < t; i++){
                //printf("before\n");
                pthread_create(&tp[i], NULL, survive, &jobs[i]);
                //printf("after\n");
            }
            
            for (int i = 0; i < t; i++){
                pthread_join(tp[i], NULL);
            }
            tmp = dots;
            dots = next_dots;
            next_dots = tmp;
        }
        
        for (int i = 0; i < row; i++){
            for (int j = 0; j < col; j++){
                //printf("%d", dots[i][j]);
                if (dots[i][j] == 1)
                    putc('O', fpout);
                    //fprintf(fpout, "%c", 'O');
                else
                    putc('.', fpout);
                    //fprintf(fpout, "%c", '.');
            }
            if (i < row-1)
                putc('\n', fpout);
                //fprintf(fpout, "\n");
        }
    }else if (p == 2){
        //printf("in p=2\n");
        bool *ddots, *next_ddots, *ttmp;
        ddots = (bool *) mmap(NULL, sizeof(bool)*row*col, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        next_ddots = (bool *) mmap(NULL, sizeof(bool)*row*col, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        //printf("after mmap\n");
        char element;
        for (int i = 0; i < row; i++){
            for (int j = 0; j < col; j++){
                //printf("%d %d\n", i, j);
                element = getc(fp);
                if (element == 'O')
                    ddots[i*col+j] = 1;
                else
                    ddots[i*col+j] = 0;
                if (j == col-1)
                    getc(fp);
            }
        }
        //printf("before epoch\n");
        for (int k = 0; k < epoch; k++){
            
            int row_l[2], row_r[2], col_l[2], col_r[2];
            if (row >= 2){
                row_l[0] = 0;
                row_r[0] = row/2;
                row_l[1] = row/2 + 1;
                row_r[1] = row-1;
                col_l[0] = 0;
                col_r[0] = col-1;
                col_l[1] = 0;
                col_r[1] = col-1;
            }else if (col >= 2){
                row_l[0] = 0;
                row_r[0] = row - 1;
                row_l[1] = 0;
                row_r[1] = row-1;
                col_l[0] = 0;
                col_r[0] = col/2;
                col_l[1] = col/2+1;
                col_r[1] = col-1;
            }else{
                row_l[0] = 0;
                row_r[0] = row-1;
                row_l[1] = 0;
                row_r[1] = -1;
                col_l[0] = 0;
                col_r[0] = col-1;
                col_l[1] = 0;
                col_r[1] = -1;
            }
            //printf("before fork\n");
            if (fork() == 0){
                int lives;
                for (int x = row_l[0]; x <= row_r[0]; x++){
                    for (int y = col_l[0]; y <= col_r[0]; y++){
                        lives = 0;
                        if (x-1 >= 0 && ddots[(x-1)*col+y] == 1)
                            lives += 1;
                        if (x+1 < row && ddots[(x+1)*col+y] == 1)
                            lives += 1;
                        if (y-1 >= 0 && ddots[x*col+(y-1)] == 1)
                            lives += 1;
                        if (y+1 < col && ddots[x*col+(y+1)] == 1)
                            lives += 1;
                        if (x-1 >= 0 && y-1 >= 0 && ddots[(x-1)*col+(y-1)] == 1)
                            lives += 1;
                        if (x-1 >= 0 && y+1 < col && ddots[(x-1)*col+(y+1)] == 1)
                            lives += 1;
                        if (x+1 < row && y-1 >= 0 && ddots[(x+1)*col+(y-1)] == 1)
                            lives += 1;
                        if (x+1 < row && y+1 < col && ddots[(x+1)*col+(y+1)] == 1)
                            lives += 1;
                        
                        if (ddots[x*col+y] == 0)
                            next_ddots[x*col+y] = transform[lives][0];
                        else
                            next_ddots[x*col+y] = transform[lives][1];
                    }
                }
                exit(0);
            }
            
            if (fork() == 0){
                int lives;
                for (int x = row_l[1]; x <= row_r[1]; x++){
                    for (int y = col_l[1]; y <= col_r[1]; y++){
                        lives = 0;
                        if (x-1 >= 0 && ddots[(x-1)*col+y] == 1)
                            lives += 1;
                        if (x+1 < row && ddots[(x+1)*col+y] == 1)
                            lives += 1;
                        if (y-1 >= 0 && ddots[x*col+(y-1)] == 1)
                            lives += 1;
                        if (y+1 < col && ddots[x*col+(y+1)] == 1)
                            lives += 1;
                        if (x-1 >= 0 && y-1 >= 0 && ddots[(x-1)*col+(y-1)] == 1)
                            lives += 1;
                        if (x-1 >= 0 && y+1 < col && ddots[(x-1)*col+(y+1)] == 1)
                            lives += 1;
                        if (x+1 < row && y-1 >= 0 && ddots[(x+1)*col+(y-1)] == 1)
                            lives += 1;
                        if (x+1 < row && y+1 < col && ddots[(x+1)*col+(y+1)] == 1)
                            lives += 1;
                        
                        if (ddots[x*col+y] == 0)
                            next_ddots[x*col+y] = transform[lives][0];
                        else
                            next_ddots[x*col+y] = transform[lives][1];
                    }
                }
                exit(0);
            }
            
            wait(NULL);
            wait(NULL);
            ttmp = ddots;
            ddots = next_ddots;
            next_ddots = ttmp;
        }
        
        for (int i = 0; i < row; i++){
            for (int j = 0; j < col; j++){
                //printf("%d", dots[i][j]);
                if (ddots[i*col+j] == 1)
                    putc('O', fpout);
                    //fprintf(fpout, "%c", 'O');
                else
                    putc('.', fpout);
                    //fprintf(fpout, "%c", '.');
            }
            if (i < row-1)
                putc('\n', fpout);
                //fprintf(fpout, "\n");
        }
        
    }
    
    
    
    /*for (int i = 0; i < t; i++){
        printf("%d %d %d %d %d\n", i, jobs[i].col_s, jobs[i].col_e, jobs[i].row_s, jobs[i].row_e);
    }*/

    
    
    fclose(fp);
    fclose(fpout);
    
    
    return 0;
}
