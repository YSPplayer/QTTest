/*
	Created By YSP
	2025.8.3
*/
#include "jsparser.h"
#include "linkbridge.h"
namespace ysp::qt::html {
	/*
	栈顶索引 -1 栈底索引0
	入栈是往栈顶添加
	*/
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
			// 先编译检查语法
			duk_compile_string(ctx, 0, script);
			if (duk_pcall(ctx, 0) != 0) {
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
	}
	JS_API duk_ret_t JsParser::ConsoleLog(duk_context* ctx) {
		duk_push_string(ctx, " ");//往栈顶压入分隔符
		duk_insert(ctx, 0); //把栈顶的元素插入到栈底
		duk_join(ctx, duk_get_top(ctx) - 1);//连接所有的分隔符
		LinkBridge::Print(duk_safe_to_string(ctx, -1));//转为字符串
		return 0;
	}
	void JSBinder::beginObject() {
		duk_push_object(ctx);
	}

	void JSBinder::bindMethod(const char* name, duk_c_function func, int nargs) {
		duk_push_c_function(ctx, func, nargs);
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

}
