#include"Server.h"


int main(int argc ,char * argv[])
{
	
	//创建套接字
	handleSocket();
	
}

int getCfd(string uid)
{
	int cfd = -1;
	for (map<int, string>::iterator it = logonmap.begin(); it != logonmap.end(); it++)
	{
		string value = (*it).second;
		if (value == uid)
		{
			 cfd= (*it).first;
		}
	}
	return cfd;
}

void* getJson(void* arg)
{
	 char* buf = (char *)arg;

	 cout << "客户端发来的消息:" << buf << endl;
	
	 //解析Json
	cJSON* json = cJSON_Parse(buf);
	cJSON* node = NULL;

	node = cJSON_GetObjectItem(json, "type");

	if (node == NULL)
	{
		printf("userInfo node =NULL\n");
	}
	int type = node->valueint;

	printf("type=%d\n", type);
	
	//根据类型调用不同句柄
	switch (type)
	{
	case(ENUM_MSG_TYPE_LOGON_REQUEST):
		{
			//登录
		 string msg = Server::getInstance().handleLogon(buf);
		 //创建孙线程 将处理好的数据json序列化 

		 pthread_t setJson_tid;

		 pthread_create(&setJson_tid, NULL, setLogonJson, &msg);

		 //接收json序列化的数据
		 void* buf2;
		 pthread_join(setJson_tid, &buf2);

		 char* str = (char*)buf2;

		pthread_exit(str);//子线程退出

			break;
		}
	case(ENUM_MSG_TYPE_LOGIN_REQUEST):
		{
			
		//注册
		string msg = Server::getInstance().handleLogin(buf);

		//创建孙线程 将处理好的数据json序列化 

		pthread_t setJson_tid;

		pthread_create(&setJson_tid, NULL, setLoginJson, &msg);

		//接收json序列化的数据
		void* buf2;
		pthread_join(setJson_tid, &buf2);

		char* str = (char*)buf2;

		pthread_exit(str);//子线程退出

			break;
		}
	case (ENUM_MSG_TYPE_LOGON_SUCCESS):
	{
	
		//获取已经登录的id
		string uid = Server::getInstance().getUserid(buf);

		//记录到容器
		logonmap[logoncfd] =uid;

		//调用消息缓存
		handleCaDate();

		pthread_exit(NULL);
		
		break;
	}
	case(ENUM_MSG_TYPE_SEARCH_USER_REQUEST):
	{
		//好友申请
		struct searchUser user;
		Server::getInstance().UserIsFriend(buf,&user);

		//创建孙线程 将处理好的数据json序列化 
		pthread_t setJson_tid;

		pthread_create(&setJson_tid, NULL, setSearchUserJson, &user);

		//接收json序列化的数据
		void* buf2;
		pthread_join(setJson_tid, &buf2);

		char* str = (char*)buf2;

		pthread_exit(str);

		break;
	}
	case(ENUM_MSG_TYPE_ADD_USER_REQUEST):
	{
		cout << "用户的好友申请" << endl;
		//获取的消息
		// user
		// uid --- 好友
		// myid ----申请好友关系的用户
		//  msgtype  --- 记录好友是否在线
		struct searchUser user;
		Server::getInstance().handleAddUser(buf, &user);

		//saveDate uid记录的是消息发起者
		struct saveDate sd;
		sd.uid = user.myid; 
		sd.type = ENUM_MSG_TYPE_ADD_USER_REQUEST;
		sd.msg = "用户" + user.myid + "发来好友申请";

		handleUserRequest(user,sd);

		//无论在不在线 都需要回复给等待的客户端 告知客户端好友请求已经转发

		user.msgType = ADD_USER_ONLINE;

		//创建孙线程 将处理好的数据json序列化 
		pthread_t setJson_tid;

		pthread_create(&setJson_tid, NULL, setforwardJson, &user);

		//接收json序列化的数据
		void* buf2;
		pthread_join(setJson_tid, &buf2);

		char* str = (char*)buf2;
		cout << "添加str=" << str;

		pthread_exit(str);

		cout << "-----------------------------" << endl;

		break;
	}
	case(ENUM_MSG_TYPE_ADD_USER_AGREE_REQUEST):
	{
		cout << "用户同意好友申请" << endl;
		//取出uid和myid 存储在user中
		struct searchUser user;
		Server::getInstance().UserIsFriend(buf, &user);


		//为uid和myid保存到friendsinfo

		string sql = "insert into friendsinfo (id,friendid) values('" + user.myid + "','" + user.uid + "')";

		Server::getInstance().handleAddFriend(sql);

		//此处不对用户是否在线进行判断  日后再修改
		//转发给用户
		string desid = user.uid;

		saveDate sd;
		sd.type = ENUM_MSG_TYPE_ADD_USER_AGREE_RESPOND;
		sd.uid = user.myid;
		sd.msg = "用户" + user.uid + "同意你的好友邀请";;
		
		fowardMsg(desid,sd);

		cout << "-----------------------------" << endl;
		
		pthread_exit(NULL);
		
		break;
	}
	case(ENUM_MSG_TYPE_ADD_USER_NO_AGREE_REQUEST):
	{
		
		cout << "用户拒绝好友申请" << endl;
		//取出uid和myid 存储在user中
		struct searchUser user;
		Server::getInstance().UserIsFriend(buf, &user);


		//此处不对用户是否在线进行判断  日后再修改
		//转发给用户
		string desid = user.uid;

		saveDate sd;
		sd.type = ENUM_MSG_TYPE_ADD_USER_NO_AGREE_RESPOND;
		sd.uid = user.myid;
		sd.msg = "用户" + user.uid + "拒绝你的好友邀请";;

		fowardMsg(desid, sd);

		cout << "-----------------------------" << endl;

		pthread_exit(NULL);
	}
	case(ENUM_MSG_TYPE_CHAT_USER_REQUEST):
	{
		//私聊请求
		//获取uid myid  msgType
		struct searchUser user;

		Server::getInstance().handleChatRequest(buf, user);

		//根据消息进行判断

		string msgType = user.msgType;

		if (msgType == MYSQL_ERROR)
		{
			//出现异常
			char* str = MYSQL_ERROR;
			pthread_exit(str);

		}
		else if (msgType == CHAT_USER_FAILED)
		{
			//用户不在线
			char* msg = CHAT_USER_FAILED;
			pthread_exit(msg);
			
		}
		else if (msgType == CHAT_USER_ONLINE)
		{
			//用户在线 进行转发
		
			saveDate sd;
			sd.uid = user.myid;
			sd.type = ENUM_MSG_TYPE_CHAT_USER_REQUEST;
			sd.msg = "好友" + user.myid + "申请私聊，是否同意";

			fowardMsg(user.uid, sd);

			pthread_exit(NULL);
		}
		break;
	}
	case(ENUM_MSG_TYPE_CHAT_USER_AGREE_REQUEST):
	{
		//同意私聊
		//取出uid和myid 存储在user中
		struct searchUser user;
		Server::getInstance().UserIsFriend(buf, &user);


		//此处不对用户是否在线进行判断  日后再修改
		//转发给用户
		string desid = user.uid;

		saveDate sd;
		sd.type = ENUM_MSG_TYPE_CHAT_USER_AGREE_RESPOND;
		sd.uid = user.myid;
		sd.msg = CHAT_USER_ONLINE;

		fowardMsg(desid, sd);

		pthread_exit(NULL);
	}
	case(ENUM_MSG_TYPE_CHAT_USER_REFUSE_REQUEST):
	{
		//拒绝私聊
		struct searchUser user;
		Server::getInstance().UserIsFriend(buf, &user);


		//此处不对用户是否在线进行判断  日后再修改
		//转发给用户
		string desid = user.uid;

		saveDate sd;
		sd.type = ENUM_MSG_TYPE_CHAT_USER_REFUSE_RESPOND;
		sd.uid = user.myid;
		sd.msg = CHAT_USER_REFUSE;

		fowardMsg(desid, sd);

		pthread_exit(NULL);
	}
	case(ENUM_MSG_TYPE_SEND_CHAT_MSG):
	{
		//提取uid myid  
		struct searchUser user;
		Server::getInstance().UserIsFriend(buf, &user);

		//提取msg

		//json反序列
		cJSON* json = cJSON_Parse(buf);
		cJSON* cJSON_msg = NULL;

		//获取key的节点
		cJSON_msg = cJSON_GetObjectItem(json, "msg");
		if (cJSON_msg == NULL)
		{
			printf("msg node =NULL\n");
		}

		//获取key对应的值
		user.msg = cJSON_msg->valuestring;

		//进行转发

		//转发的类型 转发消息  
	   // uid --- 谁发来申请的用户
	  //type  --- 什么样的申请
	  //msg ---- 发来什么样的申请

		saveDate sd;
		sd.uid = user.myid;
		sd.type = ENUM_MSG_TYPE_RECV_CHAT_MSG;
		sd.msg = user.msg;

		//转发
		fowardMsg(user.uid, sd);
	}
	case(ENUM_MSG_TYPE_READ_FILE_REQUEST):
	{
		//读文件
		string msg =Server::getInstance().handleReadFile();

		//文件内容Json序列化
		char * str =handleReadFileJson(msg);
		
		pthread_exit(str);

		
	}
	case(ENUM_MSG_TYPE_WRITE_FILE_REQUEST):
	{
		//写文件
		ofstream ofs;

		ofs.open(FILEPATH, ios::out | ios::app);

		//获取数据
		node = cJSON_GetObjectItem(json, "msg");

		if (node == NULL)
		{
			printf("userInfo node =NULL\n");
		}
		string msg = node->valuestring;
		//写数据
		ofs << msg;

		ofs.close();

		//告知客户端  文件内容已被更新
		//读取文件
		//读文件
		 msg = Server::getInstance().handleReadFile();

		//文件内容Json序列化
		

		//Json反序列化
		cJSON* json = cJSON_CreateObject();

		int type = ENUM_MSG_TYPE_WRITE_FILE_RESPOND;
		//添加数据
		cJSON_AddNumberToObject(json, "type", type);

		cJSON_AddStringToObject(json, "msg", msg.c_str());

		char* str = cJSON_Print(json);

		cout << "读文件内容" << endl;
		cout << str << endl;
		
		pthread_exit(str);
	}
	default:
		break;
	}

}


