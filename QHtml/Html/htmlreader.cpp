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
		for (auto& element : elements) {
			TagToQWidegt(element->tag);
		}
		return nullptr;
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
                if (xml.name().toString() == "body" && elementStack.empty()) {
                    break;
                }
            }
        }
	
	}
	QWidget* ElementsToQWidegt(const QList<std::shared_ptr<ElementData>>& elements) {
		QMap<ElementData*, QWidget*> map;
		QWidget* parent = new QWidget;
		for (auto& element : elements) {
			QWidget* widget = nullptr;
			if (element->tag == "div") {
				widget = new QWidget;
			}
			map[element.get()] = widget;
			if (element->parent != nullptr && map.contains(element->parent)) {
				widget->setParent(map[element->parent]);
			}
			else {
				widget->setParent(parent);
			}
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
