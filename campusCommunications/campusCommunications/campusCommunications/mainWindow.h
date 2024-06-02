#pragma once

#include <QtWidgets/QWidget>
#include "ui_mainWindow.h"

class mainWindow : public QWidget
{
    Q_OBJECT

public:
    mainWindow(QWidget *parent = nullptr);
    ~mainWindow();

private:
    Ui::mainWindowClass ui;
};
