/*
	Created By YSP
	2025.8.3
*/
#pragma once
#include <QString>
#include <QWidget>
#include <QList>
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <map>
#include <duktape.h>
namespace ysp::qt::html {
#define JS_API
#define JS_TYPE_STRING 0
#define JS_TYPE_INT 1
#define JS_TYPE_DOUBLE 2
#define JS_TYPE_BOOL 3
#define JS_TYPE_CLASS 4
#define JS_TYPE_ARRAY 5
#define JS_TYPE_UNDEFINED 6
#define K_PTRKEY "_K_WIDGET_PTRKEY"
#define JSPARSER "_JSPARSER_JSVALUE"
#define DUK_GETTER(prop) \
    [](duk_context* ctx)->duk_ret_t { \
        return GetValue(ctx, prop); \
    }
#define DUK_SETTER(prop) \
    [](duk_context* ctx)->duk_ret_t { \
        return SetValue(ctx, prop); \
    }
	class JsValue;
	using JsClass = std::map<QString, std::shared_ptr<JsValue>>;
	using JsArray = std::vector<std::shared_ptr<JsValue>>;
	class JsValue {
	public:
		void* value;
		int type;
		JsValue(void* value, int type);
		~JsValue();
		static std::shared_ptr<JsValue> CreateValue(qint32 value);
		static std::shared_ptr<JsValue> CreateValue(double value);
		static std::shared_ptr<JsValue> CreateValue(bool value);
		static std::shared_ptr<JsValue> CreateValue(JsClass* value);
		static std::shared_ptr<JsValue> CreateValue(JsArray* value);
	};

	class JSBinder {
	private:
		duk_context* ctx;
	public:
		JSBinder(duk_context* context) : ctx(context) {}
		~JSBinder() = default;
		void beginObject();
		void bindMethod(const char* name, duk_c_function func, int args);
		void bindAttributeMethod(const char* name, duk_c_function get, duk_c_function set);
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
		void Trigger(const QString& callbackType, const std::vector<std::shared_ptr<JsValue>>& params);
		void Trigger(const QString& callbackType, const std::shared_ptr<JsValue>& param);
		void Trigger(const QString& callbackType);
		void CreateDocument(QWidget* widget);
	private:
		QList<QWidget*> objects;
		duk_context* ctx;
		JSBinder* binder;
		static void CreateDocument(QWidget* widget,JsParser* js);
		void BindJsFunc();
		void PushJsValue(const std::shared_ptr<JsValue>& value);
		void PushJsObject(const JsClass* obj);
		void PushJsArray(const JsArray* arr); 
		//func//
		static QWidget* ThisWidget(duk_context* ctx);
		static duk_ret_t GetValue(duk_context* ctx, const char* name);
		static duk_ret_t SetValue(duk_context* ctx, const char* name);
		JS_API static duk_ret_t ConsoleLog(duk_context* ctx);
		JS_API static duk_ret_t WindowAddEventListener(duk_context* ctx);
		JS_API static duk_ret_t ObjectAddEventListener(duk_context* ctx);
		JS_API static duk_ret_t DocumentGetElementById(duk_context* ctx);
		JS_API static duk_ret_t CreateElement(duk_context* ctx);
		//func//
	};
}