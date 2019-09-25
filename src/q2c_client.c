
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

#define FNAME file1
#define PORT 8010
#define BUFSIZE 128
#define LISTENQ 10

typedef struct
{
    int sock;
    struct sockaddr address;
    int addr_len;
} connection_t;

pthread_t threads[LISTENQ];
pthread_attr_t attr;


char *my_itoa(int num, char *str)
{
    if(str == NULL)
    {
            return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}

void * process(void * ptr)
{
    int sockfd, fd, n, size,count=0;
    long int size1,size2;
    char buf[BUFSIZE], fname[50], command[50], st[3];
    struct sockaddr_in servaddr;
    struct stat stat_buf;
    int t = (int)ptr;

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(1);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0)
        exit(1);

    printf("22\n");

    if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
        printf("doubt\n");
        exit(1);
    }

    printf("connection established\n");
    strcpy(fname, "50MB");
    my_itoa(t, st);
    strcat(fname, st);
    strcat(fname, ".zip");
    send(sockfd,"50MB.zip",50,0);
    fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
    while ( (n = read(sockfd, buf, BUFSIZE-1)) > 0)
    {
        buf[n] = '\0';
        write(fd,buf,n);
        if( n < BUFSIZE-2)
            break;
    }

    printf("file receiving completed \n");
    close(sockfd);
    close(fd);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    void* status;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for ( int i=0; i<LISTENQ ; i++ )
    {
        pthread_create(&threads[i], 0, process, (void*) i);

    }
    pthread_attr_destroy(&attr);
    for(int t=0; t<LISTENQ; t++) {
        pthread_join(threads[t], &status);
        printf("Main: completed join with thread %d having a status of %ld \n",t,(long)status);
    }
    return 0;
}

