#include"httpd.h"

void usage(const char* proc)
{
	printf("usage : %s[port]\n", proc);
}

void print_debug(const char *msg)
{
#ifdef _DEBUG_
	printf("%s\n", msg);
#endif
}

void print_log(const char *fun, int line, int err_no, const char * err_str)
{
	printf("[%s : %d][%d][%s]\n", fun, line, err_no, err_str);
}

void clear_header(int client)
{
	char buf[1024];
	memset(buf, '\0', sizeof(buf));
	int ret = 0;
	do{
		ret = get_line(client, buf, sizeof(buf));
	}while(ret>0 && (strcmp(buf, "\n")) != 0);
}

int get_line(int sock, char *buf, int max_len)
{
	print_debug("enter getline");
	if(!buf || max_len<0){
		return -1;
	}
	int i = 0;
	int n = 0;
	char c = '\0';
	while(i< max_len-1 && c != '\n'){
		n = recv(sock, &c, 1, 0);
		if(n > 0){//success
			if(c == '\r'){
				n = recv(sock, &c, 1, MSG_PEEK);//
				if(n > 0 && c == '\n'){//windows
					recv(sock, &c, 1, 0);//delete//??
				}else{
					c = '\n';
				}
			}
			buf[i++] = c;
		}else{//failed
			c = '\n';
		}
	}
	buf[i] = '\0';
	return i;
}

