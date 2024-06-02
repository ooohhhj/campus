#pragma once

#include <QWidget>
#include "ui_logon.h"
#include<QDebug>

class logon : public QWidget
{
	Q_OBJECT

public:
	logon(QWidget *parent = nullptr);
	~logon();
public slots:
	void on_logon_clicked();
private:
	Ui::logonClass ui;
};
