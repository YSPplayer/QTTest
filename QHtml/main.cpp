#include "QHtml.h"
#include <QtWidgets/QApplication>
#include "Html/htmlreader.h"
using namespace ysp::qt::html;
class F5EventFilter : public QObject {
public:
	explicit F5EventFilter(HtmlReader* r, QWidget* parent = nullptr) : QObject(parent), r(r),
		parent(parent) {
	}

protected:
	bool eventFilter(QObject* obj, QEvent* event) override
	{
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			if (keyEvent->key() == Qt::Key_F5) {
				CWidget* widget = r->ReParse();
				widget->setParent(parent);
				widget->show();
				parent->resize(widget->size());
				return true;
			}
		}
		return QObject::eventFilter(obj, event);
	}
	HtmlReader* r;
	QWidget* parent;
};
int main(int argc, char* argv[])
{
	QApplication app(argc, argv);
	/*  QHtml window;
	  window.show();*/
	HtmlReader* r = new HtmlReader(QApplication::applicationDirPath() + "/main.html");
	QWidget* rootwidget = new QWidget;
	rootwidget->installEventFilter(new F5EventFilter(r, rootwidget));
	CWidget* widget = r->Parse();
	rootwidget->resize(widget->size());
	widget->setParent(rootwidget);
	for (QWidget* w : widget->findChildren<QWidget*>()) {
		qDebug() << w->geometry();
		qDebug() << w->metaObject()->className();
	}
	CWidget::ShowConsoleWindow(true);
	/*r->Clear();*/
	if (rootwidget) rootwidget->show();
	return app.exec();
}
