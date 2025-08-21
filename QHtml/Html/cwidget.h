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
namespace ysp::qt::html {
	class CWidget: public QWidget {
	//	Q_OBJECT 加了之后用不了自定义样式
	public:
		CallBack callback;
		CWidget(bool global = false,QWidget* parent = nullptr);
		~CWidget() {};
		void TriggerGlobalEvent(const QString& key);
		static void ShowConsoleWindow(bool show);
		static void AppendConsoleWindowMsg(const QString& text);
		static QString GetId(QWidget* widget);
		static QString GetJsId(QWidget* widget);
		static QString GetClass(QWidget* widget);
		static QString GetClassName(QWidget* widget);
		static QString GetKeyString(QWidget* widget);
	protected:
		void mousePressEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void showEvent(QShowEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;
		void wheelEvent(QWheelEvent* event)override;
		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;
		void closeEvent(QCloseEvent* event) override;
		void enterEvent(QEnterEvent* event) override;
		void leaveEvent(QEvent* event) override;
		void mouseDoubleClickEvent(QMouseEvent* event) override;
	private:
		bool global;//是否为body体父元素，和window直接挂钩
		bool isPressedLeft;
		bool firstshow;
		QString TriggerId(const QString& key);
		static void CheckConsoleWindow();
		static std::atomic<ConsoleWindow*> consoleWindow;
		template<typename T>
		void TriggerJsEvent(const QString& key, T* event) {
			LinkBridge::TriggerJsEvent(GetKeyString(this), TriggerId(key), event, global);
		};
		void TriggerJsEvent(const QString& key);
	};
}