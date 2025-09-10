/*
	Created By YSP
	2025.9.10
*/
#include "cbase.h"
namespace ysp::qt::html {
	CBase::CBase() {
		global = false;
		firstShow = false;
	}

	QString CBase::GetId(QWidget* widget) {
		return widget->objectName();
	}

	QString CBase::GetJsId(QWidget* widget) {
		return  widget->property("jsid").isValid() ?
			widget->property("jsid").toString() : "";
	}

	QString CBase::GetClass(QWidget* widget) {
		return  widget->property("class").isValid() ?
			widget->property("class").toString() : "";
	}
	QString CBase::GetClassName(QWidget* widget) {
		return widget->metaObject()->className();
	}
	QString CBase::GetKeyString(QWidget* widget) {
		return QString("%1%2%3%4").arg(GetClassName(widget)).arg(GetId(widget)).arg(GetClass(widget)).arg(QString::number(reinterpret_cast<quintptr>(widget), 16));
	}
	bool CBase::IsGlobal(QWidget* widget) {
		return false;
	}
	QString CBase::TriggerId(QWidget* widget, const QString& key, bool global) {
		return global ? key : QString("%1%2").arg(GetKeyString(widget)).arg(key);
	}
}