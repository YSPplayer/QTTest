/*
	Created By YSP
	2025.9.9
*/
#pragma once
#include <QPushButton>
namespace ysp::qt::html {
	class CPushButton : public QPushButton {
	public:
		explicit CPushButton(QWidget* parent = nullptr);
		~CPushButton() = default;
	};
}
