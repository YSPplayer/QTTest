/*
	Created By YSP
	2025.8.1
*/
#pragma once
#include <QString>
#include <QList>
namespace ysp::qt::html {
	class StyleBuilder {
	public:
		StyleBuilder();
	private:
		QList<QString> styles;
	};
}