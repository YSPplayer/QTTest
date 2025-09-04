#include "QHtml.h"
#include <QtWidgets/QApplication>
#include "Html/htmlreader.h"
using namespace ysp::qt::html;
int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	/*  QHtml window;
	  window.show();*/
	HtmlReader* r = new HtmlReader(QApplication::applicationDirPath() + "/main.html");
	CWidget* widget = r->Parse();
	for (QWidget* w : widget->findChildren<QWidget*>()) {
		qDebug() << w->geometry();
		qDebug() << w->metaObject()->className();
	}
	CWidget::ShowConsoleWindow(true);

	widget->show();
	return app.exec();
}
