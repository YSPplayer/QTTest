/*
	Created By YSP
	2025.8.23
*/
#include "clabel.h"
namespace ysp::qt::html {
	CLabel::CLabel(QWidget* parent) :QLabel(parent) {
		setMouseTracking(true);
		setAutoFillBackground(true);
		setAttribute(Qt::WA_NoChildEventsForParent, true);
	}
}