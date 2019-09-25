
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
#define LISTENQ 5

typedef struct
{
	int sock;
	struct sockaddr address;
	int addr_len;
} connection_t;

pthread_mutex_t mutex_fd;

void * process(void * ptr)
{
    int	fd, pid, n, size;
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
	recv(conn->sock, command, 50, 0);
	printf("command received: %s\n", command);

	if(strcmp(command, "GET") == 0){
		recv(conn->sock,fname,50,0);
		printf("File name is %s \n:  ", fname);

		pthread_mutex_lock(&mutex_fd);
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
		pthread_mutex_unlock(&mutex_fd);
		close(conn->sock);
		close(fd);
	}
	else if(strcmp(command, "PUT") == 0){
		recv(conn->sock,fname,50,0);
		printf("File name is %s :  ", fname);
        pthread_mutex_lock(&mutex_fd);
		fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
		while ( (n = read(conn->sock, buf, BUFSIZE-1)) > 0)
		{
			buf[n] = '\0';
			write(fd,buf,n);
			if( n < BUFSIZE-2)
				break;
		}

		printf("file receiving completed \n");
        pthread_mutex_unlock(&mutex_fd);
		close(conn->sock);
		close(fd);
	}
	else if(strcmp(command, "LIST") == 0){
        struct dirent *de;

	    DIR *dr = opendir(".");
	    if (dr == NULL)
	    {
	        char result[50] = "Could not open current directory\n";
	        printf("%s\n", result);
	        send(conn->sock, result, sizeof(result), 0);
	    }


	    while ((de = readdir(dr)) != NULL){
            char string[50];
            strcpy(string, de->d_name);
            printf("%s\n", string);
            send(conn->sock, string, sizeof(string), 0);
        }

		char stop[] = "STOP";
		send(conn->sock, stop, sizeof(stop), 0);
		closedir(dr);
		printf("file listing completed \n");
		close(conn->sock);
    }
	free(conn);
	pthread_mutex_destroy(&mutex_fd);
	pthread_exit(0);
}

int main(int argc, char **argv)
{
	int	listenfd, connfd, fd, pid, n, size;

	struct sockaddr_in servaddr;
	char buf[BUFSIZE],fname[50],command[50];
	connection_t * connection;
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

