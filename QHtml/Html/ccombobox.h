/*
	Created By YSP
	2025.9.2
*/
#pragma once
#include <QComboBox>
namespace ysp::qt::html {
	class CComboBox :public QComboBox {
	public:
		explicit CComboBox(QWidget* parent = nullptr);
		~CComboBox() = default;
	};
}
