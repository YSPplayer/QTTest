/*
	Created By YSP
	2025.7.31
*/
#include <stack>
#include <QFile>
#include "htmlreader.h"
#include "listfilter.h"
namespace ysp::qt::html {
#define HTML_ADD_EVENT  R"(
			window.addEventListener('load', function() {
			  const div = document.getElementByKey('%1');
			  if(div !== null) {
				div.addEventListener('%2',%3);
				}
			});
			)"
#define HTML_ADD_EVENT_HOVER R"(
			window.addEventListener('load', function() {
			  const div = document.getElementByKey('%1');
			  if(div !== null) {
				const oldstyle = div.style;
				div.addEventListener('mouseenter',function(e) {
						e.target.style = '%2'; 
					});
				div.addEventListener('mouseleave',(function(style,olddiv){
					return function() {
						olddiv.style = style;
					}
				})(oldstyle,div));
				}
			});
			)"
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
		JsParser* jsParser = &CWidget::jsParser;
		jsParser->Init();
		while (!xml.atEnd() && !xml.hasError()) {
			const QXmlStreamReader::TokenType& token = xml.readNext();
			if (token == QXmlStreamReader::StartElement) {
				const QString& elementName = xml.name().toString().toLower();
				if (elementName == "body") { //第一个节点必须是以body开头
					ParseChildElements(xml, elements, *jsParser);
				}
				else if (elementName == "style") {
					ParseStyleElement(xml, cssParser, cssrules);
				}
			}
		}
		CWidget* widget = ElementsToQWidegt(elements,cssrules,*jsParser);
		return widget;
	}
	void HtmlReader::ParseChildElements(QXmlStreamReader& xml,QList<std::shared_ptr<ElementData>>& elements,JsParser& jsparser) {
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
					jsparser.RunJs(jsscript.toUtf8().constData());//执行脚本
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
	void HtmlReader::ParseAttributes(ElementData* element, const QList<CSSRule*>& rules, QWidget* widget) {
		widget->setAutoFillBackground(true);
		widget->setGeometry(0, 0, 0, 0);
		QMap<QString, QString>& attributes = element->attributes;
		QString id = "";
		QString classname = "";
		//QString classtype = LinkBridge::QClassToHtmlClass(widget->metaObject()->className());
		if (attributes.contains("id")) { //优先提取id
			if (attributes["id"] != "" && !ContainsId(attributes["id"])) { //确保id唯一
				id = attributes["id"];
				widget->setProperty("jsid", id);
			}
		}
		if (attributes.contains("class")) { //优先提取class
			classname = attributes["class"];
			widget->setProperty("class", classname);
		}
		//每一个对象都会有一个独立的id(qt的id 非js id)
		widget->setObjectName(CWidget::GetKeyString(widget));
		//增加样式部分
		/*QString stylename = QString("%1%2").arg(classname != "" ? "." + classname
			: "").arg(id != "" ? "#" + id : "");*/
		const QList<CSSRule*>& filterrules = ListFilter::Where<CSSRule*>(rules, [=](CSSRule* css)->bool {
			return css->CheckRule(widget);
			});
		for (CSSRule* filterrule : filterrules) {
			const QString& selecthander = filterrule->GetSelectorHander();
			if (selecthander == "hover") {
				const QString& jsscript = QString(HTML_ADD_EVENT_HOVER).arg(CWidget::GetKeyString(widget).toUtf8().constData())
					.arg(filterrule->GetPropertiesStyle());
				CWidget::jsParser.RunJs(jsscript.toUtf8().constData());
			}
			else {
				QMap<QString, CSSProperty>& filterattributes = filterrule->properties;
				for (const auto& key : filterattributes.keys()) {
					attributes[filterattributes[key].name] = filterattributes[key].value;
				}
			}
		}
		CWidget::styleBuilder[widget] = StyleBuilder(widget);
		auto& builder = CWidget::styleBuilder[widget];
		//优先判断是否有style修饰，优先解析style中的值
		if (attributes.contains("style")) {
			QMap<QString, QString> styleattributes;
			ParseStyleString(attributes["style"], styleattributes);
			for (const QString& key : styleattributes.keys()) {
				ParseKey(key, widget, builder,styleattributes);
			}
		}
		//解析非style值的标签
		for (const QString& key : attributes.keys()) {
			if (key == "style") continue;
			ParseKey(key,widget, builder,attributes);
		}
		widget->setStyleSheet(builder.ToString());
	}

	/// <summary>
	/// 解析style字符串
	/// </summary>
	/// <param name="style"></param>
	/// <param name="map"></param>
	void HtmlReader::ParseStyleString(const QString& styleValue, QMap<QString, QString>& map) {
		if (styleValue.isEmpty()) return;
		QString value = styleValue.trimmed();
		QStringList pairs = value.split(';', Qt::SkipEmptyParts);
		for (const QString& pair : pairs) {
			const QString& trimmedPair = pair.trimmed();
			if (trimmedPair.isEmpty()) continue;
			qint32 colonIndex = trimmedPair.indexOf(':');
			if (colonIndex > 0) {
				const QString& key = trimmedPair.left(colonIndex).trimmed().toLower();
				const QString& value = trimmedPair.mid(colonIndex + 1).trimmed();
				if (!key.isEmpty()) {
					map[key] = value;
				}
			}
		}

	}
	void HtmlReader::ParseKey(const QString& key, QWidget* widget, StyleBuilder& builder, QMap<QString, QString>& attributes) {
		QWidget* parent = widget->parentWidget() ? widget->parentWidget() : nullptr;
		const QString lkey = key;
		QString value = attributes[key].trimmed();
		if (lkey == "width") {
			widget->resize(ToNumberString(value).toInt(), widget->height());
		}
		else if (lkey == "height") {
			widget->resize(widget->width(), ToNumberString(value).toInt());
		}
		else if (lkey == "top") {
			widget->move(widget->x(), ToNumberString(value).toInt());
		}
		else if (lkey == "left") {
			widget->move(ToNumberString(value).toInt(), widget->y());
		}
		else if (lkey == "bottom" && parent) {
			if (attributes.contains("height")) {
				widget->resize(widget->width(), ToNumberString(attributes["height"]).toInt());
			}
			widget->move(widget->x(), parent->height() - ToNumberString(value).toInt() - widget->height());
		}
		else if (lkey == "right" && parent) {
			if (attributes.contains("width")) {
				widget->resize(ToNumberString(attributes["width"]).toInt(), widget->height());
			}
			widget->move(parent->width() - ToNumberString(value).toInt() - widget->width(), widget->y());
		}
		else if (lkey == "background-color") {
			builder.SetBackgroundColor(value);
		}
		else if (lkey == "border-radius") {
			const QList<QString>& results = Split(value, " ");
			if (results.count() == 4) {
				builder.SetBorderRadius(ToNumberString(results[0]).toInt(),
					ToNumberString(results[1]).toInt(), ToNumberString(results[2]).toInt(),
					ToNumberString(results[3]).toInt());
			}
			else {
				builder.SetBorderRadius(ToNumberString(value).toInt());
			}
		}
		else if (lkey == "onclick" || lkey == "onmousemove" ||
			lkey == "onmouseup" || lkey == "onmousedown" || lkey == "onmouseenter"
			|| lkey == "onmouseleave" || lkey == "ondblclick") { //增加触发事件
			const QString& eventstr = RemoveStrPrefix("on", lkey);
			const QString& func = ExtractFuncString(value);
			const QString& jsscript = QString(HTML_ADD_EVENT).arg(CWidget::GetKeyString(widget).toUtf8().constData()).arg(eventstr).arg(func);
			CWidget::jsParser.RunJs(jsscript.toUtf8().constData());
		}
	}
	
	bool HtmlReader::ContainsKey(QMap<QString, QString>& map,const QString& key) {
		const QString& lowerKey = key.toLower();
		for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
			if (it.key().compare(lowerKey, Qt::CaseInsensitive) == 0) {
				map[lowerKey] = it.value();
				return true;
			}
		}
		return false;
	}

	QList<QString> HtmlReader::Split(const QString& key, const QString& split) {
		QList<QString> resultList;
		QStringList tempList = key.split(split, Qt::SkipEmptyParts);
		for (const QString& item : tempList) {
			resultList.append(item);
		}
		return resultList;
	}


	QString HtmlReader::ExtractFuncString(const QString& input) {
		qint32 pos = input.indexOf('(');
		return (pos != -1) ? input.left(pos) : input;
	}

	QString HtmlReader::RemoveStrPrefix(const QString& key, const QString& eventName) {
		if (eventName.startsWith(key, Qt::CaseInsensitive)) {
			return eventName.mid(2);
		}
		return eventName;
	}

	QString HtmlReader::ToNumberString(const QString& key) {
		qint32 i = 0;
		bool hasDot = false;
		while (i < key.length() && (key[i].isDigit() || (!hasDot && key[i] == '.'))) {
			if (key[i] == '.') hasDot = true;
			i++;
		}
		return key.left(i);
	}

	bool HtmlReader::ContainsId(const QString& id) {
		for (QWidget* widget : CWidget::styleBuilder.keys()) {
			if (!widget) continue;
			const QString& jsid = widget->property("jsid").isValid() ?
				widget->property("jsid").toString() : "";
			if (jsid == id) return true;
		}
		return false;
	}

	CWidget* HtmlReader::ElementsToQWidegt(const QList<std::shared_ptr<ElementData>>& elements, const QList<CSSRule*>& rules, JsParser& jsparser) {
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
			ParseAttributes(element.get(), rules,widget);
			jsparser.CreateDocument(widget);
		}
		return parent;
	}
}
