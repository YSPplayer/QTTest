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
#include "callback.h"
#include "linkbridge.h"
#include "cbase.h"
namespace ysp::qt::html {
	class CWidget : public QWidget, public CBase {
		//	Q_OBJECT 加了之后用不了自定义样式
	public:
		CallBack callback;
		explicit CWidget(bool global = false, QWidget* parent = nullptr);
		~CWidget() {};
		static void ShowConsoleWindow(bool show);
		static void AppendConsoleWindowMsg(const QString& text);
	private:
		static void CheckConsoleWindow();
		static std::atomic<ConsoleWindow*> consoleWindow;
	};
}