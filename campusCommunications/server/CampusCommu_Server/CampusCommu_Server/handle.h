#pragma once
#include<iostream>
#include<string>
#include"cJSON.h"
#include<mysql/mysql.h>
#include"protocol.h"
#include<map>
#include<fstream>


#define FILEPATH "H:\\QTstudy\\campusCommunications\\server\\CampusCommu_Server\\CampusCommu_Server\\CampusInfo.txt"
using namespace std;

struct userInfo
{
	string uid;
	string pwd;
};
struct searchUser
{
	string uid;
	string myid;
	string msgType;
	string msg;
};

class handle
{
public:
	handle();
	string handleLogon(const char* buf);//登录
	string handleLogin(const char* buf);//注册
	int handleQueryRet(const char* sql);//查询
	void handleConnectMysql();//连接
	void UserIsFriend(const char * buf,searchUser * user);//搜索
	void handleAddUser(const char* buf, searchUser* user);
	void handleAddFriend(string sql);//添加好友
	void handleChatRequest(const char* buf, searchUser& user);
	string handleReadFile();


	char * setAddFriendJson(int type,searchUser & user );


	MYSQL* getMysql();

	void getUidAndMyid(const char* buf, searchUser* user);
	void getUserInfo(const char* buf);
	string getUserid(const char* buf);

public:
	MYSQL* mysql;
	struct userInfo user;

};


