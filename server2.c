#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include<netdb.h>
#include <assert.h>


#define START_ERROR 1
#define SERVER_PORT 1234
#define MAXEVENTS 100


void handler_message_received(char *buf);
void accept_client(int epfd, int fd);

int main(int argc, char **argv)
{

    struct sockaddr_in server_addr;

    /*创建socket*/
    int listener;
    if ((listener = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket create failed ！");
        exit(1);
    }
    else
    {
        printf("socket create success, socket: %d \n", listener);
    }

    
    int opt=SO_REUSEADDR;
    if(setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0){
        perror("error setsockopt\n");
        exit(START_ERROR);
    }

    if(fcntl(listener, F_SETFL, fcntl(listener, F_GETFD, 0)|O_NONBLOCK) == -1){
        perror("error set O_NONBLOCK\n");
        exit(START_ERROR);
    }
   
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listener, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1) 
    {
        perror("error bind");
        exit(START_ERROR);
    }
    else
    {
        printf("IP and port bind success \n");
    }


    
    int epfd = epoll_create(1024);
    if(epfd == -1){
        perror("error epoll create");
        exit(START_ERROR);
    }

    struct epoll_event ev; 
    ev.events = EPOLLIN;
    ev.data.fd = listener;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listener, &ev) < 0) 
    {
        fprintf(stderr, "epoll set insertion error: fd=%d\n", listener);
        exit(START_ERROR);
    }
    else
    {
        printf("listen socket added in  epoll success \n");
    }

    
    int nfds;
    struct epoll_event events[MAXEVENTS];
    while(1){
        nfds = epoll_wait(epfd, events, 100, -1);

        if(nfds == -1){
            perror("error epoll wait");
            break;
        }

        int i;
        for (i = 0; i < nfds; i++){
            if(events[i].data.fd == listener){
                printf("new client connected.\n");
                accept_client(epfd, listener);
            }
            if(events[i].events & EPOLLIN){
                printf("new message received from %d.\n", events[i].data.fd);
                int nread=0;
                char buf[1024];
                bzero(buf, sizeof(buf));
                do {
                    nread = read(events[i].data.fd ,buf, 1024);
                    if(nread != -1){
                        handler_message_received(buf);
                        bzero(buf, sizeof(buf));
                    }
                }while (nread != -1);
            }
        }
    }
    
}

void accept_client(int epfd, int fd){

    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    char buf[1024];
    int ret = recvfrom(fd, buf,1024, 0, (struct sockaddr *)&client_addr, &addr_size);

    handler_message_received(buf);
    
    int newfd = socket(PF_INET, SOCK_DGRAM, 0);
    int opt1 = SO_REUSEADDR;
    setsockopt(newfd, SOL_SOCKET, SO_REUSEADDR, &opt1, sizeof(opt1));
    if(fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFD, 0)|O_NONBLOCK) == -1){
        perror("error set O_NONBLOCK for new client fd\n");
        return;
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    if (bind(newfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1) 
    {
        perror("error bind server addr");
        return;
    } 
    else
    {
        printf("IP and port bind success \n");
    }

    connect(newfd, (struct sockaddr*) &client_addr, sizeof(struct sockaddr));

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = newfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &ev) < 0) 
    {
        fprintf(stderr, "epoll set insertion error: fd=%d\n", newfd);
        return;
    }
    else
    {
        printf("listen socket added in  epoll success \n");
    }
    
}

void handler_message_received(char *buf){
    printf("message is:%s\n", buf);
}
