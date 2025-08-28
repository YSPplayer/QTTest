/*
	Created By YSP
	2025.8.23
*/
#pragma once
#include <QLabel>
namespace ysp::qt::html {
	class CLabel :public QLabel {
	public:
		explicit CLabel(QWidget* parent = nullptr);
		~CLabel() = default;
	};
}