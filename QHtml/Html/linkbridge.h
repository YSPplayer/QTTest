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
#include <QMap>
#include "jsparser.h"
#include "stylebuilder.h"
#include "cssparser.h"
namespace ysp::qt::html {
	struct ElementData {
		std::shared_ptr<ElementData> parent{ nullptr };
		QList<std::shared_ptr<ElementData>> childs{ };
		QString tag{ "" };
		QMap<QString, QString> attributes{};
		QString text{ "" };
	};
	class LinkBridge {
	public:
		static JsParser jsParser;
		static QMap<QWidget*, StyleBuilder> styleBuilder;
		static QMap<QString, QString> classmap;
		static QList<CSSRule*> cssrules;
		static void Print(const char* str);
		static void TriggerJsEvent(const QString& target, const QString& key, QResizeEvent* event, bool global = false);
		static void TriggerJsEvent(const QString& target, const QString& key, QWheelEvent* event, bool global = false);
		static void TriggerJsEvent(const QString& target, const QString& key, QKeyEvent* event, bool global = false);
		static void TriggerJsEvent(const QString& target, const QString& key, QCloseEvent* event, bool global = false);
		static void TriggerJsEvent(const QString& target, const QString& key, QMouseEvent* event, bool global = false);
		static void TriggerJsEvent(const QString& target, const QString& key, QEnterEvent* event, bool global = false);
		static void TriggerJsEvent(const QString& target, const QString& key, QEvent* event, bool global = false);
		static void TriggerJsEvent(const QString& target, const QString& key, bool global = false);
		static void TriggerJsEvent(const QString& key, bool global = false);
		static void ParseAttributes(ElementData* element, QWidget* widget);
		static void ParseAttributesBody(QWidget* widget);
		static void ParseStyleString(const QString& styleValue, QMap<QString, QString>& map);
		static void ParseKey(const QString& key, QWidget* widget, StyleBuilder& builder, QMap<QString, QString>& attributes);
		static QString ExtractFuncString(const QString& input);
		static QString QClassToHtmlClass(const QString& name);
		static QString ReplaceAfterHash(QString input, const QString& replacement);
		static QString RemoveStrPrefix(const QString& key, const QString& eventName);
		static QString ToNumberString(const QString& key);
		static QList<QString> Split(const QString& key, const QString& split);
		static bool ContainsId(const QString& id);
		qint32 static FindSubstringEndIndex(const QString& mainStr, const QString& subStr);
	};
}