/*
	Created By YSP
	2025.8.3
*/
#pragma once
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
		static void TriggerJsEvent(JsParser& jsParser, const QString& key, QResizeEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& key, QWheelEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& key, QKeyEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& key, QCloseEvent* event, bool global = false);
		static void TriggerJsEvent(JsParser& jsParser, const QString& key, QMouseEvent* event, bool global = false);
	};
}