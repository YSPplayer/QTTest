/*
	Created By YSP
	2025.8.1
*/
#include "stylebuilder.h"
namespace ysp::qt::html {
	StyleBuilder::StyleBuilder(QWidget* widget) {
		const QString& id = widget->objectName();
		const QString& classValue = widget->property("class").isValid() ?
			widget->property("class").toString() : "";
		format = widget->metaObject()->className();
		if (id != "") {
			format += ("#" + id);
		}
		if (classValue != "") {
			format += ("." + classValue);
		}
	}
	StyleBuilder& StyleBuilder::SetBorderRadius(qint32 radius) {
		styles.append(QString("border-radius: %1px;").arg(radius));
		return *this;
	}
	StyleBuilder& StyleBuilder::SetBorderRadius(qint32 r1, qint32 r2, qint32 r3, qint32 r4) {
		styles.append(QString("border-radius: %1px %2px %3px %14px;").arg(r1).arg(r2).arg(r3)
			.arg(r4));
		return *this;
	}
	StyleBuilder& StyleBuilder::SetBackgroundColor(const QColor& color) {
		styles.append(QString("background-color: %1;").arg(color.name()));
		return *this;
	}
	StyleBuilder& StyleBuilder::SetBackgroundColor(const QString& name) {
		styles.append(QString("background-color: %1;").arg(name));
		return *this;
	}
	QString StyleBuilder::ToString() {
		return Build();
	}
	QString StyleBuilder::Build() {
		return QString("%1 { %2 }").arg(format).arg(styles.join(" "));
	}
}