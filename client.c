#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
 
#define MAXBUF 100
#define CLIENT_PORT 2345
 
int main(int argc, const char *argv[])
{
    
 
    char *server = "127.0.0.1";
    char *serverport = "1234";
    char *echostring = "helloworld";
 
    struct addrinfo client,*servinfo;
    memset(&client,0,sizeof(client));
    client.ai_family = AF_INET;
    client.ai_socktype = SOCK_DGRAM;
    client.ai_protocol= IPPROTO_UDP;
 
    if(getaddrinfo(server,serverport,&client,&servinfo)<0)
    {
        printf("error in getaddrinfo");
        exit(-1);
    }
 
    int sockfd = socket(servinfo->ai_family,servinfo->ai_socktype,servinfo->ai_protocol);
    if(sockfd <0)
    {
        printf("error in socket create");
        exit(-1);
    }

    /*struct sockaddr_in client_addr;*/
    /*bzero(&client_addr, sizeof(struct sockaddr_in));*/
    /*client_addr.sin_family = AF_INET; */
    /*client_addr.sin_port = htons(CLIENT_PORT);*/
    /*client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");*/

    /*if (bind(sockfd, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_in)) == -1) */
    /*{*/
        /*perror("error bind");*/
        /*exit(-1);*/
    /*}*/
    /*else*/
    /*{*/
        /*printf("IP and port bind success \n");*/
    /*}*/

    char bufmsg[1024];
    bzero(bufmsg, sizeof(bufmsg));
    int i;
    for (i = 0; i < 10; i++){
        sprintf(bufmsg, "hello word%d", i);
        ssize_t numBytes = sendto(sockfd, bufmsg, strlen(bufmsg), 0,servinfo->ai_addr,servinfo->ai_addrlen);
        if(numBytes<0)
        {
            printf("error in send the data");
        }
        else
        {
            printf("success send data\n");
        }
        /*sleep(1);*/
    }
 
    close(sockfd);
    return 0;
}

