#include "handle.h"
#include<iostream>
#include <string.h>
#include<cstring>
handle::handle()
{
	//连接数据库
	  this->handleConnectMysql();

	  //初始化userinfo
	  this->user.uid = "";
	  this->user.pwd = "";
	 
}

void handle::handleConnectMysql()
{
	//初始化mysql对象
	this->mysql = mysql_init(NULL);

	string host = "localhost";
	string user = "yemaosi";
	string passwd = "12345678";
	string db = "campusApp";
	unsigned int port = 3306;

	//链接数据库
	mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), db.c_str(), port, nullptr, 0);
	if (mysql == nullptr)
	{
		fprintf(stderr, "%s\n", mysql_error(mysql));
		cout << "mysql connect failed" << endl;
	}
	else
	{
		cout << "mysql connect success" << endl;
	}

	//设置编码格式
	mysql_set_character_set(mysql, "utf8");

}

void handle::UserIsFriend(const char* buf, searchUser* user)
{

	//获取用户id和myid

	this->getUidAndMyid(buf,user);


	//判断uid是否存在数据库

	string sql = "select * from userinfo where userid=" + user->uid;

	int rows = this->handleQueryRet(sql.c_str());

	if (rows == -1)
	{
		//异常
		user->msgType = MYSQL_ERROR;
	}
	else if (rows == 0)
	{
		//用户不存在  
		user->msgType = SEARCH_USER_NO_EXIST;
	}
	else if (rows == 1)
	{
		//用户存在  

		//判断用户是否与对方是好友关系

		sql = "select * from friendsinfo where id =" + user->myid + " and friendid = (select userid from userinfo where userid =" + user->uid + " )";

		rows = this->handleQueryRet(sql.c_str());

		sql = "select * from friendsinfo where id = " + user->uid + " and friendid = (select userid from userinfo where userid = " + user->myid + ")";

		int rows2 = this->handleQueryRet(sql.c_str());

		if (rows == -1 || rows2 == -1)
		{
			//异常
			user->msgType = MYSQL_ERROR;
		}
		else if (rows2 != 0 || rows != 0)
		{
			user->msgType = SEARCH_USER_FRIEND;
		}
		else
		{
			user->msgType = SEARCH_USER_EXIST;
		}
	}

}

void handle::handleAddUser(const char* buf, searchUser* user)
{
	//获取myid和uid

	this->getUidAndMyid(buf, user);


	//查询uid是否在线

	string sql = "select * from userinfo where userid =" + user->uid + " and logonstatus = 1";


	int rows = this->handleQueryRet(sql.c_str());

	//如果rows==-1  出现异常
	//rows == 0 用户不在线 将消息加入消息缓存表
	//rows==1 用户在线 进行转发
	 if (rows == -1 )
	 {
		 //出现异常
		 user->msgType = MYSQL_ERROR;
	 }
	 else if(rows == 0)
	 {
		 //用户不在线
		 user->msgType = ADD_USER_NO_ONLINE;
	 }
	 else if (rows == 1)
	 {
		 //用户在线 转发
		 user->msgType = ADD_USER_ONLINE;
	 }

}

void handle::handleAddFriend(string sql)
{
	int ret =mysql_query(mysql, sql.c_str());
	if (ret == 0)
	{
		cout << "插入成功" << endl;
	}
	else
	{
		cout << "插入失败" << endl;
	}
}

void handle::handleChatRequest(const char* buf, searchUser& user)
{

	getUidAndMyid(buf, &user);

	//查询uid是否在线
	string sql = "select * from userinfo where userid = '" + user.uid + "' and logonstatus =1";

	int rows = handleQueryRet(sql.c_str());

	if (rows == -1)
	{
		user.msgType = MYSQL_ERROR;
	}
	if (rows == 0)
	{
		user.msgType = CHAT_USER_FAILED;
	}
	if (rows == 1)
	{
		//用户在线 
		user.msgType = CHAT_USER_ONLINE;
	}

	
}

string handle::handleReadFile()
{
	ifstream ifs;

	ifs.open(FILEPATH,ios::in);

	if (!ifs.is_open())
	{
		cout << "读文件打开失败" << endl;
	}

	//读数据
	string filemsg;
	string buf;
	while (getline(ifs, buf))
	{
		filemsg.append(buf+"\n");
	}

	//关闭文件
	ifs.close();

	return filemsg;

}

char* handle::setAddFriendJson(int type, searchUser& user)
{
	cJSON* json = NULL;
	char* str = NULL;

	string uid = user.uid;
	string msg = user.msg;
	//创建JSON对象

	json = cJSON_CreateObject();

	//添加数据
	cJSON_AddNumberToObject(json, "type", type);

	cJSON_AddStringToObject(json, "uid", uid.c_str());

	cJSON_AddStringToObject(json, "msg", msg.c_str());

	str = cJSON_Print(json);

	return str;

}

