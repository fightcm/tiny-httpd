#include"demo_client.h" 

void usage(const char *msg)
{
	printf("Usage : %s[ip][port]\n", msg);
}

int main( int argc, char *argv[])
{
	if(argc != 3){
		usage(argv[0]);
		exit(1);
	}
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		perror("socket");
		exit(1);
	}

	int port = atoi(argv[2]);

	struct sockaddr_in remote_server;
	remote_server.sin_family = AF_INET;
	remote_server.sin_port = htons(port);
	remote_server.sin_addr.s_addr = inet_addr(argv[1]);
	if(connect(sock, (struct sockaddr*)&remote_server, sizeof(remote_server)) < 0){
		perror("connect");
		exit(1);
	}
	return 0;

}
