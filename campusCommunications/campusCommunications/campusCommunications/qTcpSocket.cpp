#include "qTcpSocket.h"

qTcpSocket::qTcpSocket(QObject *parent)
	: QObject(parent)
{

	//����ͨ���׽���
	m_tcpsocket = new qTcpSocket(this);

	m_tcpsocket->connect();
}

qTcpSocket::~qTcpSocket()
{}
