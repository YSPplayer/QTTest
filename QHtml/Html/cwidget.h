/*
	Created By YSP
	2025.8.5
*/
#pragma once
#include <atomic>
#include <QString>
#include <QMap>
#include <QWidget>
#include "jsparser.h"
#include "consolewindow.h"
#include "stylebuilder.h"
namespace ysp::qt::html {
	class CWidget: public QWidget {
	public:
		static JsParser jsParser;
		static QMap<QWidget*,StyleBuilder> styleBuilder;
		CWidget(QWidget* parent = nullptr);
		~CWidget() {};
		void TriggerEvent(const QString& key);
		static void ShowConsoleWindow(bool show);
		static void AppendConsoleWindowMsg(const QString& text);
	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
	private:
		bool isPressedLeft;
		QString TriggerId(const QString& key);
		static void CheckConsoleWindow();
		static std::atomic<ConsoleWindow*> consoleWindow;
	};
}