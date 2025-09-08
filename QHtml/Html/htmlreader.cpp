/*
	Created By YSP
	2025.7.31
*/
#include <stack>
#include <QFile>
#include "htmlreader.h"
#include "listfilter.h"
#include "include.h"
#include "jscompiler.h"
namespace ysp::qt::html {
	HtmlReader::HtmlReader(const QString& filePath) {
		rootwidget = nullptr;
		html = "";
		this->filePath = filePath;
		LoadHtml();
	}
	CWidget* HtmlReader::Parse() {
		QXmlStreamReader xml(html);
		QList<std::shared_ptr<ElementData>> elements;
		QList<std::shared_ptr<ElementData>> customs;
		CSSParser cssParser;
		cssrules.clear();
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
		rootwidget = ElementsToQWidegt(elements, customs);
		return rootwidget;
	}

	/// <summary>
	/// 清空所有的组件内容
	/// </summary>
	void HtmlReader::Clear() {
		if (!rootwidget) return;
		//剔除css
		for (CSSRule* rule : cssrules) {
			LinkBridge::cssrules.removeAll(rule);
		}
		for (CSSRule* rule : cssrules) {
			delete rule;
			rule = nullptr;
		}
		QString key = CWidget::GetKeyString(rootwidget);
		rootwidget->hide();
		rootwidget->setParent(nullptr);
		rootwidget = nullptr;
		//剔除js
		QString script = R"(
				function _$remove_body_%1() {
					const body = document.getElementByKey('%1');
					if(body == null) return;
					body.children.forEach(function(child){
						if(child != null) {
							child.delete();
							child = null;
						}
					});
					body.delete();
					body = null;
				}
				_$remove_body_%1();
				_$remove_body_%1 = null;
			)";
		LinkBridge::jsParser.RunJs(script.arg(key).toUtf8().constData());
	}

	CWidget* HtmlReader::ReParse() {
		Clear();
		LoadHtml();
		return Parse();
	}

	bool HtmlReader::LoadHtml() {
		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << "[HtmlReader]could not open file:" << filePath;
			qDebug() << "[HtmlReader]error:" << file.errorString();
			return false;
		}
		const QByteArray& data = file.readAll();
		file.close();
		html = QString::fromUtf8(data);
		html = JsCompiler::ToCompilerScript(html, true);//替换特殊字符
		return true;
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
			if (element->tag == "div") {
				widget = new CWidget;
			}
			else if (element->tag == "progress") {
				widget = new CProgressBar;
			}
			else if (element->tag == "select") {
				widget = new CComboBox;
			}
			else if (element->tag == "label") {
				CLabel* label = new CLabel;
				label->setText(element->text);
				widget = label;
			}
			else if (element->tag == "img") {
				CImage* image = new CImage;
				qDebug() << image->metaObject()->className();
				widget = image;
			}
			else if (element->tag == "option") {
				if (element->parent == nullptr || element->parent->tag != "select") {
					continue;
				}
				QComboBox* combox = (QComboBox*)map[element->parent];
				QMap<QString, QString>& attributes = element->attributes;
				combox->addItem(element->text, element->attributes.value("value", ""));
				continue;
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
						for (auto& child : currentChilds) {
							if (child->parent) {
								const QString key = LinkBridge::HtmlClassToQClass(child->parent->tag);
								if (key == "") child->parent = eparent;
								//child->parent->childs.append(cdata);
							}
							elements.push_back(child);
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
			if (element->parent != nullptr) {
				bool success = false;
				for (const auto& key : map.keys()) {
					if (element->parent.get() == key.get()) {
						widget->setParent(map[key]);
						success = true;
						break;
					}
				}
				if (!success)widget->setParent(parent);
			}
			else {
				widget->setParent(parent);
			}
			map[element] = widget;
			LinkBridge::ParseAttributes(element.get(), widget);
			LinkBridge::jsParser.CreateDocument(widget);
		}
		LinkBridge::jsParser.CreateDocument(parent);
		return parent;
	}
}
