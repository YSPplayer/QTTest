/*
	Created By YSP
	2025.8.28
*/
#include "cprogressbar.h"
namespace ysp::qt::html {
	CProgressBar::CProgressBar(QWidget* parent) :QProgressBar(parent) {
		setMouseTracking(true);
		setAutoFillBackground(true);
		setAttribute(Qt::WA_NoChildEventsForParent, true);
	}
}