string handle::getUserid(const char* buf)
{
	cJSON* json = cJSON_Parse(buf);
	cJSON* cJSON_uid = cJSON_GetObjectItem(json, "uid");

	if (cJSON_uid == NULL)
	{
		printf("userInfo node =NULL\n");
	}

	//获取用户名和密码
	string uid = cJSON_uid->valuestring;

	return uid;
}

void handleSocket()
{
	char str[INET_ADDRSTRLEN];
	//创建套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);

	//端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//绑定套接字

	struct	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(9000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int ret = 0;
	ret = bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));


	//监听套接字
	ret = listen(lfd, MAXZSIZE);
	if (ret < 0)
	{
		printf("listen error\n");
	}


	//利用epoll来接客户端的请求

	//创建红黑树
	int epfd = epoll_create(1);
	if (epfd < 0)
	{
		printf("epoll_create error\n");
	}

	//将lfd上树
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = lfd;

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

	if (ret < 0)
	{
		printf("epoll_ctl error\n");
	}

	struct epoll_event evs[MAXZSIZE];

	while (1)
	{
		int connetNum = epoll_wait(epfd, evs, MAXZSIZE, -1);

		//判断conntNum是什么事件
		if (connetNum <= 0)
		{
			break;
		}
		char buf[MAXZSIZE];

		//判断是否是客户端申请连接
		int i = 0;
		for (; i < connetNum; i++)
		{
			if (evs[i].data.fd == lfd)
			{
				//客户端申请连接
				struct sockaddr_in clie_addr;
				socklen_t clie_len = sizeof(clie_addr);

				int cfd = accept(lfd, (struct sockaddr*)&clie_addr, &clie_len);
				printf("received from %s at port %d \n", inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)), ntohs(clie_addr.sin_port));

				//将cfd上树
				ev.events = EPOLLIN;
				ev.data.fd = cfd;
				ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);

				if (connetNum == 1)
				{
					continue;
				}
			}
			else
			{
				//客户端数据
				int cfd = evs[i].data.fd;
				logoncfd = cfd;

				cout << "客户端描述符=" << logoncfd << endl;

				int len = read(cfd, buf, MAXZSIZE);

				if (len < 0)
				{
					if (errno == ECONNRESET)
					{
						continue;
					}
					else
					{
						close(cfd);
						ev.events = EPOLLIN;
						ev.data.fd = cfd;

						//将cfd从红黑树摘下
						epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);

						//将相应的cfd删掉
						logonmap.erase(cfd);
					}
				}
				else if (len == 0)
				{
					printf("client%d close \n", cfd);


					close(cfd);
					ev.events = EPOLLIN;
					ev.data.fd = cfd;

					//将cfd从红黑树摘下
					epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, &ev);


					//将相应的cfd删掉
					logonmap.erase(cfd);
				}
				else
				{
					//接收数据
					//多线程处理json

					//将客户端数据jSON解析 
					pthread_t getJson_tid;
			
					pthread_create(&getJson_tid, NULL, getJson, &buf);
					
					//等待子进程退出  接收服务器处理的数据
					 void* msg;
					pthread_join(getJson_tid, &msg); 


					char* str = (char*)msg;

					if (str == NULL)
					{
						cout << "指针为空" << endl;
					}
					else
					{
						string str2=str;

					//将数据发送给客户端

					struct data d;
					d.buf = str;
					d.cfd = cfd;
					d.size = str2.size();

					pthread_t writeMsg_tid;
					pthread_create(&writeMsg_tid, NULL, writeMsg, &d);
					pthread_join(writeMsg_tid, NULL);
					}
				}
			}
		}

	}

}

