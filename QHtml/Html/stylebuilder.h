/*
	Created By YSP
	2025.8.1
*/
#pragma once
#include <QWidget>
#include <QString>
#include <QList>
namespace ysp::qt::html {
	class StyleBuilder {
	public:
		StyleBuilder(QWidget* widget);
		StyleBuilder& SetBorderRadius(qint32 radius);
		StyleBuilder& SetBorderRadius(qint32 r1, qint32 r2, qint32 r3, qint32 r4);
		StyleBuilder& SetBackgroundColor(const QColor& color);
		StyleBuilder& SetBackgroundColor(const QString& name);
		QString ToString();
	private:
		QString Build();
		QList<QString> styles;
		QString format;
	};
}