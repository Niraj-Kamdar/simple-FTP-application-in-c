#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<net/if_arp.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#define FNAME file1
#define PORT 8010
#define BUFSIZE 128

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

int main(int argc, char **argv)
{
    int sockfd, fd, n, size,count=0;
    int64_t Tmr;
    char Tms[50];
    long int size1,size2;
    char buf[BUFSIZE], fname[50], command[50], sRTT[50];
    struct sockaddr_in servaddr;
    struct stat stat_buf;
    char* end;

    if (argc != 2) {
    printf("Usage: %s server_address", argv[0]);
    exit(1);
    }

    printf("11\n");
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(1);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        exit(1);

    printf("22\n");

    if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
        exit(1);
    printf("connection established\n");

    recv(sockfd, command, 50, 0);
    send(sockfd, command, 50, 0);
    printf("Enter the name of the file u want to receive : ");
    scanf("%s",fname);
    send(sockfd,fname,sizeof(fname),0);
    recv(sockfd, Tms, sizeof(Tms), 0);
    recv(sockfd, sRTT, sizeof(sRTT), 0);
    long long lTms = strtoll(Tms, &end, 10);
    long long RTT = strtoll(sRTT, &end, 10);
    fd=open(fname,O_WRONLY|O_CREAT,S_IRWXU);
    while ( (n = read(sockfd, buf, BUFSIZE-1)) > 0)
    {
        buf[n] = '\0';
        write(fd,buf,n);
        if( n < BUFSIZE-2){
            Tmr = gettime();
            break;
        }
    }

    printf("RTT is %lld microseconds\n", RTT);
    printf("Tms is %lld microseconds\n", lTms);
    printf("Tmr is %"PRId64" microseconds\n", Tmr);
    printf("file receiving completed in %lld microseconds\n", (long long)Tmr - lTms);
    close(sockfd);
    close(fd);
    exit(0);
}
