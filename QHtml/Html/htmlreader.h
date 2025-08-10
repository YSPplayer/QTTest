/*
	Created By YSP
	2025.7.31
*/
#pragma once
#include <memory>
#include <QXmlStreamReader>
#include <QObject>
#include <QWidget>
#include <QMap>
#include <QList>
#include "stylebuilder.h"
#include "cssparser.h"
#include "cwidget.h"
namespace ysp::qt::html {
	struct ElementData {
		ElementData* parent{ nullptr };
		QString tag{""};
		QMap<QString, QString> attributes{};
		QString text{""};
	};
	class HtmlReader :public QObject{
		Q_OBJECT
	public:
		HtmlReader(const QString& filePath);
		CWidget* Parse();
	private:
		static void ParseChildElements(QXmlStreamReader& xml,QList<std::shared_ptr<ElementData>>& elements,JsParser& jsparser);
		static void ParseStyleElement(QXmlStreamReader& xml,CSSParser& parser, QList<CSSRule*>& rules);
		QString html;
		QList<std::shared_ptr<ElementData>> elementDatas;
		static CWidget* ElementsToQWidegt(const QList<std::shared_ptr<ElementData>>& elements, const QList<CSSRule*>& rules, JsParser& jsparser);
		static void ParseAttributes(ElementData* element, const QList<CSSRule*>& rules,QWidget* widget);
		static void ParseStyleString(const QString& styleValue, QMap<QString, QString>& map);
		static void ParseKey(const QString& key,QWidget* widget, StyleBuilder&builder,QMap<QString, QString>& attributes);
		static bool ContainsKey(QMap<QString, QString>& map,const QString& key);
		static QList<QString> Split(const QString& key,const QString& split);
		static QString QClassToHtmlClass(const QString& name);
	};
}