void handleCaDate()
{
	//获取desid
	//判断desid是否上线  
	
	   if (vData.size() == 0)
	   {
		   cout << "消息缓存为空" << endl;
			return;
	   }
	   cout << "遍历消息缓存" << endl;
	   for (vector<pair<string, struct saveDate> > ::iterator it = vData.begin(); it != vData.end(); it++)
	   {
		   pair<string, struct saveDate> p1 = *it;
		   cout << "uid=" << p1.first << " type=" << p1.second.type << " myid = " << p1.second.uid << "msg =" << p1.second.msg << endl;
	   }
	   cout << "遍历消息缓存完毕" << endl;
		for (auto it = vData.begin(); it != vData.end(); )
		{
			pair<string, struct saveDate> p1 = *it;
			string desid = p1.first;
			cout << "转发给用户id=" << desid << endl;
			//创建sql 判断是否在线
			string sql = " select * from userinfo where userid ='" + desid + "'and logonstatus =1";
			int rows = Server::getInstance().handleQueryRet(sql.c_str());
			if (rows == -1)
			{
				cout << "查询错误" << endl;
			}
			if (rows == 0)
			{
				//用户不在线
				it++;
				continue;
			}
			if (rows == 1)
			{
				//用户在线
				//进行转发

			//将消息转发
			struct saveDate date = p1.second;
			int ret =fowardMsg(desid, date);
			if (ret == -1)
			{
				cout << "转发失败" << endl;
				continue;
			}
			if (ret == 1)
			{
				//转发成功
				// 将迭代器指向元素
				it =vData.erase(it);//使用erase 会返回当前迭代器的下一个位置
			}
			 else
			{
				 it++;
			 }
		    }
			
		}
}

