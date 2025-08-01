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
                    data->attributes[attr.name().toString()] = attr.value().toString();
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
		//优先判断是否有style修饰，优先解析style中的值
		if (ContainsKey(attributes, "style")) {
			QMap<QString, QString> styleattributes;
			ParseStyleString(attributes["style"], styleattributes);
			for (const QString& key : styleattributes.keys()) {
				ParseKey(key, widget, styleattributes);
			}
		}
		//解析非style值的标签
		for (const QString& key : attributes.keys()) {
			ParseKey(key,widget, attributes);
		}
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
				const QString& key = trimmedPair.left(colonIndex).trimmed();
				const QString& value = trimmedPair.mid(colonIndex + 1).trimmed();
				if (!key.isEmpty()) {
					map[key] = value;
				}
			}
		}

	}
	void HtmlReader::ParseKey(const QString& key, QWidget* widget, QMap<QString, QString>& attributes) {
		QWidget* parent = widget->parentWidget() ? widget->parentWidget() : nullptr;
		const QString& lkey = key.toLower();
		QString value = attributes[key].trimmed();
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
			QPalette palette = widget->palette();
			palette.setColor(QPalette::Window, QColor(value));
			widget->setPalette(palette);
		}
	}
	bool HtmlReader::ContainsKey(const QMap<QString, QString>& map, const QString& key) {
		QString lowerKey = key.toLower();
		for (const QString& mapKey : map.keys()) {
			if (mapKey.toLower() == lowerKey) {
				return true;
			}
		}
		return false;
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
