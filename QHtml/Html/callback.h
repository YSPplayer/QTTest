/*
    Created By YSP
    2025.8.11
*/
#pragma once
#include <thread>
#include <functional>
#include <mutex>;
#include <QMap>
namespace ysp::qt::html {
	using CallBackFunction = void(void**);
	enum CallBackType {
		Load,
	};
	struct CallBackFunc {
		std::function<CallBackFunction> func;
		bool runFlags;
	};
	class CallBack {
	public:
		CallBack() = default;
		~CallBack() = default;
		bool Subscribe(quint32 type, const std::function<CallBackFunction>& clientFunction);
		bool SubscribeAsync(quint32 type, const std::function<CallBackFunction>& clientFunction);
		bool Unsubscribe(quint32 type);
		void RunFuncAsync(const std::function<CallBackFunction>& func, void** args);
		void Publish(quint32 type, void** args = nullptr);
		bool Contains(CallBackType type);
	private:
		std::mutex callBackMutex;
		QMap<quint32, CallBackFunc> callBackFuncs;
	};
}
