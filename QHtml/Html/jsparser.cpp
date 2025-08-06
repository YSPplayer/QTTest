/*
	Created By YSP
	2025.8.3
*/
#include "jsparser.h"
#include "linkbridge.h"
#include "listfilter.h"
namespace ysp::qt::html {
	/*
	栈顶索引 -1 栈底索引0
	入栈是往栈顶添加
	*/
	QList<QWidget*> JsParser::objects;
	JsParser::JsParser() {
		ctx = nullptr;
		binder = nullptr;
	}
	bool JsParser::Init() {
		ctx = duk_create_heap_default();
		bool success = ctx != nullptr;
		if (success) {
			binder = new JSBinder(ctx);
			BindJsFunc();
		}
		return success;
	}
	JsParser::~JsParser() {
		if (ctx) {
			duk_destroy_heap(ctx);
			ctx = nullptr;
		} 
		if (binder) {
			delete binder;
			binder = nullptr;
		}
	}
	/// <summary>
	/// 运行Js
	/// </summary>
	bool JsParser::RunJs(const char* script) {
		if (!ctx || !script) return false;
		bool success = false;
		duk_idx_t top = duk_get_top(ctx);
		try {
			if (duk_peval_string(ctx, script) != 0) {
				// 运行时错误
				const char* error = duk_safe_to_string(ctx, -1);
				LinkBridge::Print(error);
				success = false;
			}
			else {
				success = true;
			}
		}
		catch (...) {
			success = false;
		}
		duk_set_top(ctx, top);
		return success;
	}
	void JsParser::BindJsFunc() {
		binder->beginObject();
		binder->bindMethod("log", ConsoleLog, DUK_VARARGS);
		binder->setGlobal("console");

		binder->beginObject();
		binder->bindMethod("addEventListener", WindowAddEventListener, 2);
		binder->setGlobal("window");

		binder->beginObject();
		binder->bindMethod("getElementById", DocumentGetElementById, 1);
		binder->setGlobal("document");
	}
	void JsParser::PushJsValue(const std::shared_ptr<JsValue>& value) {
		void* v = value->value;
		switch (value->type) {
		case JS_TYPE_STRING: { // std::string
			const std::string& str = *(std::string*)v;
			duk_push_string(ctx, str.c_str());
			break;
		}
		case JS_TYPE_INT: { // int
			duk_push_int(ctx, *(int*)v);
			break;
		}
		case JS_TYPE_DOUBLE: { // double
			duk_push_number(ctx, *(double*)v);
			break;
		}
		case JS_TYPE_BOOL: { // bool
			duk_push_boolean(ctx, *(bool*)v);
			break;
		}
		case JS_TYPE_CLASS: { // std::map<std::string, std::shared_ptr<JsValue>>
			PushJsObject((JsClass*)v);
			break;
		}
		case JS_TYPE_ARRAY: { // std::vector<std::shared_ptr<JsValue>>
			PushJsArray((JsArray*)v);
			break;
		}
		default:
			duk_push_undefined(ctx);
			break;
		}
	}
	void JsParser::PushJsObject(const JsClass* obj) {
		duk_push_object(ctx);
		for (auto it = obj->begin(); it != obj->end(); ++it) {
			const QString& key = it->first;
			const std::shared_ptr<JsValue>& value = it->second;
			PushJsValue(value);
			duk_put_prop_string(ctx, -2, key.toUtf8().constData());
		}
	}
	void JsParser::PushJsArray(const JsArray* arr) {
		duk_push_array(ctx);
		for (size_t i = 0; i < arr->size(); ++i) {
			PushJsValue(arr->at(i));
			duk_put_prop_index(ctx, -2, i);
		}
	}

	void JsParser::Trigger(const QString& callbackType, const std::vector<std::shared_ptr<JsValue>>& params) {
		if (!ctx) {
			return;
		}
		duk_get_global_string(ctx, callbackType.toUtf8().constData());
		if (duk_is_function(ctx, -1)) {
			// 将所有参数推入栈
			for (const auto& param : params) {
				PushJsValue(param);
			}
			// 调用回调函数
			if (duk_pcall(ctx, params.size()) != 0) {
				LinkBridge::Print(duk_safe_to_string(ctx, -1));
			}
			duk_pop(ctx);
		}
		else {
			//不存在数据，不用管
			duk_pop(ctx);
		}
	}
	void JsParser::Trigger(const QString& callbackType, const std::shared_ptr<JsValue>& param) {
		const std::vector<std::shared_ptr<JsValue>>& params = { param };
		Trigger(callbackType, params);
	}
	void JsParser::Trigger(const QString& callbackType) {
		const std::vector<std::shared_ptr<JsValue>> params;
		Trigger(callbackType, params);
	}

