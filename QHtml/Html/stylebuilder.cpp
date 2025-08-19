/*
	Created By YSP
	2025.8.1
*/
#include "stylebuilder.h"
namespace ysp::qt::html {
	StyleBuilder::StyleBuilder(QWidget* widget) {
		const QString& id = widget->objectName();
		format = widget->metaObject()->className();
		//ID作为最终标识符，这是QT的唯一id标识符 非js的id
		format += "#" + id;
		/*if (classValue != "") {
			format += ("." + classValue);
		}*/
	}
	StyleBuilder::StyleBuilder() {
		format = "";
	}
	StyleBuilder& StyleBuilder::SetBorderRadius(qint32 radius) {
		return SetStyle("border-radius", QString("%1px").arg(radius));
	}
	StyleBuilder& StyleBuilder::SetBorderRadius(qint32 r1, qint32 r2, qint32 r3, qint32 r4) {
		return SetStyle("border-radius",QString("%1px %2px %3px %4px").arg(r1).arg(r2).arg(r3)
			.arg(r4));
	}
	StyleBuilder& StyleBuilder::SetBackgroundColor(const QColor& color) {
		return SetStyle("background-color", color.name());
	}
	StyleBuilder& StyleBuilder::SetBackgroundColor(const QString& name) {
		return SetStyle("background-color", name);
	}
	StyleBuilder& StyleBuilder::SetStyle(const QString& prop, const QString& value) {
		const QString p = prop.trimmed().toLower();
		const QString v = value.trimmed();
		if (p.isEmpty()) return *this;
		// 查找是否已存在该属性，存在则替换
		for (qint32 i = 0; i < styles.size(); ++i) {
			const QString s = styles[i].trimmed();
			const qint32 colon = s.indexOf(':');
			if (colon > 0) {
				const QString existProp = s.left(colon).trimmed().toLower();
				if (existProp == p) {
					styles[i] = QString("%1: %2;").arg(prop).arg(v);
					return *this;
				}
			}
		}
		// 不存在则追加
		styles.append(QString("%1: %2;").arg(prop).arg(v));
		return *this;
	}

	StyleBuilder& StyleBuilder::SetStyles(const QString& cssInline) {
		const QStringList decls = cssInline.split(';', Qt::SkipEmptyParts);
		for (const QString& raw : decls) {
			const QString decl = raw.trimmed();
			if (decl.isEmpty()) continue;
			const qint32 colon = decl.indexOf(':');
			if (colon <= 0) continue; // 非法声明，跳过
			const QString prop = decl.left(colon).trimmed();
			const QString value = decl.mid(colon + 1).trimmed();
			SetStyle(prop, value);
		}
		return *this;
	}

	QString StyleBuilder::GetStyles() {
		return styles.join(" ");
	}

	QString StyleBuilder::ToString() {
		return Build();
	}
	QString StyleBuilder::Build() {
#ifdef _DEBUG
		qDebug() << QString("%1 { %2 }").arg(format).arg(styles.join(" "));
#endif // _DEBUG
		return QString("%1 { %2 }").arg(format).arg(styles.join(" "));
	}
}