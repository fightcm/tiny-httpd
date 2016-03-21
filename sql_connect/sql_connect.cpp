#include"sql_connect.h"

sql_connect::sql_connect(const std::string &_host, const std::string &_user, const std::string &_passwd, const std::string &_db)
{
	this->mysql_base = mysql_init(NULL);
	this->res = NULL;
	this->host = _host;
	this->user = _user;
	this->passwd = _passwd;
	this->db = _db;
}
sql_connect::~sql_connect()
{
	close_connect();
	if(res != NULL){
		free(res);
	}
}
void sql_connect::show_info()
{
	std::cout<<mysql_get_client_info()<<std::endl;
}

bool sql_connect::begin_connect()
{
	std::cout<<" connect into"<<std::endl;
	if(mysql_real_connect(mysql_base, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), 3306, NULL, 0) == NULL){
		std::cout<<"connect failed"<<std::endl;
		return false;
	}else{
		std::cout<<" connect success"<<std::endl;
	}
	return true;
}

bool sql_connect::close_connect()
{
	mysql_close(mysql_base);
}

bool sql_connect::select_sql(std::string field_name[], std::string _out_data[][5], int &_out_row)
{
	std::string sql = "SELECT * FROM student";
	if(mysql_query(mysql_base, sql.c_str()) == 0){
	}else{
	}

	res = mysql_store_result(mysql_base);
	int row_num = mysql_num_rows(res);
	int field_num = mysql_num_fields(res);
	_out_row = row_num;

	MYSQL_FIELD *fd = NULL;
	int i = 0;
	for(; fd = mysql_fetch_field(res);){
		field_name[i++] = fd->name;
	}

	for(int index = 0; index < row_num; index++){
		MYSQL_ROW _row = mysql_fetch_row(res);
		if(_row){
			int start = 0;
			for(; start<field_num; start++){
				_out_data[index][start] = _row[start];
			}
		}
	}
	return true;
}

bool sql_connect::insert_sql(const std::string &data)
{
	std::string sql = "INSERT INTO student (name, age, school, hobby) values";
	sql += "(";
	sql +=data;
	sql +=");";
	std::cout<<sql.c_str()<<std::endl;
	if(mysql_query(mysql_base, sql.c_str()) == 0){
		std::cout<<"query success!"<<std::endl;
		return true;
	}else{
		std::cerr<<"query failed!"<<std::endl;
		return false;
	}
}

//int main()
//{
//	std::string _sql_data[1024][5];
//	std::string header[5];
//	int curr_row = -1;
//	const std::string _host = "127.0.0.1";
//	const std::string _user = "myuser";
//	const std::string _passwd = "mypassword";
//	const std::string _db   = "remote_db";
//	const std::string data  = "'hill', 24, 'xgd', 'sleep'";
//	sql_connect conn(_host, _user, _passwd, _db);
//	conn.begin_connect();
//	conn.insert_sql(data);
//	std::cout<<" insert success"<<std::endl;
//	conn.select_sql(header,_sql_data, curr_row);
//	std::cout<<" select success"<<std::endl;
//	sleep(1);
//	for(int i = 0; i<5; i++){
//		std::cout<<header[i]<<"\t";												}
//	std::cout<<std::endl;
//	for(int i = 0; i<curr_row; i++){
//		for(int j=0; j<5; j++){
//			std::cout<<_sql_data[i][j]<<"\t";
//		}
//		std::cout<<std::endl;
//	}
//	//conn.show_info();
//}

