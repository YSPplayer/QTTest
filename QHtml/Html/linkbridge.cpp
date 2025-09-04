/*
	Created By YSP
	2025.8.3
*/
#include <QDebug>
#include <QFontDatabase>
#include <QApplication>
#include "listfilter.h"
#include "linkbridge.h"
#include "include.h"
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
	JsParser LinkBridge::jsParser;
			QMap<QWidget*, StyleBuilder> LinkBridge::styleBuilder;
			QList<CSSRule*> LinkBridge::cssrules;
			void LinkBridge::Print(const char* str) {
#ifdef _DEBUG
				qDebug() << str;
#endif // _DEBUG
				CWidget::AppendConsoleWindowMsg(QString::fromUtf8(str));
			}
			QMap<QString, QString> LinkBridge::classmap = { {"QWidget","div"},{"QLabel","label"},
				{"QProgressBar","progress"},{"QComboBox","select"} };
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, QResizeEvent* event, bool global) {
				JsClass* obj = new JsClass;
				(*obj)["oldOffsetWidth"] = JsValue::CreateValue(event->oldSize().width());
				(*obj)["oldOffsetHeight"] = JsValue::CreateValue(event->oldSize().height());
				(*obj)["offsetWidth"] = JsValue::CreateValue(event->size().width());
				(*obj)["offsetHeight"] = JsValue::CreateValue(event->size().height());
				(*obj)["target"] = JsValue::CreateObjectValue(target.toStdString());
				jsParser.Trigger(key, JsValue::CreateValue(obj), global);
			}
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, QWheelEvent* event, bool global) {
				JsClass* obj = new JsClass;
				(*obj)["deltaX"] = JsValue::CreateValue(event->angleDelta().x());
				(*obj)["deltaY"] = JsValue::CreateValue(event->angleDelta().y());
				(*obj)["pixelX"] = JsValue::CreateValue(event->pixelDelta().x());
				(*obj)["pixelY"] = JsValue::CreateValue(event->pixelDelta().y());
				(*obj)["ctrlKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ControlModifier));
				(*obj)["shiftKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ShiftModifier));
				jsParser.Trigger(key, JsValue::CreateValue(obj), global);
			}
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, QKeyEvent* event, bool global) {
				JsClass* obj = new JsClass;
				(*obj)["key"] = JsValue::CreateValue(QKeySequence(event->key()).toString().toLower().toStdString());
				(*obj)["code"] = JsValue::CreateValue(QString("Key" + QKeySequence(event->key()).toString().toLower()).toStdString());
				(*obj)["ctrlKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ControlModifier));
				(*obj)["shiftKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ShiftModifier));
				jsParser.Trigger(key, JsValue::CreateValue(obj), global);
			}
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, QCloseEvent* event, bool global) {
				jsParser.Trigger(key, global);
			}
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, QMouseEvent* event, bool global) {
				auto button = event->button();
				JsClass* obj = new JsClass;
				(*obj)["button"] = JsValue::CreateValue(button == Qt::LeftButton ? 0 : button == Qt::MiddleButton ?
					1 : button == Qt::RightButton ? 2 : -1);
				(*obj)["clientX"] = JsValue::CreateValue(event->position().x());
				(*obj)["clientY"] = JsValue::CreateValue(event->position().y());
				(*obj)["screenX"] = JsValue::CreateValue(event->globalPosition().x());
				(*obj)["screenY"] = JsValue::CreateValue(event->globalPosition().y());
				(*obj)["offsetX"] = JsValue::CreateValue(event->pos().x());
				(*obj)["offsetY"] = JsValue::CreateValue(event->pos().y());
				(*obj)["ctrlKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ControlModifier));
				(*obj)["shiftKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ShiftModifier));
				(*obj)["target"] = JsValue::CreateObjectValue(target.toStdString());
				jsParser.Trigger(key, JsValue::CreateValue(obj), global);
			}
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, QEnterEvent* event, bool global) {
				JsClass* obj = new JsClass;
				(*obj)["clientX"] = JsValue::CreateValue(event->position().x());
				(*obj)["clientY"] = JsValue::CreateValue(event->position().y());
				(*obj)["target"] = JsValue::CreateObjectValue(target.toStdString());
				jsParser.Trigger(key, JsValue::CreateValue(obj), global);
			}
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, QEvent* event, bool global) {
				jsParser.Trigger(key, global);
			}
			void LinkBridge::TriggerJsEvent(const QString& target, const QString& key, bool global) {
				JsClass* obj = new JsClass;
				(*obj)["target"] = JsValue::CreateObjectValue(target.toStdString());
				jsParser.Trigger(key, JsValue::CreateValue(obj), global);
			}
			void LinkBridge::TriggerJsEvent(const QString& key, bool global) {
				jsParser.Trigger(key, global);
			}
			bool LinkBridge::ContainsId(const QString& id) {
				for (QWidget* widget : LinkBridge::styleBuilder.keys()) {
					if (!widget) continue;
					const QString& jsid = widget->property("jsid").isValid() ?
						widget->property("jsid").toString() : "";
					if (jsid == id) return true;
				}
				return false;
			}
			void LinkBridge::ParseAttributes(ElementData* element, QWidget* widget) {
				widget->setAutoFillBackground(true);
				widget->setGeometry(0, 0, 0, 0);
				QMap<QString, QString>& attributes = element->attributes;
				QString id = "";
				QString classname = "";
				if (attributes.contains("id")) { //优先提取id
					if (attributes["id"] != "" && !ContainsId(attributes["id"])) { //确保id唯一
						id = attributes["id"];
						widget->setProperty("jsid", id);
					}
				}
				//每一个对象都会有一个独立的id(qt的id 非js id)
				widget->setObjectName(CWidget::GetKeyString(widget));
				if (attributes.contains("class")) { //优先提取class
					classname = attributes["class"];
					widget->setProperty("class", classname);
				}
				//增加样式部分
				const QList<CSSRule*>& filterrules = ListFilter::Where<CSSRule*>(cssrules, [=](CSSRule* css)->bool {
					return css->CheckRule(widget);
					});
				for (CSSRule* filterrule : filterrules) {
					const QString& selecthander = filterrule->GetSelectorHander();
					if (selecthander == "hover") {
						const QString& jsscript = QString(HTML_ADD_EVENT_HOVER).arg(CWidget::GetKeyString(widget).toUtf8().constData())
							.arg(filterrule->GetPropertiesStyle());
						LinkBridge::jsParser.RunJs(jsscript.toUtf8().constData());
					}
					else {
						QMap<QString, CSSProperty>& filterattributes = filterrule->properties;
						for (const auto& key : filterattributes.keys()) {
							attributes[filterattributes[key].name] = filterattributes[key].value;
						}
					}
				}
				if (attributes.contains("font-family")) { //提取字体
					const QStringList& fontPriority = attributes["font-family"].split(",");
					QFontDatabase fontDB;
					const QStringList& allFonts = fontDB.families();
					for (const QString& fontName : fontPriority) {
						if (allFonts.contains(fontName)) {
							QFont font = QApplication::font();
							font.setFamily(fontName);
							widget->setFont(font);
						}
					}
				}
				LinkBridge::styleBuilder[widget] = StyleBuilder(widget);
				auto& builder = LinkBridge::styleBuilder[widget];
				//优先判断是否有style修饰，优先解析style中的值
				if (attributes.contains("style")) {
					QMap<QString, QString> styleattributes;
					ParseStyleString(attributes["style"], styleattributes);
					for (const QString& key : styleattributes.keys()) {
						ParseKey(key, widget, builder, styleattributes);
					}
				}
				//解析非style值的标签
				for (const QString& key : attributes.keys()) {
					if (key == "style") continue;
					ParseKey(key, widget, builder, attributes);
				}
				QString bulider = builder.ToString();
				if (bulider != "") widget->setStyleSheet(bulider);
			}
			void LinkBridge::ParseAttributesBody(QWidget* widget) {
				QMap<QString, QString> attributes;
				const QList<CSSRule*>& filterrules = ListFilter::Where<CSSRule*>(cssrules, [=](CSSRule* css)->bool {
					return css->selector == "body";
					});
				for (CSSRule* filterrule : filterrules) {
					QMap<QString, CSSProperty>& filterattributes = filterrule->properties;
					for (const auto& key : filterattributes.keys()) {
						attributes[filterattributes[key].name] = filterattributes[key].value;
					}
				}
				LinkBridge::styleBuilder[widget] = StyleBuilder(widget);
				auto& builder = LinkBridge::styleBuilder[widget];
				for (const QString& key : attributes.keys()) {
					if (key == "style") continue;
					ParseKey(key, widget, builder, attributes);
				}
				QString bulider = builder.ToString();
				if (bulider != "") widget->setStyleSheet(bulider);
			}
			void LinkBridge::ParseStyleString(const QString& styleValue, QMap<QString, QString>& map) {
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
			void LinkBridge::ParseKey(const QString& key, QWidget* widget, StyleBuilder& builder, QMap<QString, QString>& attributes) {
				QWidget* parent = widget->parentWidget() ? widget->parentWidget() : nullptr;
				QString classname = QString(widget->metaObject()->className());
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
				else if (lkey == "font-size") {
					QFont font = widget->font();
					font.setPointSize(ToNumberString(value).toInt());
					widget->setFont(font);
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
					LinkBridge::jsParser.RunJs(jsscript.toUtf8().constData());
				}
				else if (classname == "QLabel") {
					CLabel* clabel = ((CLabel*)widget);
					if (lkey == "text-align" && value == "center") {
						clabel->setAlignment(Qt::AlignCenter);
					}
				}
				else if (classname == "QProgressBar") {
					QProgressBar* bar = ((QProgressBar*)widget);
					if (lkey == "min") {
						bar->setMinimum(ToNumberString(value).toInt());
					}
					else if (lkey == "max") {
						bar->setMaximum(ToNumberString(value).toInt());
					}
					else if (lkey == "value") {
						bar->setValue(ToNumberString(value).toInt());
					}
				}
			}
			QString LinkBridge::ToNumberString(const QString& key) {
				qint32 i = 0;
				bool hasDot = false;
				while (i < key.length() && (key[i].isDigit() || (!hasDot && key[i] == '.'))) {
					if (key[i] == '.') hasDot = true;
					i++;
				}
				return key.left(i);
			}

			QList<QString> LinkBridge::Split(const QString& key, const QString& split) {
				QList<QString> resultList;
				QStringList tempList = key.split(split, Qt::SkipEmptyParts);
				for (const QString& item : tempList) {
					resultList.append(item);
				}
				return resultList;
			}
			QString LinkBridge::RemoveStrPrefix(const QString& key, const QString& eventName) {
				if (eventName.startsWith(key, Qt::CaseInsensitive)) {
					return eventName.mid(2);
				}
				return eventName;
			}
			QString LinkBridge::ExtractFuncString(const QString& input) {
				qint32 pos = input.indexOf('(');
				return (pos != -1) ? input.left(pos) : input;
			}
			QString LinkBridge::QClassToHtmlClass(const QString& name) {
				return classmap.contains(name) ? classmap[name] : "";
			}
			QString LinkBridge::HtmlClassToQClass(const QString& name) {
				for (auto it = classmap.begin(); it != classmap.end(); ++it) {
					if (it.value() == name) {
						return it.key();
					}
				}
				return QString("");
			}
			QString LinkBridge::ReplaceAfterHash(QString input, const QString& replacement) {
				qint32 hashPos = input.indexOf('#');
				if (hashPos == -1) return input;

				// 检查 # 是否在颜色值中（前面有冒号）
				if (hashPos > 0 && input.at(hashPos - 1) == ':') {
					return input;
				}

				// 检查 # 是否在属性值中（前面有空格或冒号）
				if (hashPos > 0) {
					QChar prevChar = input.at(hashPos - 1);
					if (prevChar == ':' || prevChar == ' ') {
						return input;
					}
				}

				QList<QChar> filter = { '_','-', '\\','/' };
				qint32 endPos = hashPos + 1;
				while (endPos < input.length()) {
					QChar c = input.at(endPos);
					if (!c.isLetterOrNumber() && !filter.contains(c)) {
						break;
					}
					endPos++;
				}
				return input.replace(hashPos + 1, endPos - hashPos - 1, replacement);
			}
			qint32 LinkBridge::FindSubstringEndIndex(const QString& mainStr, const QString& subStr) {
				if (subStr.isEmpty() || mainStr.isEmpty()) {
					return -1;
				}
				qint32 index = mainStr.indexOf(subStr);
				if (index == -1) {
					return -1;  // 子串不存在
				}
				return index + subStr.length() - 1;  // 返回结束索引
			}
}
