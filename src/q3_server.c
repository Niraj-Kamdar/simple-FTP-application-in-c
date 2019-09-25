
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <linux/in.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#define FNAME file1
#define PORT 8010
#define BUFSIZE 128
#define LISTENQ 5

int64_t RTT[LISTENQ];
int64_t MaxRTT;

typedef struct
{
    int sock;
    struct sockaddr address;
    int addr_len;
    int t;
} connection_t;

pthread_mutex_t mutex_fd;
pthread_t jthreads[LISTENQ];
pthread_t threads[LISTENQ];
pthread_attr_t attr;

int64_t gettime(){
    struct timespec tms;
    if (clock_gettime(CLOCK_REALTIME,&tms)) {
        return -1;
    }
    int64_t micros = tms.tv_sec * 1000000;
    micros += tms.tv_nsec/1000;
    if (tms.tv_nsec % 1000 >= 500) {
        ++micros;
    }
    return micros;
}

int64_t max(int64_t RTT[]){
    int64_t Max = RTT[0];
    for(int i=1; i<LISTENQ; i++){
        if(Max < RTT[i]){
            Max = RTT[i];
        }
    }
    return Max;
}


void * jprocess(void * ptr)
{
    int fd, pid, n, size;
    struct timespec tms;
    struct sockaddr_in servaddr;
    char buf[BUFSIZE],fname[50],command[50];

    pthread_t thread;
    struct stat stat_buf;

    int len;
    connection_t * conn;
    long addr = 0;

    if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;
    printf("Handling connection request\n");

    int64_t t1 = gettime();
    send(conn->sock, command, 50, 0);

    recv(conn->sock, command, 50, 0);
    int64_t t2 = gettime();

    printf("RTT of this client is %"PRId64" microseconds\n", t2-t1);

    RTT[conn->t] = t2 - t1;
    pthread_exit((void*) conn->t);
}

void* process(void* ptr){
    int fd, pid, n, size;
    struct timespec tms;
    struct sockaddr_in servaddr;
    char buf[BUFSIZE],fname[50],command[50], strt[50], strRTT[50];

    pthread_t thread;
    struct stat stat_buf;

    int len;
    connection_t * conn;
    long addr = 0;

    if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;
    printf("Handling connection request\n");
    //get file
    recv(conn->sock,fname,50,0);
    printf("File name is %s \n:  ", fname);

    pthread_mutex_lock(&mutex_fd);
    fd=open(fname,O_RDONLY,S_IRUSR);
    fstat(fd, &stat_buf);
    size = stat_buf.st_size;

    printf(" size is %d\n", size);

    printf("\nopened file\n");
    int64_t Tms = gettime();
    sprintf(strt, "%"PRId64"", Tms);
    send(conn->sock, strt, sizeof(strt), 0);
    sprintf(strRTT, "%"PRId64"", MaxRTT);
    send(conn->sock, strRTT, sizeof(strRTT), 0);
    while ( (n = read(fd, buf, BUFSIZE-1)) > 0)
    {
        buf[n] = '\0';
        // printf(".");
        write(conn->sock,buf,n);
    }
    printf("file transfer completed \n");
    pthread_mutex_unlock(&mutex_fd);
    close(conn->sock);
    close(fd);
    pthread_mutex_destroy(&mutex_fd);
    pthread_exit((void*) conn->t);
}

int main(int argc, char **argv)
{
    int listenfd, connfd, fd, pid, n, size, t;
    struct sockaddr_in servaddr;
    char buf[BUFSIZE],fname[50],command[50];
    connection_t **connection = (connection_t **)malloc(LISTENQ*sizeof(connection_t*));
    for(int i = 0; i < LISTENQ; i++) connection[i] = (connection_t *)malloc(sizeof(connection_t));
    pthread_t thread;
    struct stat stat_buf;

    pthread_mutex_init(&mutex_fd, NULL);


    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(PORT);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);
    printf("listening\n");

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc;
    void *status;

    for ( ; ; )
    {

        for(t=0; t<LISTENQ;) {
            connection[t]->sock = accept(listenfd, &connection[t]->address, &connection[t]->addr_len);
            printf("Main: creating thread %d\n", t);

            connection[t]->t = t;
            if (connection[t]->sock <= 0)
            {
                free(connection[t]);
            }
            else{
                pthread_create(&jthreads[t], &attr, jprocess, (void *)connection[t]);
                t++;
            }
        }
        pthread_attr_destroy(&attr);
        for(t=0; t<LISTENQ; t++) {
            pthread_join(jthreads[t], &status);
            printf("Main: completed join with thread %d having a status of %ld \n",t,(long)status);
        }
        MaxRTT = max(RTT);

        printf("Maximum RTT is: %"PRId64" microseconds\n", MaxRTT);
        for(t=0; t<LISTENQ;t++){
            pthread_create(&threads[t], 0, process, (void *)connection[t]);
            pthread_detach(threads[t]);
        }

    }
    return 0;
}

