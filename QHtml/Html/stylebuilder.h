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
		StyleBuilder();
		StyleBuilder& SetBorderRadius(qint32 radius);
		StyleBuilder& SetBorderRadius(qint32 r1, qint32 r2, qint32 r3, qint32 r4);
		StyleBuilder& SetBackgroundColor(const QColor& color);
		StyleBuilder& SetBackgroundColor(const QString& name);
		StyleBuilder& SetStyle(const QString& prop, const QString& value);
		StyleBuilder& SetStyles(const QString& cssInline);
		StyleBuilder& SetBorder(qint32 width, const QString& style, const QString& color);
		StyleBuilder& SetBorderWidth(qint32 width);
		StyleBuilder& SetBorderStyle(const QString& style);
		StyleBuilder& SetBorderColor(const QString& color);
		StyleBuilder& SetBorderColor(const QColor& color);
		StyleBuilder& SetBorderTop(qint32 width, const QString& style, const QString& color);
		StyleBuilder& SetBorderRight(qint32 width, const QString& style, const QString& color);
		StyleBuilder& SetBorderBottom(qint32 width, const QString& style, const QString& color);
		StyleBuilder& SetBorderLeft(qint32 width, const QString& style, const QString& color);
		QString GetStyles();
		QString ToString();
	private:
		QString Build();
		QList<QString> styles;
		QString format;
	};
}