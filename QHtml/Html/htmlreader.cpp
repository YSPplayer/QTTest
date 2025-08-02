/*
	Created By YSP
	2025.7.31
*/
#include <stack>
#include <QFile>
#include "htmlreader.h"
namespace ysp::qt::html {
	QList<QString> HtmlReader::tags = {"div"};
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
	QWidget* HtmlReader::Parse() {
		QXmlStreamReader xml(html);
		QList<std::shared_ptr<ElementData>> elements;
		while (!xml.atEnd() && !xml.hasError()) {
			const QXmlStreamReader::TokenType& token = xml.readNext();
			if (token == QXmlStreamReader::StartElement) {
				const QString& elementName = xml.name().toString();
				if (elementName == "body") { //第一个节点必须是以body开头
					ParseChildElements(xml, elements);
				}
			}
		}
		return ElementsToQWidegt(elements);
	}
	void HtmlReader::ParseChildElements(QXmlStreamReader& xml,QList<std::shared_ptr<ElementData>>& elements) {
        std::stack<ElementData*> elementStack;
        while (!xml.atEnd() && !xml.hasError()) {
            QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::StartElement) {
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
            else if (token == QXmlStreamReader::Characters) {
                const QString& text = xml.text().toString().trimmed();
                if (!text.isEmpty() && !elementStack.empty())
                {
                    elementStack.top()->text = text;
                }
            }
            else if (token == QXmlStreamReader::EndElement) {
                if (!elementStack.empty()) {
                    elementStack.pop();
                }
                if (xml.name().toString().toLower() == "body" && elementStack.empty()) {
                    break;
                }
            }
        }
	
	}
	void HtmlReader::ParseAttributes(ElementData* element, QWidget* widget) {
		widget->setAutoFillBackground(true);
		widget->setGeometry(0, 0, 0, 0);
		QMap<QString, QString>& attributes = element->attributes;
		if (attributes.contains("id")) { //优先提取id
			widget->setObjectName(attributes["id"]);
		}
		if (attributes.contains("class")) { //优先提取id
			widget->setProperty("class",attributes["class"]);
		}
		StyleBuilder builder(widget);
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
		const QString& lkey = key;
		QString value = attributes[key].trimmed();
		StyleBuilder style(widget);
		if (lkey == "width") {
			widget->resize(value.toInt(), widget->height());
		}
		else if (lkey == "height") {
			widget->resize(widget->width(), value.toInt());
		}
		else if (lkey == "top") {
			widget->move(widget->x(), value.toInt());
		}
		else if (lkey == "left") {
			widget->move(value.toInt(), widget->y());
		}
		else if (lkey == "bottom" && parent) {
			if (ContainsKey(attributes, "height")) {
				widget->resize(widget->width(), attributes["height"].toInt());
			}
			widget->move(widget->x(), parent->height() - value.toInt() - widget->height());
		}
		else if (lkey == "right" && parent) {
			if (ContainsKey(attributes, "width")) {
				widget->resize(attributes["width"].toInt(), widget->height());
			}
			widget->move(parent->width() - value.toInt() - widget->width(), widget->y());
		}
		else if (lkey == "background-color") {
			builder.SetBackgroundColor(value);
		}
		else if (lkey == "border-radius") {
			const QList<QString> results = Split(value," ");
			if (results.count() == 4) {
				builder.SetBorderRadius(results[0].toInt(),
					results[1].toInt(), results[2].toInt(), results[3].toInt());
			}
			else {
				builder.SetBorderRadius(value.toInt());
			}
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

	QWidget* HtmlReader::ElementsToQWidegt(const QList<std::shared_ptr<ElementData>>& elements) {
		QMap<ElementData*, QWidget*> map;
		QWidget* parent = new QWidget;
		parent->resize(1600, 900);
		for (auto& element : elements) {
			QWidget* widget = nullptr;
			if (element->tag.toLower() == "div") {
				widget = new QWidget;
			}
			map[element.get()] = widget;
			if (element->parent != nullptr && map.contains(element->parent)) {
				widget->setParent(map[element->parent]);
			}
			else {
				widget->setParent(parent);
			}
			ParseAttributes(element.get(), widget);
		}
		//没有数据
		if (parent->children().isEmpty()) {
			parent->deleteLater();
			return nullptr;
		}
		else {
			return parent;
		}
	}
}
