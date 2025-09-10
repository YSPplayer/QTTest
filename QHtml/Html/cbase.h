/*
	Created By YSP
	2025.9.10
*/
#pragma once
#include <QWidget>
namespace ysp::qt::html {
	class CBase {
	public:
		bool global;
		bool firstShow;
		CBase();
		~CBase() = default;
		static QString GetId(QWidget* widget);
		static QString GetJsId(QWidget* widget);
		static QString GetClass(QWidget* widget);
		static QString GetClassName(QWidget* widget);
		static QString GetKeyString(QWidget* widget);
		static bool IsGlobal(QWidget* widget);
		static QString TriggerId(QWidget* widget, const QString& key, bool global);

	};
}