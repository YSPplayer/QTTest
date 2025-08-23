/*
	Created By YSP
	2025.7.31
*/
#include <stack>
#include <QFile>
#include "htmlreader.h"
#include "listfilter.h"
namespace ysp::qt::html {
	HtmlReader::HtmlReader(const QString& filePath) {
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			qDebug() << "[HtmlReader]could not open file:" << filePath;
			qDebug() << "[HtmlReader]error:" << file.errorString();
			return;
		}
		const QByteArray& data = file.readAll();
		file.close();
		html = QString::fromUtf8(data);
	}
	CWidget* HtmlReader::Parse() {
		QXmlStreamReader xml(html);
		QList<std::shared_ptr<ElementData>> elements;
		QList<CSSRule*> cssrules;
		CSSParser cssParser;
		LinkBridge::jsParser.Init();//初始化全局JS环境
		while (!xml.atEnd() && !xml.hasError()) {
			const QXmlStreamReader::TokenType& token = xml.readNext();
			if (token == QXmlStreamReader::StartElement) {
				const QString& elementName = xml.name().toString().toLower();
				if (elementName == "body") { //第一个节点必须是以body开头
					ParseChildElements(xml, elements);
				}
				else if (elementName == "style") {
					ParseStyleElement(xml, cssParser, cssrules);
				}
			}
		}
		//全局样式
		if (cssrules.count() > 0) LinkBridge::cssrules.append(cssrules);
		CWidget* widget = ElementsToQWidegt(elements);
		return widget;
	}
	void HtmlReader::ParseChildElements(QXmlStreamReader& xml, QList<std::shared_ptr<ElementData>>& elements) {
		std::stack<ElementData*> elementStack;
		bool hasjs = false;
		QString jsscript = "";
		while (!xml.atEnd() && !xml.hasError()) {
			QXmlStreamReader::TokenType token = xml.readNext();
			if (token == QXmlStreamReader::StartElement) {
				const QString& elementName = xml.name().toString().toLower();
				if (elementName == "script") {
					hasjs = true;
				}
				else {
					auto data = std::make_shared<ElementData>();
					data->parent = elementStack.empty() ? nullptr : elementStack.top();
					data->tag = xml.name().toString();
					const QXmlStreamAttributes& attributes = xml.attributes();
					for (const QXmlStreamAttribute& attr : attributes) {
						data->attributes[attr.name().toString().toLower()] = attr.value().toString();
					}
					elements.append(data);
					elementStack.push(data.get());
				}
			}
			else if (token == QXmlStreamReader::Characters) {
				if (hasjs) {
					jsscript += xml.text().toString();
				}
				else {
					const QString& text = xml.text().toString().trimmed();
					if (!text.isEmpty() && !elementStack.empty())
					{
						elementStack.top()->text = text;
					}
				}
			}
			else if (token == QXmlStreamReader::EndElement) {
				const QString& elementName = xml.name().toString().toLower();
				if (elementName == "script") {
					hasjs = false;
					LinkBridge::jsParser.RunJs(jsscript.toUtf8().constData());//执行脚本
					jsscript = "";
				}
				else {
					if (!elementStack.empty()) {
						elementStack.pop();
					}
					if (elementName == "body" && elementStack.empty()) {
						break;
					}
				}
			}
		}

	}
	void HtmlReader::ParseStyleElement(QXmlStreamReader& xml, CSSParser& parser, QList<CSSRule*>& rules) {
		QString styleContent;
		while (!xml.atEnd() && !xml.hasError()) {
			QXmlStreamReader::TokenType nextToken = xml.readNext();
			if (nextToken == QXmlStreamReader::Characters) {
				// 获取文本内容
				styleContent += xml.text().toString();
			}
			else if (nextToken == QXmlStreamReader::EndElement) {
				// 遇到结束标签，退出循环
				if (xml.name().toString().toLower() == "style") {
					break;
				}
			}
		}
		if (parser.parseCSS(styleContent)) {
			rules = parser.getRules();
		}
		else {
			rules.clear();
		}

	}

	bool HtmlReader::ContainsKey(QMap<QString, QString>& map, const QString& key) {
		const QString& lowerKey = key.toLower();
		for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
			if (it.key().compare(lowerKey, Qt::CaseInsensitive) == 0) {
				map[lowerKey] = it.value();
				return true;
			}
		}
		return false;
	}
	CWidget* HtmlReader::ElementsToQWidegt(const QList<std::shared_ptr<ElementData>>& elements) {
		QMap<ElementData*, QWidget*> map;
		CWidget* parent = new CWidget(true);
		parent->resize(1600, 900);
		for (auto& element : elements) {
			CWidget* widget = nullptr;
			if (element->tag == "div") {
				widget = new CWidget;
			}
			if (!widget) continue;
			map[element.get()] = widget;
			if (element->parent != nullptr && map.contains(element->parent)) {
				widget->setParent(map[element->parent]);
			}
			else {
				widget->setParent(parent);
			}
			LinkBridge::ParseAttributes(element.get(), widget);
			LinkBridge::jsParser.CreateDocument(widget);
		}
		return parent;
	}
}
