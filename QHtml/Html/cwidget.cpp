/*
	Created By YSP
	2025.8.5
*/
#include "cwidget.h"
#include "triggerfilter.h"
namespace ysp::qt::html {
	std::atomic<ConsoleWindow*> CWidget::consoleWindow(nullptr);
	CWidget::CWidget(bool global, QWidget* parent) : QWidget(parent), CBase() {
		this->global = global;
		setMouseTracking(true);
		setAutoFillBackground(true);
		setAttribute(Qt::WA_NoChildEventsForParent, true);
		installEventFilter(new TriggerFilter(this, this));
	}

	void CWidget::ShowConsoleWindow(bool show) {
		CheckConsoleWindow();
		show ? consoleWindow.load()->show() : consoleWindow.load()->hide();
	}
	void CWidget::AppendConsoleWindowMsg(const QString& text) {
		CheckConsoleWindow();
		emit consoleWindow.load()->appendText(text);
	}

	void CWidget::CheckConsoleWindow() {
		if (consoleWindow.load() == nullptr) {
			consoleWindow.store(new ConsoleWindow(), std::memory_order_release);
		}
	}
}
