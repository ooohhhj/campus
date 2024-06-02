/********************************************************************************
** Form generated from reading UI file 'logon.ui'
**
** Created by: Qt User Interface Compiler version 5.12.12
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGON_H
#define UI_LOGON_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_logonClass
{
public:
    QLabel *label;
    QLineEdit *sId_Le;
    QPushButton *logon_Btn;

    void setupUi(QWidget *logonClass)
    {
        if (logonClass->objectName().isEmpty())
            logonClass->setObjectName(QString::fromUtf8("logonClass"));
        logonClass->resize(600, 400);
        label = new QLabel(logonClass);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(100, 70, 51, 51));
        sId_Le = new QLineEdit(logonClass);
        sId_Le->setObjectName(QString::fromUtf8("sId_Le"));
        sId_Le->setGeometry(QRect(190, 70, 161, 51));
        logon_Btn = new QPushButton(logonClass);
        logon_Btn->setObjectName(QString::fromUtf8("logon_Btn"));
        logon_Btn->setGeometry(QRect(230, 240, 71, 31));

        retranslateUi(logonClass);

        QMetaObject::connectSlotsByName(logonClass);
    } // setupUi

    void retranslateUi(QWidget *logonClass)
    {
        logonClass->setWindowTitle(QApplication::translate("logonClass", "logon", nullptr));
        label->setText(QApplication::translate("logonClass", "\345\255\246\345\217\267:", nullptr));
        logon_Btn->setText(QApplication::translate("logonClass", "\347\231\273\345\275\225", nullptr));
    } // retranslateUi

};

namespace Ui {
    class logonClass: public Ui_logonClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGON_H
