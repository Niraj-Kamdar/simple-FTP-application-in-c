
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

void * process(void * ptr)
{
    int fd, pid, n, size;
    struct sockaddr_in servaddr;
    char buf[BUFSIZE],fname[50],command[50];
    connection_t * connection;
    pthread_t thread;
    struct stat stat_buf;

    int len;
    connection_t * conn;
    long addr = 0;

    if (!ptr) pthread_exit(0);
    conn = (connection_t *)ptr;
    printf("Handling connection request\n");
    recv(conn->sock,fname,50,0);
    printf("File name is %s \n:  ", fname);

    fd=open(fname,O_RDONLY,S_IRUSR);
    fstat(fd, &stat_buf);
    size = stat_buf.st_size;

    printf(" size is %d\n", size);
    printf("\nopened file\n");
    while ( (n = read(fd, buf, BUFSIZE-1)) > 0)
    {
        buf[n] = '\0';
        write(conn->sock,buf,n);
    }
    printf("file transfer completed \n");
    close(conn->sock);
    close(fd);

    free(conn);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int listenfd, connfd, fd, pid, n, size;
    struct sockaddr_in servaddr;
    char buf[BUFSIZE],fname[50],command[50];
    connection_t * connection;
    pthread_t thread;
    struct stat stat_buf;


    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(PORT);

    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    listen(listenfd, LISTENQ);
        printf("listening\n");

    for ( ; ; )
    {
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(listenfd, &connection->address, &connection->addr_len);
        if (connection->sock <= 0)
        {
            free(connection);
        }
        else
        {
            /* start a new thread but do not wait for it */
            pthread_create(&thread, 0, process, (void *)connection);
            pthread_detach(thread);
        }

    }
    return 0;
}

