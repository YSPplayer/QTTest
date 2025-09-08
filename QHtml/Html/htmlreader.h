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
#include "linkbridge.h"
namespace ysp::qt::html {
	class HtmlReader :public QObject {
		Q_OBJECT
	public:
		HtmlReader(const QString& filePath);
		CWidget* Parse();
		void Clear();
		CWidget* ReParse();
	private:
		bool LoadHtml();
		static void ParseChildElements(QXmlStreamReader& xml, QList<std::shared_ptr<ElementData>>& elements);
		static void ParseCustomElements(QXmlStreamReader& xml, QList<std::shared_ptr<ElementData>>& elements);
		static void ParseStyleElement(QXmlStreamReader& xml, CSSParser& parser, QList<CSSRule*>& rules);
		QList<CSSRule*> cssrules;
		QString html;
		QString filePath;
		CWidget* rootwidget;
		static CWidget* ElementsToQWidegt(QList<std::shared_ptr<ElementData>>& elements,
			QList<std::shared_ptr<ElementData>>& customs);
		static bool ContainsKey(QMap<QString, QString>& map, const QString& key);
	};
}