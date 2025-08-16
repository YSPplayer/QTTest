/*
	Created By YSP
	2025.8.3
*/
#include <QDebug>
#include "linkbridge.h"
#include "cwidget.h"
namespace ysp::qt::html {
	void LinkBridge::Print(const char* str) {
		qDebug() << str;
		CWidget::AppendConsoleWindowMsg(QString::fromUtf8(str));
	}
	void LinkBridge::TriggerJsEvent(JsParser& jsParser, const QString& key, QResizeEvent* event, bool global) {
		JsClass* obj = new JsClass;
		(*obj)["oldOffsetWidth"] = JsValue::CreateValue(event->oldSize().width());
		(*obj)["oldOffsetHeight"] = JsValue::CreateValue(event->oldSize().height());
		(*obj)["offsetWidth"] = JsValue::CreateValue(event->size().width());
		(*obj)["offsetHeight"] = JsValue::CreateValue(event->size().height());
		jsParser.Trigger(key, JsValue::CreateValue(obj), global);
	}
	void LinkBridge::TriggerJsEvent(JsParser& jsParser, const QString& key, QWheelEvent* event, bool global) {
		JsClass* obj = new JsClass;
		(*obj)["deltaX"] = JsValue::CreateValue(event->angleDelta().x());
		(*obj)["deltaY"] = JsValue::CreateValue(event->angleDelta().y());
		(*obj)["pixelX"] = JsValue::CreateValue(event->pixelDelta().x());
		(*obj)["pixelY"] = JsValue::CreateValue(event->pixelDelta().y());
		(*obj)["ctrlKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ControlModifier));
		jsParser.Trigger(key, JsValue::CreateValue(obj), global);
	}
	void LinkBridge::TriggerJsEvent(JsParser& jsParser, const QString& key, QKeyEvent* event, bool global) {
		JsClass* obj = new JsClass;
		(*obj)["key"] = JsValue::CreateValue(QKeySequence(event->key()).toString().toLower().toStdString());
		(*obj)["code"] = JsValue::CreateValue(QString("Key" + QKeySequence(event->key()).toString().toLower()).toStdString());
		(*obj)["ctrlKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ControlModifier));
		(*obj)["shiftKey"] = JsValue::CreateValue(bool(event->modifiers() & Qt::ShiftModifier));
		jsParser.Trigger(key, JsValue::CreateValue(obj), global);
	}
	void LinkBridge::TriggerJsEvent(JsParser& jsParser, const QString& key, QCloseEvent* event, bool global) {
		jsParser.Trigger(key, global);
	}
	void LinkBridge::TriggerJsEvent(JsParser& jsParser, const QString& key, QMouseEvent* event, bool global) {
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
		jsParser.Trigger(key, JsValue::CreateValue(obj),global);
	}
}
