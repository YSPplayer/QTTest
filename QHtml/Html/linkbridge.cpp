/*
	Created By YSP
	2025.8.3
*/
#include <QDebug>
#include "linkbridge.h"
#include "cwidget.h"
namespace ysp::qt::html {
	void LinkBridge::Print(const char* str) {
		qDebug() << str;
		CWidget::AppendConsoleWindowMsg(QString::fromUtf8(str));
	}
}
