/*
	Created By YSP
	2025.8.3
*/
#include "jsparser.h"
#include "linkbridge.h"
#include "listfilter.h"
#include "include.h"
#include <QFile>
#include <QDir>
#include <QApplication>
#include "jslibrary.h"
namespace ysp::qt::html {
	/*
	栈顶索引 -1 栈底索引0
	入栈是往栈顶添加
	*/
	JsParser::JsParser() {
		ctx = nullptr;
		binder = nullptr;
		init = false;
	}
	bool JsParser::Init() {
		if (init) return true;
		ctx = duk_create_heap_default();
		bool success = ctx != nullptr;
		if (success) {
			binder = new JSBinder(ctx);
			LoadJsLibrary();
			BindJsFunc();
		}
		init = success;
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
	/// <summary>
	/// 加载js三方库
	/// </summary>
	void JsParser::LoadJsLibrary() {
		RunJs(JS_LIBRARY_SCRIPT.toUtf8().constData());
	}
	void JsParser::BindJsFunc() {
		duk_push_pointer(ctx, this);
		duk_put_global_string(ctx, JSPARSER); //设置自己为全局变量

		binder->beginObject();
		binder->bindMethod("log", ConsoleLog, DUK_VARARGS);
		binder->setGlobal("console");

		binder->beginObject();
		binder->bindMethod("addEventListener", WindowAddEventListener, 2);
		binder->bindMethod("clearEvent", ClearEvent, 0);
		binder->setGlobal("window");

		binder->beginObject();
		binder->bindMethod("getElementById", DocumentGetElementById, 1);
		binder->bindMethod("getElementByKey", DocumentGetElementByKey, 1);
		binder->bindMethod("createElement", CreateElement, 1);
		binder->bindMethod("querySelector", DocumentQuerySelector, 1);
		binder->bindMethod("querySelectorAll", DocumentQuerySelectorAll, 1);
		binder->setGlobal("document");
	}
	void JsParser::PushJsValue(const std::shared_ptr<JsValue>& value) {
		PushJsValue(ctx, value);
	}
	void JsParser::PushJsValue(duk_context* ctx, const std::shared_ptr<JsValue>& value) {
		void* v = value->value;
		switch (value->type) {
		case JS_TYPE_STRING: { // std::string
			const std::string& str = *(std::string*)v;
			duk_push_string(ctx, str.c_str());
			break;
		}
		case JS_TYPE_OBJECT: {
			const std::string& str = *(std::string*)v;
			duk_get_global_string(ctx, str.c_str());
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
			PushJsObject(ctx, (JsClass*)v);
			break;
		}
		case JS_TYPE_ARRAY: { // std::vector<std::shared_ptr<JsValue>>
			PushJsArray(ctx, (JsArray*)v);
			break;
		}
		default:
			duk_push_undefined(ctx);
			break;
		}
	}
	void JsParser::PushJsObject(duk_context* ctx, const JsClass* obj) {
		duk_push_object(ctx);
		for (auto it = obj->begin(); it != obj->end(); ++it) {
			const QString& key = it->first;
			const std::shared_ptr<JsValue>& value = it->second;
			PushJsValue(ctx, value);
			duk_put_prop_string(ctx, -2, key.trimmed().toUtf8().constData());
		}
	}
	void JsParser::PushJsArray(duk_context* ctx, const JsArray* arr) {
		duk_push_array(ctx);
		for (size_t i = 0; i < arr->size(); ++i) {
			PushJsValue(ctx, arr->at(i));
			duk_put_prop_index(ctx, -2, i);
		}
	}
	JS_API duk_ret_t JsParser::ClearEvent(duk_context* ctx) {
		QByteArray eventKeyBytes;
		for (const QString& callbackType : LinkBridge::eventType) {
			qint32 index = 0;
			while (true) {
				eventKeyBytes = QString(QString(CXX_CUT_JS_CONST_VALUE) + callbackType + QString::number(index)).toUtf8();
				duk_get_global_string(ctx, eventKeyBytes.constData());
				if (!duk_is_function(ctx, -1)) {
					duk_pop(ctx);
					break;
				}
				else {
					duk_pop(ctx);
					duk_push_global_object(ctx);
					duk_del_prop_string(ctx, -1, eventKeyBytes.constData());
					duk_pop(ctx);
					index++;
				}
			}
		}
		return 0;
	}
	void JsParser::PushJsObject(const JsClass* obj) {
		PushJsObject(ctx, obj);

	}
	void JsParser::PushJsArray(const JsArray* arr) {
		PushJsArray(ctx, arr);
	}

	void JsParser::Trigger(const QString& callbackType, const std::vector<std::shared_ptr<JsValue>>& params, bool global) {
		if (!ctx) {
			return;
		}
		//调用所有绑定的事件
		qint32 index = 0;
		while (true) {
			if (global) {
				duk_get_global_string(ctx, QString(QString(CXX_CUT_JS_CONST_VALUE) + callbackType + QString::number(index)).toUtf8().constData());
			}
			else {
				duk_get_global_string(ctx, QString(callbackType + QString::number(index)).toUtf8().constData());
			}
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
				++index;
			}
			else {
				//不存在数据，不用管
				duk_pop(ctx);
				break;
			}
		}

	}
	void JsParser::Trigger(const QString& callbackType, const std::shared_ptr<JsValue>& param, bool global) {
		const std::vector<std::shared_ptr<JsValue>>& params = { param };
		Trigger(callbackType, params, global);
	}
	void JsParser::Trigger(const QString& callbackType, bool global) {
		const std::vector<std::shared_ptr<JsValue>> params;
		Trigger(callbackType, params, global);
	}

