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
#define JS_TYPE_OBJECT 7
#define K_PTRKEY "_K_WIDGET_PTRKEY"
#define JSPARSER "_JSPARSER_JSVALUE"
#define CXX_CUT_JS_CONST_VALUE "CXX_CUT_JS_CONST_VALUE"
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
		static std::shared_ptr<JsValue> CreateValue(std::string value);
		static std::shared_ptr<JsValue> CreateObjectValue(std::string value);
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
		void Trigger(const QString& callbackType, const std::vector<std::shared_ptr<JsValue>>& params, bool global = false);
		void Trigger(const QString& callbackType, const std::shared_ptr<JsValue>& param, bool global = false);
		void Trigger(const QString& callbackType, bool global = false);
		void CreateDocument(QWidget* widget);
	private:
		bool init;
		QList<QWidget*> objects;
		duk_context* ctx;
		JSBinder* binder;
		void LoadJsLibrary();
		void BindJsFunc();
		void PushJsValue(const std::shared_ptr<JsValue>& value);
		void PushJsObject(const JsClass* obj);
		void PushJsArray(const JsArray* arr);
		static void PushJsValue(duk_context* ctx, const std::shared_ptr<JsValue>& value);
		static void PushJsObject(duk_context* ctx, const JsClass* obj);
		static void PushJsArray(duk_context* ctx, const JsArray* arr);
		//func//
		static duk_ret_t ThrowError(duk_context* ctx, duk_ret_t code, const QString& error);
		static QWidget* ThisWidget(duk_context* ctx);
		static duk_ret_t GetValue(duk_context* ctx, const char* name);
		static duk_ret_t GetOption(duk_context* ctx, const char* name);
		static duk_ret_t SetOption(duk_context* ctx, const char* name);
		static duk_ret_t SetValue(duk_context* ctx, const char* name);
		static void ExpandArray(duk_context* ctx, qint32 position, qint32 length);
		JS_API static duk_ret_t ClearEvent(duk_context* ctx);
		JS_API static duk_ret_t ArrayForEach(duk_context* ctx);
		JS_API static duk_ret_t AddOption(duk_context* ctx);
		JS_API static duk_ret_t Delete(duk_context* ctx);
		JS_API static duk_ret_t ConsoleLog(duk_context* ctx);
		JS_API static duk_ret_t WindowAddEventListener(duk_context* ctx);
		JS_API static duk_ret_t ObjectAddEventListener(duk_context* ctx);
		JS_API static duk_ret_t DocumentGetElementById(duk_context* ctx);
		JS_API static duk_ret_t DocumentQuerySelector(duk_context* ctx);
		JS_API static duk_ret_t DocumentQuerySelectorAll(duk_context* ctx);
		JS_API static duk_ret_t DocumentGetElementByKey(duk_context* ctx);
		JS_API static duk_ret_t Append(duk_context* ctx);
		JS_API static duk_ret_t QuerySelector(duk_context* ctx);
		JS_API static duk_ret_t QuerySelectorAll(duk_context* ctx);
		JS_API static duk_ret_t Remove(duk_context* ctx);
		JS_API static duk_ret_t MapTo(duk_context* ctx);
		JS_API static duk_ret_t CreateElement(duk_context* ctx);
		JS_API static duk_ret_t SetStyleSheet(duk_context* ctx);
		JS_API static duk_ret_t CreateOption(duk_context* ctx);
		//func//
	};
}