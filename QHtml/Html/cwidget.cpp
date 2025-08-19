/*
	Created By YSP
	2025.8.5
*/
#include "cwidget.h"
namespace ysp::qt::html {
	JsParser CWidget::jsParser;
	QMap<QWidget*, StyleBuilder> CWidget::styleBuilder;
	std::atomic<ConsoleWindow*> CWidget::consoleWindow(nullptr);
	CWidget::CWidget(bool global,QWidget* parent):global(global),QWidget(parent){
		isPressedLeft = false;
		firstshow = false;
		setMouseTracking(true);
		setAutoFillBackground(true);
		setAttribute(Qt::WA_NoChildEventsForParent, true);
	}
	void CWidget::TriggerGlobalEvent(const QString& key) {
		jsParser.Trigger(key, true);
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

	QString CWidget::GetJsId(QWidget* widget) {
		return  widget->property("jsid").isValid() ?
			widget->property("jsid").toString() : "";
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
		TriggerJsEvent<QMouseEvent>("mousedown", event);
		QWidget::mousePressEvent(event);
	}

	void CWidget::mouseMoveEvent(QMouseEvent* event) {
		TriggerJsEvent<QMouseEvent>("mousemove", event);
		QWidget::mouseMoveEvent(event);
	}

	void CWidget::mouseReleaseEvent(QMouseEvent* event) {
		auto button = event->button();
		if (button == Qt::LeftButton) {
			isPressedLeft = false;
			TriggerJsEvent("click");
		}
		TriggerJsEvent<QMouseEvent>("mouseup", event);
		QWidget::mouseReleaseEvent(event);
	}

	void CWidget::showEvent(QShowEvent* event) {
		if (!firstshow) {
			firstshow = true;
			if (global) jsParser.Trigger("load", true);
		}
		QWidget::showEvent(event);
	}

	void CWidget::resizeEvent(QResizeEvent* event) {
		TriggerJsEvent<QResizeEvent>("resize", event);
		QWidget::resizeEvent(event);
	}

	void CWidget::wheelEvent(QWheelEvent* event) {
		TriggerJsEvent<QWheelEvent>("wheel", event);
		QWidget::wheelEvent(event);
	}

	void CWidget::keyPressEvent(QKeyEvent* event) {
		TriggerJsEvent<QKeyEvent>("keydown", event);
		QWidget::keyPressEvent(event);
	}

	void CWidget::keyReleaseEvent(QKeyEvent* event) {
		TriggerJsEvent<QKeyEvent>("keyup", event);
		QWidget::keyReleaseEvent(event);
	}

	void CWidget::closeEvent(QCloseEvent* event) {
		TriggerJsEvent<QCloseEvent>("close", event);
		QWidget::closeEvent(event);
	}

	void CWidget::enterEvent(QEnterEvent* event) {
		TriggerJsEvent<QEnterEvent>("mouseenter", event);
		QWidget::enterEvent(event);
	}

	void CWidget::leaveEvent(QEvent* event) {
		TriggerJsEvent<QEvent>("mouseleave", event);
		QWidget::leaveEvent(event);
	}

	void CWidget::mouseDoubleClickEvent(QMouseEvent* event) {
		TriggerJsEvent("dblclick");
		QWidget::mouseDoubleClickEvent(event);
	}

	QString CWidget::TriggerId(const QString& key) {
		return global ? key : QString("%1%2").arg(GetKeyString(this)).arg(key);
	}

	void CWidget::CheckConsoleWindow() {
		if (consoleWindow.load() == nullptr) {
			consoleWindow.store(new ConsoleWindow(), std::memory_order_release);
		}
	}
	void CWidget::TriggerJsEvent(const QString& key) {
		LinkBridge::TriggerJsEvent(jsParser, GetKeyString(this), TriggerId(key), global);
	}
}