	/// <summary>
	/// 创建对象
	/// </summary>
	void JsParser::CreateDocument(QWidget* widget) {
		if (!widget || objects.contains(widget)) return;
		objects.append(widget);
		duk_push_object(ctx);
		//压入指针
		duk_push_pointer(ctx, widget);
		duk_put_prop_string(ctx, -2, K_PTRKEY);//参数二是绑定的对象，会弹出(消耗)栈顶的值
		binder->bindAttributeMethod("parentElement", DUK_GETTER("parentElement"), nullptr);
		binder->bindAttributeMethod("children", DUK_GETTER("children"), nullptr);
		binder->bindAttributeMethod("options", DUK_GETTER("options"), nullptr);
		binder->bindAttributeMethod("id", DUK_GETTER("id"), nullptr);
		binder->bindAttributeMethod("tagName", DUK_GETTER("tagName"), nullptr);
		binder->bindAttributeMethod("width", DUK_GETTER("width"), DUK_SETTER("width"));
		binder->bindAttributeMethod("height", DUK_GETTER("height"), DUK_SETTER("height"));
		binder->bindAttributeMethod("min", DUK_GETTER("min"), DUK_SETTER("min"));
		binder->bindAttributeMethod("max", DUK_GETTER("max"), DUK_SETTER("max"));
		binder->bindAttributeMethod("value", DUK_GETTER("value"), DUK_SETTER("value"));
		binder->bindAttributeMethod("textContent", DUK_GETTER("textContent"), DUK_SETTER("textContent"));
		binder->bindAttributeMethod("textWidth", DUK_GETTER("textWidth"), nullptr);
		binder->bindAttributeMethod("textHeight", DUK_GETTER("textHeight"), nullptr);
		binder->bindAttributeMethod("top", DUK_GETTER("top"), DUK_SETTER("top"));
		binder->bindAttributeMethod("bottom", DUK_GETTER("bottom"), DUK_SETTER("bottom"));
		binder->bindAttributeMethod("left", DUK_GETTER("left"), DUK_SETTER("left"));
		binder->bindAttributeMethod("leftEnd", DUK_GETTER("leftEnd"), nullptr);
		binder->bindAttributeMethod("src", nullptr, DUK_SETTER("src"));
		binder->bindAttributeMethod("right", DUK_GETTER("right"), DUK_SETTER("right"));
		binder->bindAttributeMethod("visible", DUK_GETTER("visible"), DUK_SETTER("visible"));
		binder->bindAttributeMethod("style", DUK_GETTER("style"), DUK_SETTER("style"));
		binder->bindAttributeMethod("disabled", DUK_GETTER("disabled"), DUK_SETTER("disabled"));
		binder->bindMethod("addEventListener", ObjectAddEventListener, 2);
		binder->bindMethod("setStyleSheet", SetStyleSheet, 1);
		binder->bindMethod("querySelector", QuerySelector, 1);
		binder->bindMethod("_", QuerySelector, 1); //语法糖
		binder->bindMethod("querySelectorAll", QuerySelectorAll, 1);
		binder->bindMethod("append", Append, 1);
		binder->bindMethod("remove", Remove, 1);
		binder->bindMethod("delete", Delete, 0);
		binder->bindMethod("mapTo", MapTo, 1);
		duk_put_global_string(ctx, CWidget::GetKeyString(widget).toUtf8().constData());
	}

