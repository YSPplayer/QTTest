/*
	Created By YSP
	2025.8.3
*/
#pragma once
#include <QEvent>
#include <QEnterEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMouseEvent>
#include "jsparser.h"
namespace ysp::qt::html {
	class LinkBridge {
	public:
		static void Print(const char* str);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, QResizeEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, QWheelEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, QKeyEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, QCloseEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, QMouseEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, QEnterEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, QEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& target, const QString& key, bool global = false);
	};
}