	/// <summary>
	/// 创建对象
	/// </summary>
	void JsParser::CreateDocument(QWidget* widget) {
		if (!widget || widget->objectName() == "" || objects.contains(widget)) return;
		objects.append(widget);
		duk_push_object(ctx);
		const QString& id = widget->objectName();
		duk_push_string(ctx, id.toUtf8().constData());
		duk_put_prop_string(ctx, -2, "id"); 
		duk_push_int(ctx, widget->width());
		duk_put_prop_string(ctx, -2, "width");
		duk_push_int(ctx, widget->height());
		duk_put_prop_string(ctx, -2, "height");
		binder->bindMethod("addEventListener", ObjectAddEventListener, 2);
		QString globalKey = QString("element_%1").arg(id.toUtf8().constData());
		duk_put_global_string(ctx, globalKey.toUtf8().constData()); 
	}
	JS_API duk_ret_t JsParser::ConsoleLog(duk_context* ctx) {
		duk_push_string(ctx, " ");//往栈顶压入分隔符
		duk_insert(ctx, 0); //把栈顶的元素插入到栈底
		duk_join(ctx, duk_get_top(ctx) - 1);//连接所有的分隔符
		LinkBridge::Print(duk_safe_to_string(ctx, -1));//转为字符串
		return 0;
	}
	JS_API duk_ret_t JsParser::WindowAddEventListener(duk_context* ctx) {
		if(duk_get_top(ctx) < 2) return DUK_RET_TYPE_ERROR;
		if (!duk_is_string(ctx, 0))  return DUK_RET_TYPE_ERROR;
		const char* callbackType = duk_require_string(ctx, 0);
		if (!duk_is_function(ctx, 1)) return DUK_RET_TYPE_ERROR;
		duk_put_global_string(ctx, callbackType);
		return 0;
	}
	JS_API duk_ret_t JsParser::ObjectAddEventListener(duk_context* ctx) {
		if (duk_get_top(ctx) < 2) return DUK_RET_TYPE_ERROR;
		if (!duk_is_string(ctx, 0))  return DUK_RET_TYPE_ERROR;
		const char* callbackType = duk_require_string(ctx, 0);
		if (!duk_is_function(ctx, 1)) return DUK_RET_TYPE_ERROR;
		duk_push_this(ctx);
		duk_get_prop_string(ctx, -1, "id");
		const char* id = duk_require_string(ctx, -1);
		const QString& key = QString::fromUtf8(id) + QString::fromUtf8(callbackType);
		duk_dup(ctx, 1);
		duk_put_global_string(ctx, key.toUtf8().constData());
		duk_pop(ctx);
		return 0;
	}
	JS_API duk_ret_t JsParser::DocumentGetElementById(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return DUK_RET_TYPE_ERROR;
		if (!duk_is_string(ctx, 0)) return DUK_RET_TYPE_ERROR;
		const char* elementId = duk_require_string(ctx, 0);
		const QString& id = QString::fromUtf8(elementId);
		const QString& globalKey = QString("element_%1").arg(id);
		duk_get_global_string(ctx, globalKey.toUtf8().constData());
		if (!duk_is_undefined(ctx, -1)) {
			return 1;
		}
		else {
			duk_pop(ctx);
			duk_push_null(ctx);
			return 1;  
		}
	}
	void JSBinder::beginObject() {
		duk_push_object(ctx);
	}

	void JSBinder::bindMethod(const char* name, duk_c_function func, int args) {
		duk_push_c_function(ctx, func, args);
		duk_put_prop_string(ctx, -2, name);
	}

	void JSBinder::bindSubObject(const char* name) {
		duk_push_object(ctx);
		duk_put_prop_string(ctx, -2, name);
		duk_push_object(ctx); 
	}

	void JSBinder::endSubObject() {
		duk_pop(ctx);
	}

	void JSBinder::setGlobal(const char* name) {
		duk_put_global_string(ctx, name);
	}

	JsValue::JsValue(void* value, int type) {
		this->value = value;
		this->type = type;
	}

	JsValue::~JsValue() {
		switch (type) {
		case JS_TYPE_CLASS: delete static_cast<JsClass*>(value); break;
		case JS_TYPE_ARRAY: delete static_cast<JsArray*>(value); break;
		case JS_TYPE_STRING: delete static_cast<std::string*>(value); break;
		case JS_TYPE_INT: delete static_cast<int*>(value); break;
		case JS_TYPE_DOUBLE: delete static_cast<double*>(value); break;
		case JS_TYPE_BOOL: delete static_cast<bool*>(value); break;
		default: break;
		}
		value = nullptr;
	}

	std::shared_ptr<JsValue> JsValue::CreateValue(qint32 value) {
		return std::make_shared<JsValue>(new qint32(value), JS_TYPE_INT);
	}

	std::shared_ptr<JsValue> JsValue::CreateValue(double value) {
		return std::make_shared<JsValue>(new double(value), JS_TYPE_DOUBLE);
	}

	std::shared_ptr<JsValue> JsValue::CreateValue(bool value) {
		return std::make_shared<JsValue>(new bool(value), JS_TYPE_BOOL);
	}

	std::shared_ptr<JsValue> JsValue::CreateValue(JsClass* value) {
		return std::make_shared<JsValue>(value, JS_TYPE_CLASS);
	}

	std::shared_ptr<JsValue> JsValue::CreateValue(JsArray* value) {
		return std::make_shared<JsValue>(value, JS_TYPE_ARRAY);
	}

}
