#include "QHtml.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QHtml window;
    window.show();
    return app.exec();
}
