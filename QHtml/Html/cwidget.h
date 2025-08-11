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
namespace ysp::qt::html {
	class CWidget: public QWidget {
	//	Q_OBJECT ����֮���ò����Զ�����ʽ
	public:
		static JsParser jsParser;
		static QMap<QWidget*,StyleBuilder> styleBuilder;
		CallBack callback;
		CWidget(QWidget* parent = nullptr);
		~CWidget() {};
		void TriggerEvent(const QString& key);
		static void ShowConsoleWindow(bool show);
		static void AppendConsoleWindowMsg(const QString& text);
		static QString GetId(QWidget* widget);
		static QString GetClass(QWidget* widget);
		static QString GetClassName(QWidget* widget);
		static QString GetKeyString(QWidget* widget);
	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void showEvent(QShowEvent* event) override;
	private:
		bool isPressedLeft;
		bool firstshow;
		QString TriggerId(const QString& key);
		static void CheckConsoleWindow();
		static std::atomic<ConsoleWindow*> consoleWindow;
	};
}