	duk_ret_t JsParser::ThrowError(duk_context* ctx, duk_ret_t code, const QString& error) {
		duk_type_error(ctx, error.toUtf8().constData());
		return code;
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
			QString classname = w->metaObject()->className();
			QString key = QString::fromUtf8(name);
			if (key == "id") duk_push_string(ctx, CWidget::GetJsId(w).toUtf8().constData());
			else if (key == "tagName") duk_push_string(ctx, LinkBridge::QClassToHtmlClass(classname).toUpper().toUtf8().constData());
			else if (key == "width") duk_push_int(ctx, w->width());
			else if (key == "height") duk_push_int(ctx, w->height());
			else if (key == "top") duk_push_int(ctx, w->y());
			else if (key == "bottom") {
				parent ? duk_push_int(ctx, parent->height() - w->y() - w->height()) : duk_push_int(ctx, -1);
			}
			else if (key == "left") duk_push_int(ctx, w->x());
			else if (key == "leftEnd") duk_push_int(ctx, w->x() + w->width());
			else if (key == "right") {
				parent ? duk_push_int(ctx, parent->width() - w->x() - w->width()) : duk_push_int(ctx, -1);
			}
			else if (key == "style") {
				duk_push_string(ctx, LinkBridge::styleBuilder.contains(w) ?
					LinkBridge::styleBuilder[w].GetStyles().toUtf8().constData() : "");
			}
			else if (key == "visible") {
				duk_push_boolean(ctx, w->isVisible());
			}
			else if (key == "disabled") {
				duk_push_boolean(ctx, w->isEnabled());
			}
			else if (key == "parentElement") {
				if (parent) {
					duk_push_string(ctx, CWidget::GetKeyString(parent).toUtf8().constData());
					return DocumentGetElementByKey(ctx);
				}
				else {
					duk_push_null(ctx);
				}
			}
			else if (key == "children") {
				duk_idx_t arr_idx = duk_push_array(ctx);
				qint32 arr_index = 0;
				for (QWidget* child : w->findChildren<QWidget*>()) {
					if (!child) continue;
					duk_push_string(ctx, CWidget::GetKeyString(child).toUtf8().constData());
					DocumentGetElementByKey(ctx);
					if (!duk_is_null(ctx, -1)) {
						duk_put_prop_index(ctx, arr_idx, arr_index);
						++arr_index;
					}
					else {
						duk_pop(ctx);
					}
					duk_pop(ctx); //清理字符串，不会自动清理
				}
				ExpandArray(ctx, arr_idx, arr_index);
			}
			else if (classname == "QComboBox") {
				QComboBox* qcomboBox = (QComboBox*)w;
				if (key == "options") {
					duk_idx_t arr_idx = duk_push_array(ctx);
					qint32 arr_index = 0;
					duk_push_pointer(ctx, w);
					duk_put_prop_string(ctx, -2, K_PTRKEY);
					for (qint32 i = 0; i < qcomboBox->count(); ++i) {
						duk_push_int(ctx, i);
						duk_ret_t result = CreateOption(ctx);
						if (!duk_is_undefined(ctx, -1)) {
							duk_put_prop_index(ctx, arr_idx, arr_index);
							++arr_index;
						}
						else {
							duk_pop(ctx);
						}
						duk_pop(ctx); //清理压入的i，不会自动清理
					}
					ExpandArray(ctx, arr_idx, arr_index);
					duk_push_c_function(ctx, AddOption, DUK_VARARGS);
					duk_put_prop_string(ctx, arr_idx, "addOption");
					return 1;
				}
			}
			else if (classname == "QLabel") {
				QLabel* qlabel = (QLabel*)w;
				if (key == "textWidth") {
					QFontMetrics fm(qlabel->font());
					QString text = qlabel->text();
					duk_push_int(ctx, fm.size(Qt::TextSingleLine, text).width());
				}
				else if (key == "textHeight") {
					QFontMetrics fm(qlabel->font());
					QString text = qlabel->text();
					duk_push_int(ctx, fm.size(Qt::TextSingleLine, text).height());
				}
				else if (key == "textContent") {
					duk_push_string(ctx, qlabel->text().toUtf8().constData());
				}
			}
			else if (classname == "QProgressBar") {
				QProgressBar* bar = ((QProgressBar*)w);
				if (key == "min") {
					duk_push_int(ctx, bar->minimum());
				}
				else if (key == "max") {
					duk_push_int(ctx, bar->maximum());
				}
				else if (key == "value") {
					duk_push_int(ctx, bar->value());
				}
			}
			else {
				duk_push_undefined(ctx);
			}
			return 1;//返回值表示弹出
		}

