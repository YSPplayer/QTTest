/*
	Created By YSP
	2025.8.30
*/
#include "jscompiler.h"
namespace ysp::qt::html {
	QString JsCompiler::ToCompilerScript(const QString& script, bool html) {
		QString result = script;
		qint32 start = 0;
		if (html) {
			while ((start = result.indexOf("<script>", start)) != -1) {
				qint32 end = result.indexOf("</script>", start);
				if (end != -1) {
					// 提取脚本内容
					QString scriptContent = result.mid(start + 8, end - start - 8);
					scriptContent = PreprocessScriptTags(scriptContent);
					// 替换回原位置
					QString newScript = "<script>" + scriptContent + "</script>";
					result.replace(start, end + 9 - start, newScript);
					// 更新搜索位置
					start += newScript.length();
				}
				else {
					break;
				}
			}
		}
		else {
			result = PreprocessScriptTags(result);
		}
		return result;
	}
	QString JsCompiler::RrocessTemplateStrings(const QString& script) {
		QString result = script;
		qint32 start = 0;
		while ((start = result.indexOf('`', start)) != -1) {
			qint32 end = result.indexOf('`', start + 1);
			if (end != -1) {
				// 提取模板字符串内容
				QString templateContent = result.mid(start + 1, end - start - 1);

				// 先转义单引号
				templateContent.replace("'", "\\'");

				// 将换行符替换为字符串连接
				templateContent.replace("\n", "' + '");
				templateContent.replace("\r", "' + '");
				templateContent.replace("\t", "' + '");
				// 替换为字符串连接形式
				QString normalString = "'" + templateContent + "'";
				result.replace(start, end - start + 1, normalString);

				// 更新搜索位置
				start += normalString.length();
			}
			else {
				break;
			}
		}

		return result;
	}
	QString JsCompiler::PreprocessScriptTags(const QString& script) {
		QString result = script;
		// 转义特殊字符
		result.replace("<", "&lt;");
		result.replace(">", "&gt;");
		result = RrocessTemplateStrings(result);
		//scriptContent.replace("&", "&amp;");
		return result;
	}
}