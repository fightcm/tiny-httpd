#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include "sql_connect.h" //sql_connect.h

const std::string _remote_ip = "127.0.0.1";
const std::string _remote_user = "myuser";
const std::string _remote_passwd = "mypassword";
const std::string _remote_db   = "remote_db";

int main()
{
	int content_length = -1;
	char method[1024];
	char query_string[1024];
	char post_data[4096];
	memset(method, '\0', sizeof(method));
	memset(query_string, '\0', sizeof(query_string));
	memset(post_data, '\0', sizeof(post_data));

	std::cout<<"<html>"<<std::endl;
	std::cout<<"<head>show student</head>"<<std::endl;
	std::cout<<"<body>"<<std::endl;
	const std::string _host = _remote_ip;
	const std::string _user = _remote_user;
	const std::string _passwd = _remote_passwd;
	const std::string _db   = _remote_db;

	std::string _sql_data[1024][5];
	std::string header[5];
	int curr_row = -1;
	sql_connect conn(_host, _user, _passwd, _db);
	conn.begin_connect();
	conn.select_sql(header,_sql_data, curr_row);
	std::cout<<"<table border=\"1\">"<<std::endl;
	std::cout<<"<tr>"<<std::endl;
	for(int i = 0; i<5; i++){
		std::cout<<"<th>"<<header[i]<<"</th>"<<std::endl;
	}
	std::cout<<"</tr>"<<std::endl;

	for(int i = 0; i<curr_row; i++){
		std::cout<<"<tr>"<<std::endl;
		for(int j=0; j<5; j++){
			std::cout<<"<td>"<<_sql_data[i][j]<<"</td>"<<std::endl;;
		}
		std::cout<<"</tr>"<<std::endl;
	}
	std::cout<<"</table>"<<std::endl;


	std::cout<<"</body>"<<std::endl;
	std::cout<<"</html>"<<std::endl;
}
