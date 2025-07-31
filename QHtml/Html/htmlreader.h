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
		static void ParseChildElements(QXmlStreamReader& xml,QList<std::shared_ptr<ElementData>>& elements);
		QString html;
		QList<std::shared_ptr<ElementData>> elementDatas;
		static QWidget* ElementsToQWidegt(const QList<std::shared_ptr<ElementData>>& elements);
		static QList<QString> tags;
	};
}