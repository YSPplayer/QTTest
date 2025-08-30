/*
	Created By YSP
	2025.8.30
*/
#pragma once
#include <QString>
namespace ysp::qt::html {
	const QString JS_LIBRARY_SCRIPT = R"(
	function $(template) {
		var args = [];
			for (var i = 1; i < arguments.length; i++) {
				args.push(arguments[i]);
			}
			return template.replace(/\{(\d+)?\}/g, function(match, index) {
			if (index === undefined) {
				return args.shift() || '';
			}
			return args[parseInt(index)] || '';
		});
		}
	var _ = {
		"#":function(id) {return document.getElementById(id);}
		}
	)";
}