void handleUserRequest(searchUser& user, saveDate& sd)
{
	//获取消息类型 
	string msgType = user.msgType;

	if (msgType == MYSQL_ERROR)
	{
		//出现异常
		char* str = MYSQL_ERROR;

		pthread_exit(str);

	}
	else if (msgType == ADD_USER_NO_ONLINE)
	{
		//用户不在线
		//将消息缓存
		
		//user.uid 目的id

		pair<string, struct saveDate> p(user.uid, sd);

		vData.push_back(p);

		for (vector<pair<string, struct saveDate> > ::iterator it = vData.begin(); it != vData.end(); it++)
		{
			pair<string, struct saveDate> p1 = *it;

			cout << "遍历消息缓存" << endl;
			cout << "目的desid=" << p1.first << " 类型type=" << p1.second.type << " 转发者id = " << p1.second.uid << " 转发的msg =" << p1.second.msg << endl;
		}
	}
	else if (msgType == ADD_USER_ONLINE)
	{
		// 如果用户在线  转发给用户
		string desid = user.uid;

		fowardMsg(desid, sd);
	}
}

char* handleReadFileJson(string msg)
{
	//创建JSON对象

	cJSON* json = cJSON_CreateObject();

	int type = ENUM_MSG_TYPE_READ_FILE_RESPOND;
	//添加数据
	cJSON_AddNumberToObject(json, "type", type);

	cJSON_AddStringToObject(json, "msg", msg.c_str());

	char* str = cJSON_Print(json);

	cout << "读文件内容" << endl;
	cout << str << endl;

	return str;
}

