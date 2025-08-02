/*
	Created By YSP
	2025.7.31
*/
#pragma once
#include <memory>
#include <QXmlStreamReader>
#include <QWidget>
#include <QMap>
#include <QList>
#include "stylebuilder.h"
#include "cssparser.h"
namespace ysp::qt::html {
	struct ElementData {
		ElementData* parent{ nullptr };
		QString tag{""};
		QMap<QString, QString> attributes{};
		QString text{""};
	};
	class HtmlReader {
	public:
		HtmlReader(const QString& filePath);
		QWidget* Parse();
	private:
		CSSParser parser;
		static void ParseChildElements(QXmlStreamReader& xml,QList<std::shared_ptr<ElementData>>& elements);
		QString html;
		QList<std::shared_ptr<ElementData>> elementDatas;
		static QWidget* ElementsToQWidegt(const QList<std::shared_ptr<ElementData>>& elements);
		static QList<QString> tags;
		static void ParseAttributes(ElementData* element,QWidget* widget);
		static void ParseStyleString(const QString& styleValue, QMap<QString, QString>& map);
		static void ParseKey(const QString& key,QWidget* widget, StyleBuilder&builder,QMap<QString, QString>& attributes);
		static bool ContainsKey(QMap<QString, QString>& map,const QString& key);
		static QList<QString> Split(const QString& key,const QString& split);
	};
}