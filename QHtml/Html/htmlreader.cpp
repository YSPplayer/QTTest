/*
	Created By YSP
	2025.7.31
*/
#include <stack>
#include <QFile>
#include "htmlreader.h"
#include "listfilter.h"
#include "cprogressbar.h"
#include "clabel.h"
#include "jscompiler.h"
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
		html = JsCompiler::ToCompilerScript(html, true);//替换特殊字符
	}
	CWidget* HtmlReader::Parse() {
		QXmlStreamReader xml(html);
		QList<std::shared_ptr<ElementData>> elements;
		QList<std::shared_ptr<ElementData>> customs;
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
				else if (elementName == "custom") { //自定义组件
					ParseCustomElements(xml, customs);
				}
				//qDebug() << "elementName:" << customs;
			}
		}
		//全局样式
		if (cssrules.count() > 0) LinkBridge::cssrules.append(cssrules);
		CWidget* widget = ElementsToQWidegt(elements, customs);
		return widget;
	}

	void HtmlReader::ParseChildElements(QXmlStreamReader& xml, QList<std::shared_ptr<ElementData>>& elements) {
		std::stack<std::shared_ptr<ElementData>> elementStack;
		bool hasjs = false;
		QString jsscript = "";
		while (!xml.atEnd() && !xml.hasError()) {
			QXmlStreamReader::TokenType token = xml.readNext();
			if (token == QXmlStreamReader::StartElement) {
				const QString& elementName = xml.name().toString().toLower();
				if (elementName == "script") {
					hasjs = true;
					jsscript.clear();
				}
				else {
					auto data = std::make_shared<ElementData>();
					data->parent = elementStack.empty() ? nullptr : elementStack.top();
					if (data->parent) data->parent->childs.append(data);
					data->tag = xml.name().toString();
					const QXmlStreamAttributes& attributes = xml.attributes();
					for (const QXmlStreamAttribute& attr : attributes) {
						data->attributes[attr.name().toString().toLower()] = attr.value().toString();
					}
					elements.append(data);
					elementStack.push(data);
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
					//转化回来
					jsscript.replace("&lt;", "<");
					jsscript.replace("&gt;", ">");
					//jsscript.replace("&amp;", "&");
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
	void HtmlReader::ParseCustomElements(QXmlStreamReader& xml, QList<std::shared_ptr<ElementData>>& elements) {
		std::stack<std::shared_ptr<ElementData>> elementStack;
		while (!xml.atEnd() && !xml.hasError()) {
			QXmlStreamReader::TokenType token = xml.readNext();
			if (token == QXmlStreamReader::StartElement) {
				const QString& elementName = xml.name().toString().toLower();
				auto data = std::make_shared<ElementData>();
				data->parent = elementStack.empty() ? nullptr : elementStack.top();
				if (data->parent) data->parent->childs.append(data);
				data->tag = xml.name().toString();
				const QXmlStreamAttributes& attributes = xml.attributes();
				for (const QXmlStreamAttribute& attr : attributes) {
					data->attributes[attr.name().toString().toLower()] = attr.value().toString();
				}
				elements.append(data);
				elementStack.push(data);
			}
			else if (token == QXmlStreamReader::Characters) {
				const QString& text = xml.text().toString().trimmed();
				if (!text.isEmpty() && !elementStack.empty())
				{
					elementStack.top()->text = text;
				}
			}
			else if (token == QXmlStreamReader::EndElement) {
				const QString& elementName = xml.name().toString().toLower();
				if (!elementStack.empty()) {
					elementStack.pop();
				}
				if (elementName == "custom" && elementStack.empty()) {
					break;
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
	CWidget* HtmlReader::ElementsToQWidegt(QList<std::shared_ptr<ElementData>>& elements,
		QList<std::shared_ptr<ElementData>>& customs) {
		QMap<std::shared_ptr<ElementData>, QWidget*> map;
		CWidget* parent = new CWidget(true);
		parent->resize(1600, 900);
		//设置body样式
		LinkBridge::ParseAttributesBody(parent);
		for (qint32 i = 0; i < elements.size(); ++i) {
			auto& element = elements[i];
			if (!element.get()) continue;
			QWidget* widget = nullptr;
			if (element->tag == "div") widget = new CWidget;
			else if (element->tag == "progress") {
				widget = new CProgressBar;
			}
			else if (element->tag == "label") {
				CLabel* label = new CLabel;
				label->setText(element->text);
				widget = label;
			}
			else {
				/*
				element->tag: custom->xxxx
				custom[0]->tag: custom->xxxx
				*/
				std::shared_ptr<ElementData> eparent = element->parent;
				//自定义组件
				const auto& custom = ListFilter::Where<std::shared_ptr<ElementData>>(
					customs, [&element](const std::shared_ptr<ElementData>& edata)->bool {
						return element->tag == edata->tag;
					});
				if (custom.count() > 0) {
					const std::shared_ptr<ElementData>& cdata = custom[0];
					const auto& childs = ListFilter::Where<std::shared_ptr<ElementData>>(
						customs, [&element](const std::shared_ptr<ElementData>& edata)->bool {
							return edata->parent != nullptr && edata->parent->tag == element->tag;
						});
					std::stack<QList<std::shared_ptr<ElementData>>> stack;
					stack.push(childs);
					while (!stack.empty()) {
						const auto currentChilds = stack.top();
						stack.pop();
						for (const auto& child : currentChilds) {
							auto cdata = std::make_shared<ElementData>();
							cdata->parent = child->parent &&
								child->parent->tag == element->tag ? eparent :
								child->parent;
							if (cdata->parent) cdata->parent->childs.append(cdata);
							cdata->attributes = child->attributes;
							cdata->tag = child->tag;
							cdata->text = child->text;
							elements.push_back(cdata);
							// 如果有子元素，压入栈
							if (!child->childs.isEmpty()) {
								stack.push(child->childs);
							}
						}
					}
					continue;
				}
			}
			if (!widget) continue;
			map[element] = widget;
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