void* setforwardJson(void* arg)
{
	cJSON* json = NULL;
	char* str = NULL;

	searchUser* user = (searchUser*)arg;

	string msgType = user->msgType;
	//创建JSON对象

	json = cJSON_CreateObject();

	int type = ENUM_MSG_TYPE_ADD_USER_RESPOND;
	//添加数据
	cJSON_AddNumberToObject(json, "type", type);

	cJSON_AddStringToObject(json, "msgType", msgType.c_str());

	str = cJSON_Print(json);

	pthread_exit(str);
}

void* setLogonJson(void* arg)
{
	cJSON* json = NULL;
	char* str = NULL;

	string *buf = (string*)arg;

	//创建JSON对象
	json = cJSON_CreateObject();

	int type = ENUM_MSG_TYPE_LOGON_RESPOND;
	//添加数据
	cJSON_AddNumberToObject(json, "type", type);
	cJSON_AddStringToObject(json, "msgType", (*buf).c_str());

	str = cJSON_Print(json);

	pthread_exit(str);
}

void* setLoginJson(void* arg)
{
	cJSON* json = NULL;
	char* str = NULL;

	string* buf = (string*)arg;

	//创建JSON对象
	json = cJSON_CreateObject();

	int type = ENUM_MSG_TYPE_LOGIN_RESPOND;
	//添加数据
	cJSON_AddNumberToObject(json, "type", type);
	cJSON_AddStringToObject(json, "msgType", (*buf).c_str());

	str = cJSON_Print(json);

	pthread_exit(str);
}

void* setSearchUserJson(void* arg)
{
	cJSON* json = NULL;
	char* str = NULL;

	searchUser* user = (searchUser*)arg;

	string uid = user->uid;
	string msgType = user->msgType;
	//创建JSON对象


	json = cJSON_CreateObject();

	int type = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
	//添加数据
	cJSON_AddNumberToObject(json, "type", type);


	cJSON_AddStringToObject(json,"uid",uid.c_str());

	cJSON_AddStringToObject(json, "msgType", msgType.c_str());


	str = cJSON_Print(json);

	pthread_exit(str);

}

int fowardMsg(string desid, struct saveDate &date)
{
	//查询用户的cfd
	int cfd = -1;

	cfd =getCfd(desid);

	cout << "要转发的cfd=" << cfd << endl;

	if (cfd == -1)
	{
		cout << "查询cfd失败" << endl;
		return -1;
	}

	//转发的类型 转发消息  
	// uid --- 谁发来申请的用户
	//type  --- 什么样的申请
	//msg ---- 发来什么样的申请
	cJSON* json = NULL;

	string uid = date.uid;
	string msg = date.msg;

	//创建JSON对象

	json = cJSON_CreateObject();

	int type = date.type;
	//添加数据
	cJSON_AddNumberToObject(json, "type", type);

	cJSON_AddStringToObject(json, "uid", uid.c_str());

	cJSON_AddStringToObject(json, "msg", msg.c_str());

	string str = cJSON_Print(json);

	cout << uid+"转发的信息为:" << str << endl;

	int size = str.size();

	int ret = write(cfd, str.c_str(),size);
	if (ret < 0)
	{
		cout << "转发失败" << endl;
		return -1;
	}
	else
	{
		cout << "转发成功" << endl;
		return 1;
	}	
}

void* writeMsg(void* arg)
{
	struct data* d = (struct data*)arg;

	int cfd = d->cfd;
	const char* buf = d->buf;
	int size = d->size;


	int ret = write(cfd,buf,size);

	if (ret < 0)
	{
		cout << "write failed" << endl;
	}

	cout << "write success" << endl;
	cout << "buf =" << buf << endl;
	return nullptr;
}

void* readMsg(void *arg)
{	 
}

handle& Server::getInstance()
{
	static handle instance;
	return instance;
}