string handle::handleLogon( const char* buf)
{
	//获取userinfo信息

	this->getUserInfo(buf);

	string uid = user.uid;
	string pwd = user.pwd;
	//查询用户与密码是否存在数据库中--还有是否登录

	cout << "uid =" << uid << " pwd =" << pwd << endl;

	string sql = " select * from userinfo where userid ='" + uid + "'and pwd ='" + pwd + "'";

	int  rows = this->handleQueryRet(sql.c_str());//成功返回行数 失败返回-1
	

	if (rows == -1)
	{
		//说明异常

		return MYSQL_ERROR;

	}
	else if(rows == 0)
	{
		
		return LOGON_USER_NO_EXIST;
		
	}
	else if(rows>0)
	{
		//密码和用户匹配
		// 判断用户是否登录
		 sql = " select * from userinfo where userid = '" + uid + " 'and pwd ='" + pwd + "' and logonstatus =1";
		 //如果返回的是1 结果集有数据 说明用户已经登录
		rows = this->handleQueryRet(sql.c_str());
		 if (rows == 0 )
		 {
			 //可以登录
			 //告知客户端  用户可以登录
			 //将用户状态修改为1  ---bug1
			 sql = "update userinfo set logonstatus = 1  where userid= " + uid + "";
			 mysql_query(mysql, sql.c_str());

			 return LOGON_SUCCESS;
		}
		 else if(rows == 1)
		 {
			 return LOGON_FAILED;
		 }
		 
	}

}

string handle::handleLogin(const char* buf)
{
	// 获取userinfo信息

	this->getUserInfo(buf);

	string uid = user.uid;
	string pwd = user.pwd;


	//查询数据库 用户名是否存在    

	string sql = "select * from userinfo where userid =" + uid ;

	int rows = this->handleQueryRet(sql.c_str());

	//如果rows >0 说明 用户已经存在 拒绝注册

	//如果rows = 0 说明用户不存在 可以登录

	//如果rows = -1 说明出现异常

	if (rows == -1)
	{
		return MYSQL_ERROR;
	}

	if (rows > 0)
	{
		return LOGIN_USER_EXIST;
	}
	if (rows == 0)
	{
		//用户不存在 
		//将用户插入数据库
		string sql = " insert into userinfo(userid,pwd,logonstatus) values (" + uid + ", " + pwd.c_str() + ",1)";

		//执行sql语句
		int ret = mysql_query(this->mysql, sql.c_str());
		
		//如果ret == 1 执行失败 ret == 0 插入成功   

		if (ret == 1)
		{
			return MYSQL_ERROR;
		}
		else if(ret == 0)
		{
			return LOGIN_SUCCESS;
		}
		
	}
}

int handle::handleQueryRet(const char * sql)
{
	cout << "sql =" << sql << endl;
	int ret =mysql_real_query(mysql, sql, strlen(sql));//查询成功返回0 返回非0
	if (ret != 0)
	{
		cout << "查询语句失败,sql:" << mysql_error(mysql) << endl;
		return -1;
		
	}
	else
	{
		cout << "查询语句成功" << endl;
		//创建mysql对象
	   //从对象读取结果
		MYSQL_RES* res = mysql_store_result(mysql);
		if (res == nullptr)
		{
			cout << "读取结果失败" << endl;
			return -1;
		}
		//从对象获取行数
		int rows = mysql_num_rows(res);
		//清除结果集
		mysql_free_result(res);

		return rows;
	}
}

MYSQL* handle::getMysql()
{
	return this->mysql;
}

void handle::getUidAndMyid(const char* buf, searchUser* user)
{
	//json反序列
	cJSON* json = cJSON_Parse(buf);
	cJSON* cJSON_uid = NULL;

	//获取key的节点
	cJSON_uid = cJSON_GetObjectItem(json, "uid");
	if (cJSON_uid == NULL)
	{
		printf("uid node =NULL\n");
	}

	//获取key对应的值
	user->uid= cJSON_uid->valuestring;

	cJSON_uid = cJSON_GetObjectItem(json, "myid");

	if (cJSON_uid == NULL)
	{
		printf("uid node =NULL\n");
	}

	user->myid = cJSON_uid->valuestring;


}

void  handle::getUserInfo(const char* buf)
{
	//json反序列化
	cJSON* json = cJSON_Parse(buf);
	cJSON* node = NULL;

	node = cJSON_GetObjectItem(json, "userInfo");

	if (node == NULL)
	{
		printf("userInfo node =NULL\n");
	}
	cJSON* cJSON_uid = cJSON_GetObjectItem(node, "uid");
	cJSON* cJSON_pwd = cJSON_GetObjectItem(node, "pwd");

	//获取用户名和密码
	string uid = cJSON_uid->valuestring;
	string pwd = cJSON_pwd->valuestring;

	cout << "uid =" << uid << " pwd =" << pwd << endl;


	this->user.uid = uid;
	this->user.pwd = pwd;

	cout << "结构体uid =" << user.uid << " pwd =" << user.pwd << endl;

}



