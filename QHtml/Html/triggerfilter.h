/*
	Created By YSP
	2025.8.1
*/
#pragma once
#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QShowEvent>
#include <QCloseEvent>
#include <QEnterEvent>
#include <QEvent>
#include "include.h"
#include "linkbridge.h"
namespace ysp::qt::html {
	class TriggerFilter : public QObject {
	public:
		explicit TriggerFilter(CBase* base, QWidget* parent);
	protected:
		// 事件过滤器核心函数
		bool eventFilter(QObject* watched, QEvent* event) override;

	private:
		// 各种事件处理函数
		bool handleMousePressEvent(QObject* watched, QMouseEvent* event);
		bool handleMouseMoveEvent(QObject* watched, QMouseEvent* event);
		bool handleMouseReleaseEvent(QObject* watched, QMouseEvent* event);
		bool handleShowEvent(QObject* watched, QShowEvent* event);
		bool handleResizeEvent(QObject* watched, QResizeEvent* event);
		bool handleWheelEvent(QObject* watched, QWheelEvent* event);
		bool handleKeyPressEvent(QObject* watched, QKeyEvent* event);
		bool handleKeyReleaseEvent(QObject* watched, QKeyEvent* event);
		bool handleCloseEvent(QObject* watched, QCloseEvent* event);
		bool handleEnterEvent(QObject* watched, QEnterEvent* event);
		bool handleLeaveEvent(QObject* watched, QEvent* event);
		bool handleMouseDoubleClickEvent(QObject* watched, QMouseEvent* event);

		CBase* base;  // 持有CWidget对象的引用
		QWidget* widget;
		bool isPressedLeft = false;
		template<typename T>
		void TriggerJsEvent(const QString& key, T* event) {
			if (widget) {
				LinkBridge::TriggerJsEvent(
					CBase::GetKeyString(widget),
					CBase::TriggerId(widget, key, base->global),
					event,
					base->global
				);
			}
		}

		void TriggerJsEvent(const QString& key) {
			if (widget) {
				LinkBridge::TriggerJsEvent(
					CBase::GetKeyString(widget),
					CBase::TriggerId(widget, key, base->global),
					base->global);
			}
		}

		void TriggerGlobalEvent(const QString& key) {
			LinkBridge::TriggerJsEvent(key, true);
		}
	};
}

