/*
	Created By YSP
	2025.8.1
*/
#include "triggerfilter.h"
namespace ysp::qt::html {

	TriggerFilter::TriggerFilter(CBase* base, QWidget* parent)
		: QObject(parent), base(base), widget(parent) {
	}

	bool TriggerFilter::eventFilter(QObject* watched, QEvent* event) {
		if (!qobject_cast<QWidget*>(watched)) {
			return QObject::eventFilter(watched, event);
		}
		switch (event->type()) {
		case QEvent::MouseButtonPress:
			return handleMousePressEvent(watched, static_cast<QMouseEvent*>(event));

		case QEvent::MouseMove:
			return handleMouseMoveEvent(watched, static_cast<QMouseEvent*>(event));

		case QEvent::MouseButtonRelease:
			return handleMouseReleaseEvent(watched, static_cast<QMouseEvent*>(event));

		case QEvent::Show:
			return handleShowEvent(watched, static_cast<QShowEvent*>(event));

		case QEvent::Resize:
			return handleResizeEvent(watched, static_cast<QResizeEvent*>(event));

		case QEvent::Wheel:
			return handleWheelEvent(watched, static_cast<QWheelEvent*>(event));

		case QEvent::KeyPress:
			return handleKeyPressEvent(watched, static_cast<QKeyEvent*>(event));

		case QEvent::KeyRelease:
			return handleKeyReleaseEvent(watched, static_cast<QKeyEvent*>(event));

		case QEvent::Close:
			return handleCloseEvent(watched, static_cast<QCloseEvent*>(event));

		case QEvent::Enter:
			return handleEnterEvent(watched, static_cast<QEnterEvent*>(event));

		case QEvent::Leave:
			return handleLeaveEvent(watched, event);

		case QEvent::MouseButtonDblClick:
			return handleMouseDoubleClickEvent(watched, static_cast<QMouseEvent*>(event));

		default:
			return QObject::eventFilter(watched, event);
		}
	}

	bool TriggerFilter::handleMousePressEvent(QObject* watched, QMouseEvent* event)
	{
		auto button = event->button();
		if (button == Qt::LeftButton) {
			isPressedLeft = true;
		}
		TriggerJsEvent("mousedown", event);
		return false;
	}

	bool TriggerFilter::handleMouseMoveEvent(QObject* watched, QMouseEvent* event) {
		TriggerJsEvent("mousemove", event);
		return false;
	}

	bool TriggerFilter::handleMouseReleaseEvent(QObject* watched, QMouseEvent* event) {
		auto button = event->button();
		if (button == Qt::LeftButton) {
			isPressedLeft = false;
			TriggerJsEvent("click");
		}
		TriggerJsEvent("mouseup", event);
		return false;
	}

	bool TriggerFilter::handleShowEvent(QObject* watched, QShowEvent* event) {
		if (!base->firstShow) {
			base->firstShow = true;
			if (base->global) {
				TriggerGlobalEvent("load");
			}
		}
		return false;
	}

	bool TriggerFilter::handleResizeEvent(QObject* watched, QResizeEvent* event) {
		TriggerJsEvent("resize", event);
		return false;
	}

	bool TriggerFilter::handleWheelEvent(QObject* watched, QWheelEvent* event) {
		TriggerJsEvent("wheel", event);
		return false;
	}

	bool TriggerFilter::handleKeyPressEvent(QObject* watched, QKeyEvent* event) {
		TriggerJsEvent("keydown", event);
		return false;
	}

	bool TriggerFilter::handleKeyReleaseEvent(QObject* watched, QKeyEvent* event) {
		TriggerJsEvent("keyup", event);
		return false;
	}

	bool TriggerFilter::handleCloseEvent(QObject* watched, QCloseEvent* event) {
		TriggerJsEvent("close", event);
		return false;
	}

	bool TriggerFilter::handleEnterEvent(QObject* watched, QEnterEvent* event) {
		TriggerJsEvent("mouseenter", event);
		return false;
	}

	bool TriggerFilter::handleLeaveEvent(QObject* watched, QEvent* event) {
		TriggerJsEvent("mouseleave", event);
		return false;
	}

	bool TriggerFilter::handleMouseDoubleClickEvent(QObject* watched, QMouseEvent* event) {
		TriggerJsEvent("dblclick");
		return false;
	}
}