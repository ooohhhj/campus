#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include <errno.h>
#include<pthread.h>
#include"cJSON.h"
#include<iostream>
#include<vector>
#include"handle.h"
#include"protocol.h"
using namespace std;

#define MAXZSIZE 1024

//记录已经登录的用户
map<int,string>logonmap;

//消息缓存容器
vector<pair<string,struct saveDate>> vData;

int logoncfd;


struct data
{
public:
	 int cfd;
	 const char *buf;
	 int size;
};
struct saveDate
{
	int type;
	string uid;
	string msg;
};

struct cfdInfo
{
public:
	int cfd;
	int uid;
};
class Server
{
public:
	static handle& getInstance();
};

int getCfd(string uid);

void* getJson(void* arg);

void handleSocket();

void handleCaDate();

void handleMsg();

void handleUserRequest(searchUser & user,saveDate & sd);

char* handleReadFileJson(string msg);




void* setforwardJson(void* arg);
void* setLogonJson(void * arg);
void* setLoginJson(void* arg);
void* setSearchUserJson(void* arg);







int fowardMsg(string desid, struct saveDate& date);
void* writeMsg(void * arg );

int readMsg(int cfd, char buf[1024], int size);