static void bad_request(int client)
{
	char buf[_COMM_SIZE_];
	sprintf(buf, "HTTP/1.0 400 BAD REQUEST\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf, "<p> you are enter msg is bad request</p>");
	send(client, buf, strlen(buf), 0);
}

void echo_errno_to_client(int sock_client, int errno_code)
{
	switch(errno_code)
	{
		case 400://request errno
			//bad_request();
			break;
		case 404://not found
			//not_found();
			break;
		case 500://server_errno
			//server_errno();
			break;
		case 503://servr unavaible
			//server_unavaible();
			break;
		default:
			break;
	}
}

void echo_html(int client, const char *path, unsigned int file_size)
{
	if(!path){
		return;
	}

	int in_fd = open(path, O_RDONLY);
	if(in_fd < 0){
		print_debug("open index.html error");
		//echo_errno_to_client();
		return;
	}
	print_debug("open index.html success");
	char echo_line[_COMM_SIZE_];
	memset(echo_line, '\0', sizeof(echo_line));
	strncpy(echo_line, HTTP_VERSION, strlen(HTTP_VERSION)+1);
	strcat(echo_line, " 200 OK");
	strcat(echo_line, "\r\n\r\n");
	send(client, echo_line, strlen(echo_line), 0);
	print_debug("send echo head success");
	if(sendfile(client, in_fd, NULL, file_size) < 0){
		print_debug("send_file error");
	//	echo_errno_to_client();
		close(in_fd);
		return;
	}
	print_debug("send_file success");
	close(in_fd);
}

void exe_cgi(int sock_client, const char *path, const char *method, const char *query_string)
{
	print_debug("enter cgi\n");
	char buf[_COMM_SIZE_];
	int numchars = 0;
	int content_length = -1;
	//pipe
	int cgi_input[2] = {0,0};
	int cgi_output[2] = {0,0};
	//child proc
	pid_t id;
	print_debug(method);
	if(strcasecmp(method, "GET")==0){//GET
		clear_header(sock_client);
	}else{//POST
		do{
			memset(buf, '\0', sizeof(buf));
			numchars = get_line(sock_client, buf, sizeof(buf));
			if(strncasecmp(buf, "Content-Length:", strlen("Content-Length:"))==0){
				content_length = atoi(&buf[16]); // content-length: atoi(&buf[16])
			}
		}while(numchars>0 && strcmp(buf, "\n")!=0);
		if(content_length == -1){
			//echo_errno_to_client();
			return;
		}
	}
	memset(buf,'\0', sizeof(buf));
	strcpy(buf, HTTP_VERSION);
	strcat(buf, " 200 OK\r\n\r\n");
	send(sock_client, buf, strlen(buf), 0);

	if(pipe(cgi_input) == -1){//pipe errno
		//echo_errno_to_client();
		return;
	}
	if(pipe(cgi_output) == -1){//pipe errno
		close(cgi_input[0]);
		close(cgi_input[1]);
		//echo_errno_to_client();
		return;
	}

	if( (id = fork()) < 0 ){//fork errno
		close(cgi_input[0]);
		close(cgi_input[1]);
		close(cgi_output[0]);
		close(cgi_output[1]);
		//echo_errno_to_cleint();
		return;
	}else if( id == 0 ){//child proc
		print_debug("enter exe_cgi child");
		char method_env[_COMM_SIZE_/10];
		char query_env[_COMM_SIZE_];
		char content_len_env[_COMM_SIZE_];
		memset(method_env, '\0', sizeof(method_env));
		memset(query_env, '\0', sizeof(query_env));
		memset(content_len_env, '\0', sizeof(content_len_env));
		close(cgi_input[1]);
		close(cgi_output[0]);
		//redir
		dup2(cgi_input[0], 0);
		dup2(cgi_output[1], 1);

		sprintf(method_env, "REQUEST_METHOD=%s", method);
		putenv(method_env);
		if(strcasecmp("GET", method) == 0){//GET
			sprintf(query_env, "QUERY_STRING=%s", query_string);
			putenv(query_env);
		}else{//POST
			sprintf(content_len_env, "CONTENT_LENGTH=%d", content_length);
			putenv(content_len_env);
		}

		execl(path, path, NULL);
		exit(1);
	}else{//father
		print_debug("enter exe_cgi father");
		close(cgi_input[0]);
		close(cgi_output[1]);
		int i = 0;
		char c = '\0';
		if(strcasecmp("POST", method) == 0){
			for(; i<content_length; i++){
				recv(sock_client, &c, 1, 0);
				write(cgi_input[1], &c, 1);
			}
		}
		while(read(cgi_output[0], &c, 1) > 0){
			send(sock_client, &c, 1, 0);
		}
		close(cgi_input[1]);
		close(cgi_output[0]);

		waitpid(id, NULL, 0);//block
	}


}

void* accept_request(void *arg)
{
	print_debug("get a new connect...");
	pthread_detach(pthread_self());//detach pthread

	int cgi = 0;
#ifdef _EPOLL_
	int sock_client = ((struct fds*)arg)->sockfd;
	int epolld = ((fds*)arg)->epollfd;
	print_debug("epoll sock is ready....");
#endif
#ifdef _PTHREAD_
	int sock_client = (int)arg;
	printf("sock_client : %d\n", sock_client);
#endif
#ifdef _SELECT_
	int sock_client = (int)arg;
	printf("sock_client : %d\n", sock_client);
#endif
	char *query_string = NULL;
	char method[_COMM_SIZE_/10];
	char url[_COMM_SIZE_];
	char buffer[_COMM_SIZE_];
	char path[_COMM_SIZE_];
	memset(buffer, '\0', sizeof(buffer));
	memset(url, '\0', sizeof(url));
	memset(method, '\0', sizeof(method));
	memset(path, '\0', sizeof(path));
//#ifdef _DEBUG_
//	while(get_line(sock_client, buffer, sizeof(buffer)) > 0){
//		printf("%s",buffer);
//		fflush(stdout);
//	}
//	printf("\n");
//#endif
	if(get_line(sock_client, buffer, sizeof(buffer)) < 0){
		//echo_error__to_client();
		return (void *)-1;
	}

	int i = 0;
	int j = 0;//buffer line index
	//get method
	while(!isspace(buffer[j]) && i < sizeof(method)-1 && j<sizeof(buffer)){
		method[i] = buffer[j];
		i++;
		j++;
	}

	if(strcasecmp(method, "GET") && strcasecmp(method, "POST")){
		//echo_errno_to_client();
		return NULL;
	}
	//clear space point useful start
	while(isspace(buffer[j]) && j < sizeof(buffer)){
		j++;
	}
	//get url
	i = 0;
	while(!isspace(buffer[j]) && i< sizeof(url)-1 && j<sizeof(buffer)){
		url[i] = buffer[j];
		i++;
		j++;
	}

	printf("method : %s\n", method);
	printf("url : %s\n", url);


	if(strcasecmp(method, "POST") == 0){
		cgi = 1;
	}

	if(strcasecmp(method, "GET") == 0){
		query_string = url;
		while(*query_string != '?' && *query_string != '\0'){
			query_string++;
		}
		if(*query_string == '?'){//url =/xxx/xxx + arg
			*query_string = '\0';
			query_string++;
			cgi = 1;
		}
	}
	sprintf(path, "htdocs%s", url);
	if(path[strlen(path)-1] == '/'){
		strcat(path, MAN_PAGE);
	}

	print_debug(path);
 
	struct stat st;
 	if(stat(path, &st) < 0){//failed, does not exist
		clear_header(sock_client);
		//echo_errno_to_cllient();
	}else{//file exist
		if(S_ISDIR(st.st_mode)){
			strcat(path,"/");
			strcat(path,"MAN_PAGE");
		}else if(st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH){
			cgi = 1;
		}else{
		}
		if(cgi){
			exe_cgi(sock_client, path, method, query_string);
		}else{
			clear_header(sock_client);
			print_debug("begin enter our echo_html");
			echo_html(sock_client, path, st.st_size);
		}
	}

	close(sock_client);
	return NULL;
}

//if success return sock
//else exit process
int startup(int port)
{
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock == -1){
		print_log(__FUNCTION__, __LINE__, errno, strerror(errno));
		exit(1);
	}
	
	//reuse port
	int flag = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &flag,sizeof(flag));//

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t len = sizeof(local);

	if(bind(listen_sock, (struct sockaddr*)&local, len) == -1){
		print_log(__FUNCTION__, __LINE__, errno, strerror(errno));
		exit(2);
	}

	if(listen(listen_sock, _BACK_LOG_) == -1){
		print_log(__FUNCTION__, __LINE__, errno, strerror(errno));
		exit(3);
	}

	return listen_sock;//success
}

