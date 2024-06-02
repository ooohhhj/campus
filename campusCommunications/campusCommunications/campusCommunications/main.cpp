#include "mainWindow.h"
#include <QtWidgets/QApplication>
#include "logon.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /*mainWindow w;
    w.show();*/
    logon l;
    l.show();

   
    return a.exec();
}
