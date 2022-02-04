#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[512];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    // you don't need to change this.
    int id;
    int wait_for_write;  // used by handle_read to know if the header is read or not.
} request;

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list

const char* accept_read_header = "ACCEPT_FROM_READ";
const char* accept_write_header = "ACCEPT_FROM_WRITE";
const char* ask_id = "Please enter your id (to check your preference order):\n";
const char* ask_pref = "Please input your preference order respectively(AZ,BNT,Moderna):\n";
const char* locked = "Locked.\n";
const char* failed = "[Error] Operation failed. Please try again.\n";

static void init_server(unsigned short port);
// initailize a server, exit for error

static void init_request(request* reqP);
// initailize a request instance

static void free_request(request* reqP);
// free resources used by a request instance

typedef struct {
    int id;          //902001-902020
    int AZ;          
    int BNT;         
    int Moderna;     
}registerRecord;


int handle_read(request* reqP) {
    int r;
    char buf[512];
    

    // Read in request from client
    r = read(reqP->conn_fd, buf, sizeof(buf));
    if (r < 0) return -1;
    if (r == 0) return 0;
    char* p1 = strstr(buf, "\015\012");
    int newline_len = 2;
    if (p1 == NULL) {
       p1 = strstr(buf, "\012");
        if (p1 == NULL) {
            ERR_EXIT("this really should not happen...");
        }
    }
    size_t len = p1 - buf + 1;
    memmove(reqP->buf, buf, len);
    reqP->buf[len - 1] = '\0';
    reqP->buf_len = len-1;
    return 1;
}

