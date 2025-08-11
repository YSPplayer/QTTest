/*
	Created By YSP
	2025.8.11
*/
#include "callback.h"
namespace ysp::qt::html {
	bool CallBack::Subscribe(quint32 type, const std::function<CallBackFunction>& clientFunction) {
		std::lock_guard<std::mutex> lock(callBackMutex);
		CallBackFunc callBackFunc;
		callBackFunc.func = clientFunction;
		callBackFunc.runFlags = false;
		callBackFuncs[type] = callBackFunc;
		return true;
	}


	bool CallBack::SubscribeAsync(quint32 type, const std::function<CallBackFunction>& clientFunction) {
		std::lock_guard<std::mutex> lock(callBackMutex);
		CallBackFunc callBackFunc;
		callBackFunc.func = clientFunction;
		callBackFunc.runFlags = true;
		callBackFuncs[type] = callBackFunc;
		return true;
	}

	bool CallBack::Unsubscribe(quint32 type) {
		std::lock_guard<std::mutex> lock(callBackMutex);
		if (!callBackFuncs.contains(type)) return false;
		callBackFuncs[type].func = nullptr;
		callBackFuncs[type].runFlags = false;
		return true;
	}

	void CallBack::RunFuncAsync(const std::function<CallBackFunction>& func, void** args) {
		std::thread([func, args]() {
			func(args);
			}).detach();
		return;
	}

	void CallBack::Publish(quint32 type, void** args) {
		std::function<CallBackFunction> func;
		bool runAsync = false;
		{
			std::lock_guard<std::mutex> lock(callBackMutex);
			if (!callBackFuncs.contains(type)) return;
			func = callBackFuncs[type].func;
			if (!func) return;
			runAsync = callBackFuncs[type].runFlags;
		}
		if (func) {
			if (runAsync) {
				RunFuncAsync(func, args);
			}
			else {
				func(args);
			}
		}
	}

	bool CallBack::Contains(CallBackType type) {
		bool contains = false;
		{
			std::lock_guard<std::mutex> lock(callBackMutex);
			contains = callBackFuncs.contains(type) && callBackFuncs[type].func != nullptr;
		}
		return contains;
	}
}
