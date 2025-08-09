/*
	Created By YSP
	2025.8.5
*/
#include "cwidget.h"
#include <QMouseEvent>
namespace ysp::qt::html {
	JsParser CWidget::jsParser;
	QMap<QWidget*, StyleBuilder> CWidget::styleBuilder;
	std::atomic<ConsoleWindow*> CWidget::consoleWindow(nullptr);
	CWidget::CWidget(QWidget* parent):QWidget(parent){
		isPressedLeft = false;
		setMouseTracking(true);
	}
	void CWidget::TriggerEvent(const QString& key) {
		if (key == "load") {
			//UI�������֮�󴥷�
			jsParser.Trigger("load");
		}
	}
	void CWidget::ShowConsoleWindow(bool show) {
		CheckConsoleWindow();
		show ? consoleWindow.load()->show() : consoleWindow.load()->hide();
	}
	void CWidget::AppendConsoleWindowMsg(const QString& text) {
		CheckConsoleWindow();
		emit consoleWindow.load()->appendText(text);
	}
	void CWidget::mousePressEvent(QMouseEvent* event) {
		auto button = event->button();
		if (button == Qt::LeftButton) {
			isPressedLeft = true;
		}
		JsClass* obj = new JsClass;
		(*obj)["button"] = JsValue::CreateValue(button == Qt::LeftButton ? 0 : button == Qt::MiddleButton ?
			1 : button == Qt::RightButton ? 2 : -1);
		jsParser.Trigger(TriggerId("mousedown"), JsValue::CreateValue(obj));
		QWidget::mousePressEvent(event);
	}

	void CWidget::mouseReleaseEvent(QMouseEvent* event) {
		auto button = event->button();
		if (button == Qt::LeftButton) {
			isPressedLeft = false;
			jsParser.Trigger(TriggerId("click"));
		}
		JsClass* obj = new JsClass;
		(*obj)["button"] = JsValue::CreateValue(button == Qt::LeftButton ? 0 : button == Qt::MiddleButton ?
			1 : button == Qt::RightButton ? 2 : -1);
		jsParser.Trigger(TriggerId("mouseup"), JsValue::CreateValue(obj));
		QWidget::mouseReleaseEvent(event);
	}

	QString CWidget::TriggerId(const QString& key) {
		return QString("%1%2").arg(objectName()).arg(key);
	}

	void CWidget::CheckConsoleWindow() {
		if (consoleWindow.load() == nullptr) {
			consoleWindow.store(new ConsoleWindow(), std::memory_order_release);
		}
	}
}
