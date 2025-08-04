/*
	Created By YSP
	2025.8.3
*/
#pragma once
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <map>
#include <duktape.h>
namespace ysp::qt::html {
#define JS_API
	//using JsValue = std::variant<
	//	std::string,           // 字符串
	//	int,                   // 整数
	//	double,                // 浮点数
	//	bool,                  // 布尔值
	//	std::map<std::string, std::shared_ptr<JsValue>>,  // 对象
	//	std::vector<std::shared_ptr<JsValue>>             // 数组
	//>;
#define JS_TYPE_STRING 0
#define JS_TYPE_INT 1
#define JS_TYPE_DOUBLE 2
#define JS_TYPE_BOOL 3
#define JS_TYPE_CLASS 4
#define JS_TYPE_ARRAY 5
#define JS_TYPE_UNDEFINED 6
	class JsValue;
	using JsClass = std::map<std::string, std::shared_ptr<JsValue>>;
	using JsArray = std::vector<std::shared_ptr<JsValue>>;
	class JsValue {
	public:
		void* value;
		int type;
		JsValue(void* value, int type);
		~JsValue();
	};

	class JSBinder {
	private:
		duk_context* ctx;

	public:
		JSBinder(duk_context* context) : ctx(context) {}
		~JSBinder() = default;
		void beginObject();
		void bindMethod(const char* name, duk_c_function func, int args);
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
		void Trigger(const std::string& callbackType, const std::vector<JsValue>& params);
		void Trigger(const std::string& callbackType, const JsValue& param);
		void Trigger(const std::string& callbackType);
	private:
		duk_context* ctx;
		JSBinder* binder;
		void BindJsFunc();
		void PushJsValue(const JsValue& value);
		void PushJsObject(const JsClass& obj);
		void PushJsArray(const JsArray& arr);
		//func//
		JS_API static duk_ret_t ConsoleLog(duk_context* ctx);
		JS_API static duk_ret_t WindowAddEventListener(duk_context* ctx);
		JS_API static duk_ret_t DocumentGetElementById(duk_context* ctx);
	};
}