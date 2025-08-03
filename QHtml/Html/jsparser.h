/*
	Created By YSP
	2025.8.3
*/
#pragma once
#include <duktape.h>
namespace ysp::qt::html {
#define JS_API
	class JSBinder {
	private:
		duk_context* ctx;

	public:
		JSBinder(duk_context* context) : ctx(context) {}
		~JSBinder() = default;
		void beginObject();
		void bindMethod(const char* name, duk_c_function func, int nargs);
		void bindSubObject(const char* name);
		void endSubObject();
		void setGlobal(const char* name);
	};
	class JsParser {
	public:
		JsParser();
		bool Init();
		~JsParser();
		bool RunJs(const char* script);
	private:
		duk_context* ctx;
		JSBinder* binder;
		void BindJsFunc();
		//func//
		JS_API static duk_ret_t ConsoleLog(duk_context* ctx);
	};
}