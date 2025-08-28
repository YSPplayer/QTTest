/*
	Created By YSP
	2025.8.28
*/
#pragma once
#include <QProgressBar>
namespace ysp::qt::html {
	class CProgressBar : public QProgressBar {
	public:
		explicit CProgressBar(QWidget* parent = nullptr);
		~CProgressBar() = default;
	};
}
