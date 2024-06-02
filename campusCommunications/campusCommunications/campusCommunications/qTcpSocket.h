#pragma once

#include <QObject>

class qTcpSocket  : public QObject
{
	Q_OBJECT

public:
	qTcpSocket(QObject *parent);
	~qTcpSocket();
};
