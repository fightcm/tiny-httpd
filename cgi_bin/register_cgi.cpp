#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "sql_connect.h" 

const std::string _remote_ip = "127.0.0.1";
const std::string _remote_user = "myuser";
const std::string _remote_passwd = "mypassword";
const std::string _remote_db   = "remote_db";

static void function(char *const query)
{
	char name[64];
	char age[64];
	char school[64];
	char hobby[64];
	memset(name,'\0', sizeof(name));
	memset(age,'\0', sizeof(age));
	memset(school,'\0', sizeof(school));
	memset(hobby,'\0', sizeof(hobby));
	printf("aaaaaaaaaaa\n");
	int i = 0;
	while(query[i] != '\0'){
		if(query[i] == '=' || query[i] == '&' ){
			query[i] = ' ';
		}
		i++;
	}

	printf("sssssssssss\n");
	sscanf(query, "%*s %s %*s %s %*s %s %*s %s", name, age, school, hobby);
	
	printf("name : %s\n", name);
	printf("age : %s\n", age);
	printf("school : %s\n", school);
	printf("hobby : %s\n", hobby);

	int int_age = atoi(age);
	std::string insert_data;//= "name, int_age, school, hobby";

	insert_data += "'";
	insert_data += name;
	insert_data += "',";
	insert_data += age;
	insert_data += ",'";
	insert_data += school;
	insert_data += "','";
	insert_data += hobby;
	insert_data += "'";



	const std::string _host = _remote_ip;
	const std::string _user = _remote_user;
	const std::string _passwd = _remote_passwd;
	const std::string _db   = _remote_db;
	sql_connect conn(_host, _user, _passwd, _db);
	conn.begin_connect();
	conn.insert_sql(insert_data);
}

int main()
{
	std::cout<<"<html>"<<std::endl;
	std::cout<<"<head>show student</head>"<<std::endl;
	std::cout<<"<body>"<<std::endl;
	int content_length = -1;
	char method[128];
	char query_string[1024];// = "name=yy&age=11&school=xjd&hobby=code";
	char post_data[4096];
	memset(method, '\0', sizeof(method));
	memset(query_string, '\0', sizeof(query_string));
	memset(post_data, '\0', sizeof(post_data));
	strcpy(method, getenv("REQUEST_METHOD"));
	//printf("query_string : > %s\n",query_string);

	if(strcasecmp(method, "GET") == 0){
		strcpy(query_string, getenv("QUERY_STRING"));
	    printf("query_string : > %s\n",query_string);
		printf("get query_string to register...\n");
		function(query_string);

	}else if(strcasecmp("POST", method) == 0){// post
		content_length = atoi(getenv("CONTENT_LENGTH"));
		int i = 0;
		for(; i<content_length; i++){
			read(0,&post_data[i], 1);
		}
		post_data[i] =  '\0';
	    printf("post_data : > %s\n",post_data);
		printf("post post_data to register..\n");
		function(post_data);
	}else{
		//do nothing
		return 1;
	}



	std::cout<<"</body>"<<std::endl;
	std::cout<<"</html>"<<std::endl;
	return 0;
}
