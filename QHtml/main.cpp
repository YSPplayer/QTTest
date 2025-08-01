#include "QHtml.h"
#include <QtWidgets/QApplication>
#include "Html/htmlreader.h"
using namespace ysp::qt::html;
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
  /*  QHtml window;
    window.show();*/
    HtmlReader *r = new HtmlReader("E:/QTtest/QTTest/QHtml/QHtml/main.xml");
    QWidget* widget = r->Parse();
    //for (QWidget* w : widget->findChildren<QWidget*>()) {
    //    qDebug() << w->geometry();
    //}


    widget->show();
    return app.exec();
}
