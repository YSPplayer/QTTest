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
		firstshow = false;
		setMouseTracking(true);
		setAutoFillBackground(true);
	}
	void CWidget::TriggerEvent(const QString& key) {
		if (key == "load") {
			//UI加载完毕之后触发
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
	QString CWidget::GetId(QWidget* widget) {
		return widget->objectName();
	}

	QString CWidget::GetClass(QWidget* widget) {
		return  widget->property("class").isValid() ?
			widget->property("class").toString() : "";
	}
	QString CWidget::GetClassName(QWidget* widget) {
		return widget->metaObject()->className();
	}
	QString CWidget::GetKeyString(QWidget* widget) {
		return QString("%1%2%3%4").arg(GetClassName(widget)).arg(GetId(widget)).arg(GetClass(widget)).arg(QString::number(reinterpret_cast<quintptr>(widget), 16));
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

	void CWidget::showEvent(QShowEvent* event) {
		if (!firstshow) {
			firstshow = true;
			callback.Publish(CallBackType::Load);
		}
		QWidget::showEvent(event);
	}

	QString CWidget::TriggerId(const QString& key) {
		return QString("%1%2").arg(GetKeyString(this)).arg(key);
	}

	void CWidget::CheckConsoleWindow() {
		if (consoleWindow.load() == nullptr) {
			consoleWindow.store(new ConsoleWindow(), std::memory_order_release);
		}
	}
}
