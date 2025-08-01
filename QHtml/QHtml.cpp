#include "QHtml.h"
#include "Html/htmlreader.h"
using namespace ysp::qt::html;
QHtml::QHtml(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
   /* HtmlReader r("E:/QTtest/QTTest/QHtml/QHtml/main.xml");
    r.Parse();*/
}

QHtml::~QHtml()
{}

