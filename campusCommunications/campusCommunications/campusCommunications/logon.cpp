#include "logon.h"

logon::logon(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.logon_Btn, SIGNAL(clicked()), this, SLOT(on_logon_clicked()));
}

logon::~logon()
{}
void  logon::on_logon_clicked()  //��¼��ť�����
{
	//��ȡѧ�ź�����

}