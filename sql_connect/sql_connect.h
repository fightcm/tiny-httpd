#ifndef _SQL_CONNECT_H_
#define _SQL_CONNECT_H_

#include<iostream>
#include<string>
#include<stdlib.h>
#include"mysql.h"

class sql_connect{
	public:
		 sql_connect(const std::string &_host, const std::string &_user, const std::string &_passwd, const std::string &_db);
		//sql_connect(const std::string &_host, const std::string &_user, const std::string &_passwd, const std::string _db);
		~sql_connect();
		void show_info();
		bool begin_connect();
		bool close_connect();
		bool select_sql(std::string [], std::string _out_str[][5], int &_out_row);
		bool insert_sql(const std::string &data);
	private:
		MYSQL_RES *res;
		MYSQL *mysql_base;
		std::string host;
		std::string user;
		std::string passwd;
		std::string db;
};

#endif
