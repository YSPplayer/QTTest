/*
	Created By YSP
	2025.8.30
*/
#pragma once
#include <QString>
namespace ysp::qt::html {
	class JsCompiler {
	public:
		static QString ToCompilerScript(const QString& script, bool html);
	private:
		static QString  RrocessTemplateStrings(const QString& script);
		static QString PreprocessScriptTags(const QString& script);
	};
}