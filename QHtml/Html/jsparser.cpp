/*
	Created By YSP
	2025.8.3
*/
#include "jsparser.h"
#include "linkbridge.h"
#include "listfilter.h"
#include "cwidget.h"
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
		//压入指针
		duk_push_pointer(ctx, widget);
		duk_put_prop_string(ctx, -2, K_PTRKEY);//参数二是绑定的对象，会弹出(消耗)栈顶的值
		binder->bindAttributeMethod("id", DUK_GETTER("id"), nullptr);
		binder->bindAttributeMethod("width", DUK_GETTER("width"), DUK_SETTER("width"));
		binder->bindAttributeMethod("height", DUK_GETTER("height"), DUK_SETTER("height"));
		binder->bindAttributeMethod("top", DUK_GETTER("top"), DUK_SETTER("top"));
		binder->bindAttributeMethod("bottom", DUK_GETTER("bottom"), DUK_SETTER("bottom"));
		binder->bindAttributeMethod("left", DUK_GETTER("left"), DUK_SETTER("left"));
		binder->bindAttributeMethod("right", DUK_GETTER("right"), DUK_SETTER("right"));
		binder->bindAttributeMethod("style", DUK_GETTER("style"), DUK_SETTER("style"));
		binder->bindMethod("addEventListener", ObjectAddEventListener, 2);
		const QString& id = widget->objectName();
		QString globalKey = QString("element_%1").arg(id.toUtf8().constData());
		duk_put_global_string(ctx, globalKey.toUtf8().constData()); 
	}
	QWidget* JsParser::ThisWidget(duk_context* ctx) {
		duk_push_this(ctx);
		if (!duk_get_prop_string(ctx, -1, K_PTRKEY)) { //不存在这个键 返回
			duk_pop_2(ctx);
			return nullptr;
		}
		QWidget* w = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
		duk_pop_2(ctx);
		return w;
	}
	JS_API duk_ret_t JsParser::GetValue(duk_context* ctx, const char* name) {
		if (auto* w = ThisWidget(ctx)) {
			QWidget* parent = w->parentWidget() ? w->parentWidget() : nullptr;
			QString key(name);
			if (key == "id") duk_push_string(ctx, w->objectName().toUtf8().constData());
			else if (key == "width") duk_push_int(ctx, w->width());
			else if (key == "height") duk_push_int(ctx, w->height());
			else if (key == "top") duk_push_int(ctx, w->y());
			else if (key == "bottom") {
				parent ? duk_push_int(ctx, parent->height() - w->y() - w->height()) : duk_push_int(ctx, -1);
			}
			else if (key == "left") duk_push_int(ctx, w->x());
			else if (key == "right") {
				parent ? duk_push_int(ctx, parent->width() - w->x() - w->width()) : duk_push_int(ctx, -1);
			}
			else if (key == "style") {
				duk_push_string(ctx, CWidget::styleBuilder.contains(w) ?
					CWidget::styleBuilder[w].GetStyles().toUtf8().constData() : "");
			}
			return 1;//返回值表示弹出
		}
		duk_push_undefined(ctx); 
		return 1;
	}
	duk_ret_t JsParser::SetValue(duk_context* ctx, const char* name) {
		QWidget* w = ThisWidget(ctx);
		if (!w) return 0;

		QWidget* parent = w->parentWidget() ? w->parentWidget() : nullptr;
		const QString key(name);

		if (key == "width") {
			const qint32 v = duk_require_int(ctx, 0);
			w->resize(v, w->height());
		}
		else if (key == "height") {
			const qint32 v = duk_require_int(ctx, 0);
			w->resize(w->width(), v);
		}
		else if (key == "left") {
			const qint32 v = duk_require_int(ctx, 0);
			w->move(v, w->y());
		}
		else if (key == "top") {
			const qint32 v = duk_require_int(ctx, 0);
			w->move(w->x(), v);
		}
		else if (key == "right") {
			const qint32 v = duk_require_int(ctx, 0);
			if (parent) w->move(parent->width() - v - w->width(), w->y());
		}
		else if (key == "bottom") {
			const qint32 v = duk_require_int(ctx, 0);
			if (parent) w->move(w->x(), parent->height() - v - w->height());
		}
		else if (key == "style") {
			if (CWidget::styleBuilder.contains(w)) {
				const char* v = duk_require_string(ctx, 0);
				CWidget::styleBuilder[w].SetStyles(QString::fromUtf8(v));
				w->setStyleSheet(CWidget::styleBuilder[w].ToString());
			}
		}
		return 0;
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

	void JSBinder::bindAttributeMethod(const char* name, duk_c_function get, duk_c_function set) {
		duk_uint_t flags = DUK_DEFPROP_ENUMERABLE | DUK_DEFPROP_CONFIGURABLE;
		duk_push_string(ctx, name); // key
		if (get) {
			duk_push_c_function(ctx, get, 0);
			flags |= DUK_DEFPROP_HAVE_GETTER;
			if (set) {
				duk_push_c_function(ctx, set, 1); // setter(value)
				flags |= DUK_DEFPROP_HAVE_SETTER;
				duk_def_prop(ctx, -4, flags);     // obj在-4
			}
			else {
				duk_def_prop(ctx, -3, flags);     // obj在-3
			}
		}
		else if (set) {
			duk_push_c_function(ctx, set, 1);
			flags |= DUK_DEFPROP_HAVE_SETTER;
			duk_def_prop(ctx, -3, flags);         // obj在-3（只有 key+setter）
		}
		else {
			duk_pop(ctx); // 既无get也无set，丢弃key
		}
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