int setnoblock(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void add_event_fd(int epollfd, int fd, int flag)
{
	struct epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	if(flag){
		event.events |= EPOLLONESHOT;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	setnoblock(fd);
}

void et(struct epoll_event *events, int number, int epollfd, int listenfd)
{
	int i = 0;
	for(;i<number; i++){
		int sockfd = events[i].data.fd;
		if(sockfd == listenfd){
			struct sockaddr_in client_address;
			socklen_t client_addrlength = sizeof(client_address);
			int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
			add_event_fd(epollfd, connfd , 1);
		}else if (events[i].events & EPOLLIN){
			printf("we are in et else if");
			pthread_t new_thread;
			struct fds fd_for_new_accept;
			fd_for_new_accept.epollfd = epollfd;
			fd_for_new_accept.sockfd = sockfd;
			pthread_create(&new_thread, NULL, accept_request, (void *)&fd_for_new_accept);
		}else{
			//nothing
		}
	}
}

void add_read_fd(fd_set *set, select_fd_t *select_fd)
{
	int i = 0;
	select_fd->max_fd = _FD_DEFAULT_VAL_;
	for(; i<_FD_NUM_; ++i){
		if(select_fd->fd_arr[i] != _FD_DEFAULT_VAL_){
			FD_SET(select_fd->fd_arr[i], set);
			if(select_fd->fd_arr[i] > select_fd->max_fd){
				select_fd->max_fd = select_fd->fd_arr[i];
			}
		}
	}
}

void delete_new_fd(select_fd_t *select_fd, int fd)
{
	int i = 1;
	for(; i< _FD_NUM_; ++i){
		int curr_fd = select_fd->fd_arr[i];
		if(curr_fd == fd){
			select_fd->fd_arr[i] = _FD_DEFAULT_VAL_;
		}
	}
}

int add_new_fd(select_fd_t *select_fd, int new_fd)
{
	int i = 1;
	for(;i<_FD_NUM_;++i){
		if(select_fd->fd_arr[i] == _FD_DEFAULT_VAL_){//NO USE
			select_fd->fd_arr[i] = new_fd;
			return 0;//ok
		}
	}
	return 1;//failed
}

void init_select_set(select_fd_t *select_fd, int listen_fd)
{
	int i = 1;
	select_fd->max_fd = listen_fd;
	select_fd->fd_arr[0] = listen_fd;
	for(; i<_FD_NUM_; ++i){
		select_fd->fd_arr[i] = _FD_DEFAULT_VAL_;
	}
}

int main(int argc, char *argv[])
{
	if(argc != 2){
		usage(argv[0]);
		exit(1);
	}
	int port = atoi(argv[1]);
	printf("%d\n",port);

	int sock = startup(port);
#ifdef _EPOLL_
	struct epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);
	if(epollfd == -1){
		print_debug("epoll_create is faile");
		return -1;
	}
	add_event_fd(epollfd, sock, 0);

	while(1){
		int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if(ret < 0){
			print_debug("epool_wait id faile");
			break;
		}
		et(events, ret, epollfd, sock);
	}
	close(epollfd);
#endif
#ifdef _SELECT_
	select_fd_t select_set;
	init_select_set(&select_set, sock);

	fd_set r_set;

	while(1){
		FD_ZERO(&r_set);
		add_read_fd(&r_set, &select_set);
		struct timeval timeout = {0,0};

		switch(select(select_set.max_fd+1, &r_set, NULL, NULL, &timeout)){
			case 0://timeout
				perror("timeout");
				break;
			case -1://error
				perror("select");
				break;
			default://ok
				{
					int i = 0;
					for(; i<_FD_NUM_;++i){
						int fd = select_set.fd_arr[i];
						if(i ==0 && FD_ISSET(fd, &r_set)){//new connect
							struct sockaddr_in client;
							socklen_t client_length = sizeof(client);
							int new_fd = accept(fd, (struct sockaddr*)&client, &client_length);
							if(new_fd == -1){
								perror("accept");
								continue;
							}
							if(0==add_new_fd(&select_set, new_fd)){
								//do nothing
							}else{//add error, arr is full
								close(fd);
							}
							continue;
						}
						if(fd != _FD_DEFAULT_VAL_ && FD_ISSET(fd, &r_set)){//other read fd
							pthread_t new_thread;
							pthread_create(&new_thread, NULL, accept_request, (void *)fd);
							delete_new_fd(&select_set, fd);
						}
					}
				}
				break;//default
		}
	}
#endif
#ifdef _PTHREAD_
	struct sockaddr_in client;
	socklen_t len = 0;

	while(1){
		int  new_sock = accept(sock, (struct sockaddr*)&client, &len);
		if(new_sock < 0){
			print_log(__FUNCTION__, __LINE__, errno, strerror(errno));
			continue;
		}
		pthread_t new_thread;
		pthread_create(&new_thread, NULL, accept_request, (void *)new_sock);
	}
#endif
	close(sock);
	return 0;
}
