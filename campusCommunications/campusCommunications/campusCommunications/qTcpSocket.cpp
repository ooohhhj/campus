#include "qTcpSocket.h"

qTcpSocket::qTcpSocket(QObject *parent)
	: QObject(parent)
{

	//创建通信套接字
	m_tcpsocket = new qTcpSocket(this);

	m_tcpsocket->connect();
}

qTcpSocket::~qTcpSocket()
{}
