#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QHtml.h"

class QHtml : public QMainWindow
{
    Q_OBJECT

public:
    QHtml(QWidget *parent = nullptr);
    ~QHtml();

private:
    Ui::QHtmlClass ui;
};

