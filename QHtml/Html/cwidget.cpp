/*
	Created By YSP
	2025.8.5
*/
#include "cwidget.h"
#include "linkbridge.h"
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
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("mousedown"), event, global);
		QWidget::mousePressEvent(event);
	}

	void CWidget::mouseMoveEvent(QMouseEvent* event) {
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("mousemove"), event, global);
		QWidget::mouseMoveEvent(event);
	}

	void CWidget::mouseReleaseEvent(QMouseEvent* event) {
		auto button = event->button();
		if (button == Qt::LeftButton) {
			isPressedLeft = false;
			jsParser.Trigger(TriggerId("click"));
		}
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("mouseup"), event, global);
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
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("resize"), event,global);
		QWidget::resizeEvent(event);
	}

	void CWidget::wheelEvent(QWheelEvent* event) {
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("wheel"), event, global);
		QWidget::wheelEvent(event);
	}

	void CWidget::keyPressEvent(QKeyEvent* event) {
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("keydown"), event, global);
		QWidget::keyPressEvent(event);
	}

	void CWidget::keyReleaseEvent(QKeyEvent* event) {
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("keyup"), event, global);
		QWidget::keyReleaseEvent(event);
	}

	void CWidget::closeEvent(QCloseEvent* event) {
		LinkBridge::TriggerJsEvent(jsParser, TriggerId("close"), event, global);
		QWidget::closeEvent(event);
	}

	QString CWidget::TriggerId(const QString& key) {
		return global ? key : QString("%1%2").arg(GetKeyString(this)).arg(key);
	}

	void CWidget::CheckConsoleWindow() {
		if (consoleWindow.load() == nullptr) {
			consoleWindow.store(new ConsoleWindow(), std::memory_order_release);
		}
	}
}
