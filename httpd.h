#ifndef _HTTPD_H_
#define _HTTPD_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<arpa/inet.h>
//#include<linux/in.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<assert.h>
#include<ctype.h>

#define _FD_NUM_ 512
#define _FD_DEFAULT_VAL_ -1

typedef struct fds{
	int epollfd;
	int sockfd;
}fds;

typedef struct select_fd{
	int max_fd;
	int fd_arr[_FD_NUM_];
}select_fd_t;

#define MAX_EVENT_NUMBER 1024
#define _BACK_LOG_ 5
#define _COMM_SIZE_ 1024
#define HTTP_VERSION "http/1.0"
#define MAN_PAGE "index.html"
#endif 