		duk_push_undefined(ctx);
		return 1;
	}
	duk_ret_t JsParser::GetOption(duk_context* ctx, const char* name) {
		duk_idx_t initial_stack_size = duk_get_top(ctx);
		duk_push_this(ctx);
		if (!duk_get_prop_string(ctx, -1, K_PTRKEY)) { //不存在这个键 返回
			duk_set_top(ctx, initial_stack_size);
			duk_push_undefined(ctx);
			return 1;
		}
		QWidget* w = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
		duk_pop(ctx);
		if (!duk_get_prop_string(ctx, -1, "INDEX")) {
			duk_set_top(ctx, initial_stack_size);
			duk_push_undefined(ctx);
			return 1;
		}
		qint32 v = duk_get_number(ctx, -1);
		QString key = QString::fromUtf8(name);
		QString classname = QString::fromUtf8(w->metaObject()->className());
		duk_pop_2(ctx);

		if (w && classname == "QComboBox") {
			QComboBox* comboBox = (QComboBox*)w;
			if (v >= 0 && v < comboBox->count()) {
				if (key == "text") {
					duk_push_string(ctx, comboBox->itemText(v).toUtf8().constData());
				}
				else if (key == "value") {
					QVariant data = comboBox->itemData(v);
					if (data.isValid()) {
						duk_push_string(ctx, data.toString().toUtf8().constData());
					}
					else {
						duk_push_string(ctx, "");
					}
				}
				else {
					duk_push_undefined(ctx);
				}
			}
			else {
				duk_push_undefined(ctx);
			}
		}
		else {
			duk_push_undefined(ctx);
		}
		return 1;
	}
	duk_ret_t JsParser::SetOption(duk_context* ctx, const char* name) {
		duk_idx_t initial_stack_size = duk_get_top(ctx);
		duk_push_this(ctx);
		if (!duk_get_prop_string(ctx, -1, K_PTRKEY)) {
			duk_set_top(ctx, initial_stack_size);
			return 0;
		}
		QWidget* w = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
		duk_pop(ctx);
		if (!duk_get_prop_string(ctx, -1, "INDEX")) {
			duk_set_top(ctx, initial_stack_size);
			return 0;
		}
		qint32 v = duk_get_number(ctx, -1);
		QString key = QString::fromUtf8(name);
		QString classname = QString::fromUtf8(w->metaObject()->className());
		duk_pop_2(ctx);
		if (w && classname == "QComboBox") {
			QComboBox* comboBox = (QComboBox*)w;
			if (v >= 0 && v < comboBox->count()) {
				if (key == "text") {
					if (!duk_is_string(ctx, -1))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
						"parameter is not string.");
					comboBox->setItemText(v, QString::fromUtf8(duk_require_string(ctx, -1)));
				}
				else if (key == "value") {
					if (!duk_is_string(ctx, -1))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
						"parameter is not string.");
					const char* str = duk_require_string(ctx, -1);
					comboBox->setItemData(v,
						QString::fromUtf8(str).isEmpty() ? QVariant() : QVariant(str));
				}
			}
		}
		return 0;
	}
	duk_ret_t JsParser::SetValue(duk_context* ctx, const char* name) {
		QWidget* w = ThisWidget(ctx);
		if (!w) return 0;
		QString classname = w->metaObject()->className();
		QWidget* parent = w->parentWidget() ? w->parentWidget() : nullptr;
		QString key = QString::fromUtf8(name);

		if (key == "width") {
			if (!duk_is_number(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not number.");
			const qint32 v = duk_require_int(ctx, 0);
			w->resize(v, w->height());
		}
		else if (key == "height") {
			if (!duk_is_number(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not number.");
			const qint32 v = duk_require_int(ctx, 0);
			w->resize(w->width(), v);
		}
		else if (key == "left") {
			if (!duk_is_number(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not number.");
			const qint32 v = duk_require_int(ctx, 0);
			w->move(v, w->y());
		}
		else if (key == "top") {
			if (!duk_is_number(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not number.");
			const qint32 v = duk_require_int(ctx, 0);
			w->move(w->x(), v);
		}
		else if (key == "right") {
			if (!duk_is_number(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not number.");
			const qint32 v = duk_require_int(ctx, 0);
			if (parent) w->move(parent->width() - v - w->width(), w->y());
		}
		else if (key == "bottom") {
			if (!duk_is_number(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not number.");
			const qint32 v = duk_require_int(ctx, 0);
			if (parent) w->move(w->x(), parent->height() - v - w->height());
		}
		else if (key == "style") {
			if (!duk_is_string(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not string.");
			if (LinkBridge::styleBuilder.contains(w)) {
				const char* v = duk_require_string(ctx, 0);
				LinkBridge::styleBuilder[w].SetStyles(QString::fromUtf8(v));
				w->setStyleSheet(LinkBridge::styleBuilder[w].ToString());
			}
		}
		else if (key == "visible") {
			if (!duk_is_boolean(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not boolean.");
			const bool v = duk_require_boolean(ctx, 0);
			v ? w->show() : w->hide();
		}
		else if (key == "disabled") {
			if (!duk_is_boolean(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not boolean.");
			const bool v = duk_require_boolean(ctx, 0);
			w->setEnabled(v);
		}
		else if (classname == "QLabel") {
			QLabel* qlabel = (QLabel*)w;
			if (key == "textContent") {
				if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
					"parameter is not string.");
				const char* v = duk_require_string(ctx, 0);
				qlabel->setText(QString::fromUtf8(v));
			}
		}
		else if (classname == C_IMAGE) {
			CImage* cimage = (CImage*)w;
			if (key == "src") {
				if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
					"parameter is not string.");
				const char* v = duk_require_string(ctx, 0);
				QDir dir(QApplication::applicationDirPath());
				QString fullPath = dir.absoluteFilePath(QString::fromUtf8(v));
				fullPath = QDir(fullPath).canonicalPath();
				cimage->setPixmap(fullPath);
			}
		}
		else if (classname == "QProgressBar") {
			QProgressBar* bar = ((QProgressBar*)w);
			if (!duk_is_number(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"parameter is not number.");
			const qint32 v = duk_require_int(ctx, 0);
			if (key == "min") {
				bar->setMinimum(v);
			}
			else if (key == "max") {
				bar->setMaximum(v);
			}
			else if (key == "value") {
				bar->setValue(v);
			}
		}
		return 0;
	}
	void JsParser::ExpandArray(duk_context* ctx, qint32 position, qint32 length) {
		duk_push_c_function(ctx, ArrayForEach, 1);
		duk_put_prop_string(ctx, position, "forEach");
	}

	JS_API duk_ret_t JsParser::ArrayForEach(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR, "The number of parameters(1) is incorrect");
		}
		if (!duk_is_function(ctx, 0)) {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR, "parameter is not function.");
		}
		duk_push_this(ctx);
		if (!duk_is_array(ctx, -1)) {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR, "forEach must be called on an array");
		}
		// 获取数组长度
		duk_size_t length = duk_get_length(ctx, -1);
		// 遍历数组
		for (duk_size_t i = 0; i < length; i++) {
			// 获取数组元素
			duk_get_prop_index(ctx, -1, i);
			// 准备回调函数参数：[callback, element, index, array]
			duk_dup(ctx, 0);        // 复制回调函数
			duk_dup(ctx, -2);       // 复制元素值
			duk_push_int(ctx, i);   // 索引
			duk_dup(ctx, -4);       // 复制数组对象
			// 检查并添加length属性（如果不存在）
			duk_get_prop_string(ctx, -1, "length");
			if (duk_is_undefined(ctx, -1)) {
				duk_pop(ctx); // 清理undefined
				ExpandArray(ctx, -2, length);

			}
			else {
				duk_pop(ctx); // 清理已存在的length值
			}
			// 调用回调函数
			if (duk_pcall(ctx, 3) != DUK_EXEC_SUCCESS) {
				// 回调函数执行出错
				duk_pop(ctx); // 清理元素值
				return ThrowError(ctx, DUK_RET_ERROR, duk_safe_to_string(ctx, -1));
			}
			// 清理回调函数返回值
			duk_pop(ctx);
			// 清理元素值
			duk_pop(ctx);
		}
		// 清理this对象
		duk_pop(ctx);
		return 0;
	}
	JS_API duk_ret_t JsParser::AddOption(duk_context* ctx) {
		duk_ret_t topcount = duk_get_top(ctx);
		if (topcount < 1) {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR, "The number of parameters(1) is incorrect");
		}
		if (!duk_is_string(ctx, -1)) {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR, "parameter is not string.");
		}
		QString value = "";
		QString text = "";
		if (topcount == 2) {
			if (!duk_is_string(ctx, -2)) {
				return ThrowError(ctx, DUK_RET_TYPE_ERROR, "parameter is not string.");
			}
			value = QString::fromUtf8(duk_require_string(ctx, -1));
			text = QString::fromUtf8(duk_require_string(ctx, -2));
		}
		else if (topcount == 1) {
			text = QString::fromUtf8(duk_require_string(ctx, -1));
		}
		else {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR, "Too many parameters. Expected 1 or 2.");
		}
		duk_idx_t initial_stack_size = duk_get_top(ctx);
		duk_push_this(ctx);
		if (!duk_get_prop_string(ctx, -1, K_PTRKEY)) {
			duk_set_top(ctx, initial_stack_size);
			return 0;
		}
		QWidget* w = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
		duk_set_top(ctx, initial_stack_size);
		QString classname = QString::fromUtf8(w->metaObject()->className());
		if (w && classname == "QComboBox") {
			QComboBox* comboBox = (QComboBox*)w;
			comboBox->addItem(text, value);
			duk_push_this(ctx);
			duk_size_t current_length = duk_get_length(ctx, -1);
			duk_push_int(ctx, current_length);
			duk_ret_t result = CreateOption(ctx);
			if (!duk_is_undefined(ctx, -1)) {
				duk_put_prop_index(ctx, -3, current_length);
			}
			else {
				duk_pop(ctx);
			}
			duk_pop(ctx);
		}
		//更新数组大小
		return 0;
	}
	JS_API duk_ret_t JsParser::Delete(duk_context* ctx) {
		duk_get_global_string(ctx, JSPARSER);
		JsParser* parser = static_cast<JsParser*>(duk_get_pointer(ctx, -1));
		duk_pop(ctx);
		duk_push_this(ctx);
		if (!duk_get_prop_string(ctx, -1, K_PTRKEY)) { //不存在这个键 返回
			duk_pop_2(ctx);
			return 0;
		}
		QWidget* w = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
		duk_pop(ctx);
		if (w) {
			duk_del_prop_string(ctx, -1, K_PTRKEY);  // 删除当前JS绑定的指针属性
			parser->objects.removeOne(w);//数组移除
			LinkBridge::styleBuilder.remove(w);//样式移除
			LinkBridge::flagType.remove(w);//标签移除
			const QList<std::shared_ptr<ElementData>>& keys = LinkBridge::datamap.keys(w);
			for (auto& key : keys) {
				LinkBridge::datamap.remove(key);
			}
			w->setParent(nullptr);
			w->deleteLater();//对象删除
		}
		duk_pop(ctx);
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
		if (duk_get_top(ctx) < 2) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(2) is incorrect");
		if (!duk_is_string(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		const char* callbackType = duk_require_string(ctx, 0);
		if (!duk_is_function(ctx, 1)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not function.");
		qint32 index = 0;//实现插入多个函数，以实现可以绑定多个事件
		QByteArray eventKeyBytes;
		while (true) {
			eventKeyBytes = QString(QString(CXX_CUT_JS_CONST_VALUE) + QString::fromUtf8(callbackType) + QString::number(index)).toUtf8();
			duk_get_global_string(ctx, eventKeyBytes.constData());
			if (!duk_is_function(ctx, -1)) {
				duk_pop(ctx);
				duk_put_global_string(ctx, eventKeyBytes.constData());
				break;
			}
			duk_pop(ctx);
			++index;
		}
		return 0;
	}
	JS_API duk_ret_t JsParser::ObjectAddEventListener(duk_context* ctx) {
		if (duk_get_top(ctx) < 2) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(2) is incorrect");
		if (!duk_is_string(ctx, 0))  return DUK_RET_TYPE_ERROR;
		const char* callbackType = duk_require_string(ctx, 0);
		if (!duk_is_function(ctx, 1)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not function.");
		QWidget* w = ThisWidget(ctx);
		if (!w) return DUK_RET_ERROR;
		qint32 index = 0;
		QByteArray eventKeyBytes;
		while (true) {
			eventKeyBytes = QString(CWidget::GetKeyString(w) + QString::fromUtf8(callbackType) + QString::number(index)).toUtf8();
			duk_get_global_string(ctx, eventKeyBytes.constData());
			if (!duk_is_function(ctx, -1)) {
				duk_pop(ctx);
				duk_put_global_string(ctx, eventKeyBytes.constData());
				break;
			}
			duk_pop(ctx);
			++index;
		}
		return 0;
	}
	JS_API duk_ret_t JsParser::DocumentGetElementById(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		const char* elementId = duk_require_string(ctx, 0);
		const QString& id = QString::fromUtf8(elementId);
		duk_get_global_string(ctx, JSPARSER);
		JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
		duk_pop(ctx);
		QList<QWidget*> widegts = ListFilter::Where<QWidget*>(ptr->objects, [id](QWidget* widget)->bool {
			return CWidget::GetJsId(widget) == id;
			});
		if (widegts.count() > 0 && widegts[0]) {
			const QString& globalKey = CWidget::GetKeyString(widegts[0]);
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
		else {
			duk_push_null(ctx);
			return 1;
		}
	}
	JS_API duk_ret_t JsParser::DocumentQuerySelector(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		const char* str = duk_require_string(ctx, 0);
		const QString& classname = QString::fromUtf8(str).trimmed();
		duk_get_global_string(ctx, JSPARSER);
		JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
		duk_pop(ctx);
		QList<QWidget*> widegts;
		if (classname.startsWith(".")) { //类名
			QList<QString> classlist = classname.split(".", Qt::SkipEmptyParts);
			for (QString& str : classlist) {
				str = str.trimmed();
			}
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [classlist](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClass(widget);
				for (const auto& key : classlist) {
					if (!cname.contains(key)) return false;
				}
				return true;
				});
		}
		else { //标签
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [classname](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClassName(widget);
				if (LinkBridge::QClassToHtmlClass(cname) == classname) {
					return true;
				}
				});
		}
		if (widegts.count() > 0 && widegts[0]) {
			const QString& globalKey = CWidget::GetKeyString(widegts[0]);
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
		else {
			duk_push_null(ctx);
			return 1;
		}
	}
	JS_API duk_ret_t JsParser::DocumentQuerySelectorAll(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		const char* str = duk_require_string(ctx, 0);
		const QString& classname = QString::fromUtf8(str);
		duk_get_global_string(ctx, JSPARSER);
		JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
		duk_pop(ctx);
		QList<QWidget*> widegts;
		if (classname.startsWith(".")) { //类名
			QList<QString> classlist = classname.split(".", Qt::SkipEmptyParts);
			for (QString& str : classlist) {
				str = str.trimmed();
			}
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [classlist](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClass(widget);
				for (const auto& key : classlist) {
					if (!cname.contains(key)) return false;
				}
				return true;
				});
		}
		else { //标签
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [classname](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClassName(widget);
				if (LinkBridge::QClassToHtmlClass(cname) == classname) {
					return true;
				}
				});
		}
		duk_idx_t arr_idx = duk_push_array(ctx);
		for (qint32 i = 0; i < widegts.count(); i++) {
			QWidget* widget = widegts[i];
			if (!widget) {
				duk_push_null(ctx);
				duk_put_prop_index(ctx, arr_idx, i);
				continue;
			}
			const QString& globalKey = CWidget::GetKeyString(widget);
			duk_get_global_string(ctx, globalKey.toUtf8().constData());
			if (duk_is_undefined(ctx, -1)) {
				duk_pop(ctx);
				duk_push_null(ctx);
			}
			duk_put_prop_index(ctx, arr_idx, i);
		}
		ExpandArray(ctx, arr_idx, widegts.count());
		return 1;
	}
	JS_API duk_ret_t JsParser::DocumentGetElementByKey(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, -1)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		const char* key = duk_require_string(ctx, -1);
		duk_get_global_string(ctx, key);
		if (!duk_is_undefined(ctx, -1)) {
			return 1;
		}
		else {
			duk_pop(ctx);
			duk_push_null(ctx);
			return 1;
		}
	}
	JS_API duk_ret_t JsParser::Append(duk_context* ctx) { //参数就是自动压入的函数 require函数就是检查的意思 也就是不压栈 本来数据就在
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_object(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not object.");
		auto* w = ThisWidget(ctx);
		if (!w) {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"C++ object is null.");
		}
		QWidget* child = nullptr;
		duk_require_object(ctx, 0);//获取到参数对象
		duk_get_prop_string(ctx, -1, K_PTRKEY);
		if (!duk_is_undefined(ctx, -1)) {
			child = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
			duk_pop_2(ctx);
			if (child) {
				child->setParent(w);
				w->isVisible() ? child->show() : child->hide();
			}
			else {
				return ThrowError(ctx, DUK_RET_TYPE_ERROR,
					"C++ object is null.");
			}
		}
		else {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"object is undefined.");
		}
		return 0;
	}
	JS_API duk_ret_t JsParser::QuerySelector(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		QWidget* w = ThisWidget(ctx);
		if (!w) {
			duk_pop(ctx);
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"C++ object is null.");
		}
		const char* str = duk_require_string(ctx, 0);
		const QString& classname = QString::fromUtf8(str).trimmed();
		duk_get_global_string(ctx, JSPARSER);
		JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
		duk_pop(ctx);
		QList<QWidget*> widegts;
		if (classname.startsWith(".")) { //类名
			QList<QString> classlist = classname.split(".", Qt::SkipEmptyParts);
			for (QString& str : classlist) {
				str = str.trimmed();
			}
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [w, classlist](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClass(widget);
				for (const auto& key : classlist) {
					if (!cname.contains(key) || widget->parent() != w) return false;
				}
				return true;
				});
		}
		else { //标签
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [w, classname](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClassName(widget);
				if (widget->parent() == w && LinkBridge::QClassToHtmlClass(cname) == classname) {
					return true;
				}
				});
		}
		if (widegts.count() > 0 && widegts[0]) {
			const QString& globalKey = CWidget::GetKeyString(widegts[0]);
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
		else {
			duk_push_null(ctx);
			return 1;
		}
	}
	JS_API duk_ret_t JsParser::QuerySelectorAll(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		QWidget* w = ThisWidget(ctx);
		if (!w) {
			duk_pop(ctx);
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"C++ object is null.");
		}
		const char* str = duk_require_string(ctx, 0);
		const QString& classname = QString::fromUtf8(str).trimmed();
		duk_get_global_string(ctx, JSPARSER);
		JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
		duk_pop(ctx);
		QList<QWidget*> widegts;
		if (classname.startsWith(".")) { //类名
			QList<QString> classlist = classname.split(".", Qt::SkipEmptyParts);
			for (QString& str : classlist) {
				str = str.trimmed();
			}
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [w, classlist](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClass(widget);
				for (const auto& key : classlist) {
					if (!cname.contains(key) || widget->parent() != w) return false;
				}
				return true;
				});
		}
		else { //标签
			widegts = ListFilter::Where<QWidget*>(ptr->objects, [w, classname](QWidget* widget)->bool {
				const QString& cname = CWidget::GetClassName(widget);
				if (widget->parent() == w && LinkBridge::QClassToHtmlClass(cname) == classname) {
					return true;
				}
				});
		}
		duk_idx_t arr_idx = duk_push_array(ctx);
		for (qint32 i = 0; i < widegts.count(); i++) {
			QWidget* widget = widegts[i];
			if (!widget) {
				duk_push_null(ctx);
				duk_put_prop_index(ctx, arr_idx, i);
				continue;
			}
			const QString& globalKey = CWidget::GetKeyString(widget);
			duk_get_global_string(ctx, globalKey.toUtf8().constData());
			if (duk_is_undefined(ctx, -1)) {
				duk_pop(ctx);
				duk_push_null(ctx);
			}
			duk_put_prop_index(ctx, arr_idx, i);
		}
		ExpandArray(ctx, arr_idx, widegts.count());
		return 1;
	}
	JS_API duk_ret_t JsParser::Remove(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_object(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not object.");
		QWidget* w = ThisWidget(ctx);
		if (!w) {
			duk_pop(ctx);
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"C++ object is null.");
		}
		duk_get_global_string(ctx, JSPARSER);
		JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
		duk_pop(ctx);
		QWidget* child = nullptr;
		duk_require_object(ctx, 0);
		if (duk_get_prop_string(ctx, -1, K_PTRKEY)) {
			child = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
			duk_pop(ctx);
			if (child) {
				for (QWidget* widget : w->findChildren<QWidget*>()) {
					if (widget == child) {
						//duk_del_prop_string(ctx, -1, K_PTRKEY);  // 删除当前JS绑定的指针属性
						//ptr->objects.removeOne(child);//数组也要移除
						//CWidget::styleBuilder.remove(w);//样式移除
						child->setParent(nullptr);
						child->hide();
						//child->deleteLater(); //仅仅移除元素，不释放内存
						break;
					}
				}
			}
			duk_pop(ctx);
		}
		else {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"object is undefined.");
		}
		return 0;
	}
	JS_API duk_ret_t JsParser::MapTo(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_object(ctx, 0))  return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not object.");
		auto* w = ThisWidget(ctx);
		if (!w) {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"C++ object is null.");
		}
		QWidget* map = nullptr;
		duk_require_object(ctx, 0);//获取到参数对象
		duk_get_prop_string(ctx, -1, K_PTRKEY);
		if (!duk_is_undefined(ctx, -1)) {
			map = static_cast<QWidget*>(duk_get_pointer(ctx, -1));
			duk_pop_2(ctx);
			if (map) {
				QPoint postion = w->mapTo(map, QPoint(0, 0));
				JsClass* obj = new JsClass;
				(*obj)["x"] = JsValue::CreateValue(postion.x());
				(*obj)["y"] = JsValue::CreateValue(postion.y());
				PushJsValue(ctx, JsValue::CreateValue(obj));
			}
			else {
				return ThrowError(ctx, DUK_RET_TYPE_ERROR,
					"C++ object is null.");
			}
		}
		else {
			return ThrowError(ctx, DUK_RET_TYPE_ERROR,
				"object is undefined.");
		}
		return 1;

	}
	JS_API duk_ret_t JsParser::CreateElement(duk_context* ctx) {
		if (duk_get_top(ctx) < 1)return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		const QString& classname = QString::fromUtf8(duk_require_string(ctx, 0)).trimmed().toLower();
		QWidget* widget = nullptr;
		if (classname == "div") widget = new CWidget();
		else if (classname == "label") widget = new CLabel();
		else if (classname == "progress") widget = new CProgressBar();
		else if (classname == "select") widget = new CComboBox();
		else if (classname == "img") widget = new CImage();
		else if (classname == "button") widget = new CPushButton();
		//widget->setObjectName(CWidget::GetKeyString(widget));//默认的objectname
		//LinkBridge::styleBuilder[widget] = StyleBuilder(widget);
		LinkBridge::ParseAttributes(std::make_shared<ElementData>().get(), widget);
		duk_get_global_string(ctx, JSPARSER);
		JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
		duk_pop(ctx);
		ptr->CreateDocument(widget);
		duk_get_global_string(ctx, CWidget::GetKeyString(widget).toUtf8().constData());
		if (!duk_is_undefined(ctx, -1)) {
			return 1;
		}
		else {
			duk_pop(ctx);
			duk_push_null(ctx);
			return 1;
		}
	}
	JS_API duk_ret_t JsParser::SetStyleSheet(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_string(ctx, 0)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not string.");
		const char* stylesheet = duk_require_string(ctx, 0);
		QWidget* w = ThisWidget(ctx);
		if (!w) return DUK_RET_ERROR;
		/*auto style = LinkBridge::ReplaceAfterHash(stylesheet, CWidget::GetId(w));*/
		w->setStyleSheet(stylesheet);
		return 0;
	}

	/// <summary>
	/// combox的option
	/// </summary>
	JS_API duk_ret_t JsParser::CreateOption(duk_context* ctx) {
		if (duk_get_top(ctx) < 1) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"The number of parameters(1) is incorrect");
		if (!duk_is_number(ctx, -1)) return ThrowError(ctx, DUK_RET_TYPE_ERROR,
			"parameter is not number.");
		const qint32 v = duk_require_int(ctx, -1);
		if (auto* w = ThisWidget(ctx)) {
			QComboBox* combox = (QComboBox*)w;
			duk_get_global_string(ctx, JSPARSER);
			JsParser* ptr = (JsParser*)duk_get_pointer(ctx, -1);
			duk_pop(ctx);
			duk_push_object(ctx);
			duk_push_pointer(ctx, w);
			duk_put_prop_string(ctx, -2, K_PTRKEY);
			duk_push_number(ctx, v);
			duk_put_prop_string(ctx, -2, "INDEX");
			ptr->binder->bindAttributeMethod("value", [](duk_context* ctx)->duk_ret_t {
				return GetOption(ctx, "value");
				}, [](duk_context* ctx)->duk_ret_t {
					return SetOption(ctx, "value");
					});
				ptr->binder->bindAttributeMethod("text", [](duk_context* ctx)->duk_ret_t {
					return GetOption(ctx, "text");
					}, [](duk_context* ctx)->duk_ret_t {
						return SetOption(ctx, "text");
						});

		}
		else {
			duk_push_undefined(ctx);
		}
		return 1;
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
		case JS_TYPE_OBJECT: delete static_cast<std::string*>(value); break;
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

	std::shared_ptr<JsValue> JsValue::CreateValue(std::string value) {
		return std::make_shared<JsValue>(new std::string(value), JS_TYPE_STRING);
	}

	std::shared_ptr<JsValue> JsValue::CreateObjectValue(std::string value) {
		return std::make_shared<JsValue>(new std::string(value), JS_TYPE_OBJECT);
	}

	std::shared_ptr<JsValue> JsValue::CreateValue(JsClass* value) {
		return std::make_shared<JsValue>(value, JS_TYPE_CLASS);
	}

	std::shared_ptr<JsValue> JsValue::CreateValue(JsArray* value) {
		return std::make_shared<JsValue>(value, JS_TYPE_ARRAY);
	}

}