int main(int argc, char** argv) {

    // Parse args.
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        exit(1);
    }
    
    //readlock
    struct flock readlock;
    readlock.l_type = F_RDLCK;
    readlock.l_whence = SEEK_SET;
    readlock.l_start = 0;
    readlock.l_len = sizeof(registerRecord);
    readlock.l_pid = getpid();
    
    //writelock
    struct flock writelock;
    writelock.l_type = F_WRLCK;
    writelock.l_whence = SEEK_SET;
    writelock.l_start = 0;
    writelock.l_len = sizeof(registerRecord);
    writelock.l_pid = getpid();
    
    //unlock
    struct flock unlock;
    unlock.l_type = F_UNLCK;
    unlock.l_whence = SEEK_SET;
    unlock.l_start = 0;
    unlock.l_len = sizeof(registerRecord);
    unlock.l_pid = getpid();

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[512];
    int buf_len;
    //read registerRecord
    registerRecord record[20];
    int fd;
    registerRecord input;
    fd = open("registerRecord", O_RDWR);
    //fprintf(stderr, "------\n");
    //pread(fd, &input, sizeof(registerRecord), sizeof(registerRecord));
    //fprintf(stderr, "id=%d AZ=%d BNT=%d Moderna=%d\n", input.id, input.AZ, input.BNT, input.Moderna);
    /*int i = 0;
    while(read(fd, &input, sizeof(registerRecord))){
        fprintf(stderr, "id=%d AZ=%d BNT=%d Moderna=%d\n", input.id, input.AZ, input.BNT, input.Moderna);
        record[i] = input;
        i += 1;
    }*/
    //fprintf(stderr, "%d %d %d %d", record[0].id, record[0].AZ, record[0].BNT, record[0].Moderna);

    // Initialize server
    init_server((unsigned short) atoi(argv[1]));

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);

    fd_set set, ori_set;
    FD_ZERO(&set);
    
    FD_ZERO(&set);
    
    int max_fd;
    FD_SET(svr.listen_fd, &ori_set);
    
    max_fd = svr.listen_fd;
    set = ori_set;
    int Record[20] = {0};
    
    
    while (select(max_fd+1, &set, NULL, NULL, NULL)) {
        // TODO: Add IO multiplexing
        int bound = max_fd;
        for (int i = 0; i <= bound; i++){
                
            if (FD_ISSET(i, &set)){
                if (i == svr.listen_fd){
                    clilen = sizeof(cliaddr);
                    conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
                    if (conn_fd < 0) {
                        if (errno == EINTR || errno == EAGAIN) continue;  // try again
                        if (errno == ENFILE) {
                            (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                            continue;
                        }
                        ERR_EXIT("accept");
                    }
                    FD_SET(conn_fd, &ori_set);
                    //fprintf(stderr, "connfd=%d\n", conn_fd);
                    requestP[conn_fd].conn_fd = conn_fd;
                    strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
                    requestP[conn_fd].wait_for_write = 0;
                    fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
                    //fprintf(stderr, "test!\n");
                    //sprintf(buf, "%s", ask_id);
                    write(requestP[conn_fd].conn_fd, ask_id, strlen(ask_id));
                    if (conn_fd > max_fd)
                        max_fd = conn_fd;
                }else{
                    if (requestP[i].wait_for_write == 0){
                        int ret = handle_read(&requestP[i]); // parse data from client to requestP[conn_fd].buf
                        //fprintf(stderr, "ret = %d\n", ret);
                        if (ret < 0) {
                            fprintf(stderr, "bad request from %s\n", requestP[i].host);
                            continue;
                        }
                        //fprintf(stderr, "inputlen=%d\n", strlen(requestP[i].buf));
                        if (strlen(requestP[i].buf) != 6){
                            write(requestP[i].conn_fd, failed, strlen(failed));
                            close(requestP[i].conn_fd);
                            free_request(&requestP[i]);
                            FD_CLR(i, &ori_set);
                            continue;
                        }
                        int id = atoi(requestP[i].buf);
                        requestP[i].id = id;
                        //fprintf(stderr, "%d", id);
                        if (id > 902020 || id < 902001){
                                //close connection "[Error] Operation failed. Please try again."
                            write(requestP[i].conn_fd, failed, strlen(failed));
                            close(requestP[i].conn_fd);
                            free_request(&requestP[i]);
                            FD_CLR(i, &ori_set);
                            continue;
                        }
#ifdef READ_SERVER
                        //fprintf(stderr, "this is read server\n");
                        readlock.l_start = (id-902001)*sizeof(registerRecord);
                        if (fcntl(fd, F_SETLK, &readlock) == -1){
                            //perror("fcntl error");
                            write(requestP[i].conn_fd, locked, strlen(locked));
                            close(requestP[i].conn_fd);
                            free_request(&requestP[i]);
                            FD_CLR(i, &ori_set);
                            continue;
                        }
#elif defined WRITE_SERVER
                        //fprintf(stderr, "this is write server\n");
                        writelock.l_start = (id-902001)*sizeof(registerRecord);
                        //fprintf(stderr, "lock=%d", fcntl(requestP[i].conn_fd, F_SETLK, &readlock));
                        //fprintf(stderr, "fd=%d", requestP[i].conn_fd);
                        if (fcntl(fd, F_SETLK, &writelock) == -1 || Record[requestP[i].id-902001] == 1){
                            //perror("fcntl error");
                            write(requestP[i].conn_fd, locked, strlen(locked));
                            close(requestP[i].conn_fd);
                            free_request(&requestP[i]);
                            FD_CLR(i, &ori_set);
                            continue;
                        }
#endif
                        Record[requestP[i].id-902001] = 1;
                        char first[10] = {'\0'}, second[10] = {'\0'}, third[10] = {'\0'};
                        pread(fd, &input, sizeof(registerRecord), (requestP[i].id-902001)*sizeof(registerRecord));
                            //fprintf(stderr, "1111");
                        switch (input.AZ) {
                            case 1:
                                strcpy(first, "AZ");
                                break;
                            case 2:
                                strcpy(second, "AZ");
                                break;
                            case 3:
                                strcpy(third, "AZ");
                                break;
                        }
                        switch (input.BNT) {
                            case 1:
                                strcpy(first, "BNT");
                                break;
                            case 2:
                                strcpy(second, "BNT");
                                break;
                            case 3:
                                strcpy(third, "BNT");
                                break;
                        }
                        switch (input.Moderna) {
                            case 1:
                                strcpy(first, "Moderna");
                                break;
                            case 2:
                                strcpy(second, "Moderna");
                                break;
                            case 3:
                                strcpy(third, "Moderna");
                                break;
                        }
                            
                            //fprintf(stderr,"2222");
                            
                        char order[100] = {'\0'};
                        sprintf(order, "Your preference order is %s > %s > %s.\n", first, second, third);
                        write(requestP[i].conn_fd, order, strlen(order));
                        requestP[i].wait_for_write = 1;
#ifdef READ_SERVER
                        //fprintf(stderr, "%s", requestP[i].buf);
                        //sprintf(buf, "%s", ask_id);
                        sprintf(buf,"%s : %s",accept_read_header,requestP[i].buf);
                        write(requestP[i].conn_fd, buf, strlen(buf));
                        Record[requestP[i].id-902001] = 0;
                        unlock.l_start = (requestP[i].id-902001)*sizeof(registerRecord);
                        fcntl(fd, F_SETLK, &unlock);
                        close(requestP[i].conn_fd);
                        free_request(&requestP[i]);
                        FD_CLR(i, &ori_set);
                            
#elif defined WRITE_SERVER
                        write(requestP[i].conn_fd, ask_pref, strlen(ask_pref));
                    }else if (requestP[i].wait_for_write == 1){
                        int id = requestP[i].id;
                        handle_read(&requestP[i]);
                        char *newpref = requestP[i].buf;
                        //fprintf(stderr, "%s", newpref);
                        
                        if (strlen(newpref) != 5 || (int)newpref[1] != 32 || (int)newpref[3] != 32){
                            
                            write(requestP[i].conn_fd, failed, strlen(failed));
                            
                            Record[requestP[i].id-902001] = 0;
                            unlock.l_start = (requestP[i].id-902001)*sizeof(registerRecord);
                            fcntl(fd, F_SETLK, &unlock);
                            close(requestP[i].conn_fd);
                            free_request(&requestP[i]);
                            FD_CLR(i, &ori_set);
                            continue;
                        }
                        
                        int fir = atoi(&newpref[0]), sec = atoi(&newpref[2]), thir = atoi(&newpref[4]);
                        
                        if (fir + sec + thir != 6 || fir * sec * thir == 0 || (fir == 2 && sec == 2 && thir == 2)){
                            write(requestP[i].conn_fd, failed, strlen(failed));
                            
                            Record[requestP[i].id-902001] = 0;
                            unlock.l_start = (requestP[i].id-902001)*sizeof(registerRecord);
                            fcntl(fd, F_SETLK, &unlock);
                            close(requestP[i].conn_fd);
                            free_request(&requestP[i]);
                            FD_CLR(i, &ori_set);
                            continue;
                        }
                        
                        pread(fd, &input, sizeof(registerRecord), (requestP[i].id-902001)*sizeof(registerRecord));
                        input.AZ = fir;
                        input.BNT = sec;
                        input.Moderna = thir;
                        pwrite(fd, &input, sizeof(registerRecord), (requestP[i].id-902001)*sizeof(registerRecord));
                        
                        char first[10] = {'\0'}, second[10] = {'\0'}, third[10] = {'\0'};
                        switch (input.AZ) {
                            case 1:
                                strcpy(first, "AZ");
                                break;
                            case 2:
                                strcpy(second, "AZ");
                                break;
                            case 3:
                                strcpy(third, "AZ");
                                break;
                        }
                        switch (input.BNT) {
                            case 1:
                                strcpy(first, "BNT");
                                break;
                            case 2:
                                strcpy(second, "BNT");
                                break;
                            case 3:
                                strcpy(third, "BNT");
                                break;
                        }
                        switch (input.Moderna) {
                            case 1:
                                strcpy(first, "Moderna");
                                break;
                            case 2:
                                strcpy(second, "Moderna");
                                break;
                            case 3:
                                strcpy(third, "Moderna");
                                break;
                        }
                        
                        char order[100] = {'\0'};
                        sprintf(order, "Preference order for %d modified successed, new preference order is %s > %s > %s.\n", requestP[i].id, first, second, third);
                        write(requestP[i].conn_fd, order, strlen(order));
                            
                            
                            
                        //fprintf(stderr, "%s", requestP[i].buf);
                            //sprintf(buf, "Please enter your id (to check your preference order):");
                        sprintf(buf,"%s : %s",accept_write_header,requestP[i].buf);
                        write(requestP[i].conn_fd, buf, strlen(buf));
                        Record[requestP[i].id-902001] = 0;
                        unlock.l_start = (requestP[i].id-902001)*sizeof(registerRecord);
                        fcntl(fd, F_SETLK, &unlock);
                        close(requestP[i].conn_fd);
                        free_request(&requestP[i]);
                        FD_CLR(i, &ori_set);
                            
                            
#endif
                        
                            
                    }
                }
            }
            
        }
                
        
        
        // Check new connection
        /*clilen = sizeof(cliaddr);
        conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
        if (conn_fd < 0) {
            if (errno == EINTR || errno == EAGAIN) continue;  // try again
            if (errno == ENFILE) {
                (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                continue;
            }
            ERR_EXIT("accept");
        }
        fprintf(stderr, "connfd=%d\n", conn_fd);
        requestP[conn_fd].conn_fd = conn_fd;
        strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
        fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
        //fprintf(stderr, "test!\n");
        //sprintf(buf, "%s", ask_id);
        write(requestP[conn_fd].conn_fd, ask_id, strlen(ask_id));
        */
        //write(requestP[conn_fd].conn_fd, "Your preference order is Moderna > BNT > AZ.", 55);
        

        /*int ret = handle_read(&requestP[conn_fd]); // parse data from client to requestP[conn_fd].buf
        fprintf(stderr, "ret = %d\n", ret);
        if (ret < 0) {
            fprintf(stderr, "bad request from %s\n", requestP[conn_fd].host);
            continue;
        }
        
        int id = atoi(requestP[conn_fd].buf);
        fprintf(stderr, "%d", id);
        char first[10] = {'\0'}, second[10] = {'\0'}, third[10] = {'\0'};
        if (id > 902020 || id < 902001){
            //close connection "[Error] Operation failed. Please try again."
        }
        //fprintf(stderr, "1111");
        switch (record[id-902001].AZ) {
            case 1:
                strcpy(first, "AZ");
                break;
            case 2:
                strcpy(second, "AZ");
                break;
            case 3:
                strcpy(third, "AZ");
                break;
        }
        switch (record[id-902001].BNT) {
            case 1:
                strcpy(first, "BNT");
                break;
            case 2:
                strcpy(second, "BNT");
                break;
            case 3:
                strcpy(third, "BNT");
                break;
        }
        switch (record[id-902001].Moderna) {
            case 1:
                strcpy(first, "Moderna");
                break;
            case 2:
                strcpy(second, "Moderna");
                break;
            case 3:
                strcpy(third, "Moderna");
                break;
        }
        
        //fprintf(stderr,"2222");
        
        char order[100] = {'\0'};
        sprintf(order, "Your preference order is %s > %s > %s.\n", first, second, third);
        write(requestP[conn_fd].conn_fd, order, strlen(order));*/
        
        //fprintf(stderr, "sizeof=%d", sizeof(ask_pref));
        //write(requestP[conn_fd].conn_fd, ask_pref, strlen(ask_pref));
        
        //fprintf(stderr, "%s", order);
        //write(requestP[conn_fd].conn_fd, order, sizeof(order));
        
        /*handle_read(&requestP[conn_fd]);
        char *newpref = requestP[conn_fd].buf;
        fprintf(stderr, "%s", newpref);
        
        record[id-902001].AZ = atoi(&newpref[0]);
        record[id-902001].BNT = atoi(&newpref[2]);
        record[id-902001].Moderna = atoi(&newpref[4]);
        
        switch (record[id-902001].AZ) {
            case 1:
                strcpy(first, "AZ");
                break;
            case 2:
                strcpy(second, "AZ");
                break;
            case 3:
                strcpy(third, "AZ");
                break;
        }
        switch (record[id-902001].BNT) {
            case 1:
                strcpy(first, "BNT");
                break;
            case 2:
                strcpy(second, "BNT");
                break;
            case 3:
                strcpy(third, "BNT");
                break;
        }
        switch (record[id-902001].Moderna) {
            case 1:
                strcpy(first, "Moderna");
                break;
            case 2:
                strcpy(second, "Moderna");
                break;
            case 3:
                strcpy(third, "Moderna");
                break;
        }
        
        sprintf(order, "Preference order for %d modified successed, new preference order is %s > %s > %s.\n", id, first, second, third);
        write(requestP[conn_fd].conn_fd, order, strlen(order));
        
        
        
        
	    
        

    // TODO: handle requests from clients
#ifdef READ_SERVER      
        fprintf(stderr, "%s", requestP[conn_fd].buf);
        //sprintf(buf, "%s", ask_id);
        sprintf(buf,"%s : %s",accept_read_header,requestP[conn_fd].buf);
        write(requestP[conn_fd].conn_fd, buf, strlen(buf));    
#elif defined WRITE_SERVER
        fprintf(stderr, "%s", requestP[conn_fd].buf);
        //sprintf(buf, "Please enter your id (to check your preference order):");
        sprintf(buf,"%s : %s",accept_write_header,requestP[conn_fd].buf);
        write(requestP[conn_fd].conn_fd, buf, strlen(buf));    
#endif

        close(requestP[conn_fd].conn_fd);
        free_request(&requestP[conn_fd]);*/
        set = ori_set;
    }
    free(requestP);
    return 0;
}

// ======================================================================================================
// You don't need to know how the following codes are working
#include <fcntl.h>

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
}

static void free_request(request* reqP) {
    /*if (reqP->filename != NULL) {
        free(reqP->filename);
        reqP->filename = NULL;
    }*/
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
