/*
	Created By YSP
	2025.8.2
*/
#include "cssparser.h"
#include "linkbridge.h"
namespace ysp::qt::html {

	CSSParser::CSSParser(QObject* parent)
		: QObject(parent)
	{
	}

	CSSParser::~CSSParser()
	{
		/*   for (qint32 i = 0; i < m_rules.count(); ++i) {
			   delete m_rules[i];
			   m_rules[i] = nullptr;
		   }*/
	}

	bool CSSParser::parseCSS(const QString& cssString)
	{
		clear();

		try {
			QString cleanedCSS = cleanCSS(cssString);
			QStringList rules = splitRules(cleanedCSS);

			for (const QString& ruleText : rules) {
				if (!ruleText.trimmed().isEmpty()) {
					CSSRule* rule = parseRule(ruleText);
					if (!rule->selector.isEmpty()) {
						m_rules.append(rule);
					}
				}
			}

			return m_rules.count() > 0;
		}
		catch (const std::exception& e) {
			qWarning() << "CSS parsing error:" << e.what();
			return false;
		}
	}

	QString CSSParser::cleanCSS(const QString& cssText)
	{
		QString cleaned = cssText;

		// 移除注释
		QRegularExpression commentRegex(R"(\/\*.*?\*\/)", QRegularExpression::DotMatchesEverythingOption);
		cleaned.remove(commentRegex);

		// 移除多余的空白字符
		cleaned = cleaned.simplified();

		// 标准化换行符
		cleaned.replace(QRegularExpression(R"(\r\n|\r)"), "\n");

		return cleaned;
	}

	QStringList CSSParser::splitRules(const QString& cssText)
	{
		QStringList rules;
		QString currentRule;
		int braceCount = 0;
		bool inRule = false;

		for (int i = 0; i < cssText.length(); ++i) {
			QChar ch = cssText[i];

			if (ch == '{') {
				braceCount++;
				inRule = true;
			}
			else if (ch == '}') {
				braceCount--;
				if (braceCount == 0) {
					currentRule += ch;
					rules.append(currentRule.trimmed());
					currentRule.clear();
					inRule = false;
					continue;
				}
			}

			if (inRule || !ch.isSpace()) {
				currentRule += ch;
			}
		}

		return rules;
	}

	CSSRule* CSSParser::parseRule(const QString& ruleText)
	{
		CSSRule* rule = new CSSRule;

		// 查找选择器和属性部分
		int braceStart = ruleText.indexOf('{');
		int braceEnd = ruleText.lastIndexOf('}');

		if (braceStart == -1 || braceEnd == -1) {
			return rule;
		}

		// 解析选择器
		QString selectorText = ruleText.left(braceStart).trimmed();
		rule->selector = parseSelector(selectorText);

		// 解析属性
		QString propertiesText = ruleText.mid(braceStart + 1, braceEnd - braceStart - 1);
		rule->properties = parseProperties(propertiesText);

		return rule;
	}

	QString CSSParser::parseSelector(const QString& selectorText)
	{
		return selectorText.trimmed();
	}

	QMap<QString, CSSProperty> CSSParser::parseProperties(const QString& propertiesText)
	{
		QMap<QString, CSSProperty> properties;
		QStringList propertyList = splitProperties(propertiesText);

		for (const QString& propertyText : propertyList) {
			CSSProperty property = parseProperty(propertyText);
			if (!property.name.isEmpty()) {
				properties[property.name] = property;
			}
		}

		return properties;
	}

	QStringList CSSParser::splitProperties(const QString& propertiesText)
	{
		QStringList properties;
		QString currentProperty;
		int parenCount = 0;

		for (int i = 0; i < propertiesText.length(); ++i) {
			QChar ch = propertiesText[i];

			if (ch == '(') {
				parenCount++;
			}
			else if (ch == ')') {
				parenCount--;
			}
			else if (ch == ';' && parenCount == 0) {
				if (!currentProperty.trimmed().isEmpty()) {
					properties.append(currentProperty.trimmed());
				}
				currentProperty.clear();
				continue;
			}

			currentProperty += ch;
		}

		// 添加最后一个属性
		if (!currentProperty.trimmed().isEmpty()) {
			properties.append(currentProperty.trimmed());
		}

		return properties;
	}

	CSSProperty CSSParser::parseProperty(const QString& propertyText)
	{
		CSSProperty property;

		// 查找冒号分隔符
		int colonIndex = propertyText.indexOf(':');
		if (colonIndex == -1) {
			return property;
		}

		// 解析属性名
		property.name = propertyText.left(colonIndex).trimmed();

		// 解析属性值
		QString valueText = propertyText.mid(colonIndex + 1).trimmed();

		// 检查是否包含 !important
		if (valueText.contains("!important", Qt::CaseInsensitive)) {
			property.important = true;
			valueText.remove(QRegularExpression(R"(\s*!important\s*$)", QRegularExpression::CaseInsensitiveOption));
		}

		property.value = valueText.trimmed();

		return property;
	}

	QList<CSSRule*> CSSParser::findRulesBySelector(const QString& selector) const
	{
		QList<CSSRule*> foundRules;

		for (CSSRule* rule : m_rules) {
			if (rule->selector.contains(selector, Qt::CaseInsensitive)) {
				foundRules.append(rule);
			}
		}

		return foundRules;
	}

	QMap<QString, QString> CSSParser::getProperties(const QString& selector) const
	{
		QMap<QString, QString> properties;

		for (CSSRule* rule : m_rules) {
			if (rule->selector.contains(selector, Qt::CaseInsensitive)) {
				for (auto it = rule->properties.begin(); it != rule->properties.end(); ++it) {
					properties[it.key()] = it.value().value;
				}
			}
		}

		return properties;
	}

	void CSSParser::clear()
	{
		m_rules.clear();
	}

	void CSSParser::addRule(CSSRule* rule)
	{
		m_rules.append(rule);
	}

	void CSSParser::debugPrint() const
	{
		qDebug() << "=== CSS Parser Debug Output ===";
		qDebug() << "Total rules:" << m_rules.size();

		for (int i = 0; i < m_rules.size(); ++i) {
			CSSRule* rule = m_rules[i];
			qDebug() << "\nRule" << (i + 1) << ":";
			qDebug() << "  Selector:" << rule->selector;
			qDebug() << "  Properties:";

			for (auto it = rule->properties.begin(); it != rule->properties.end(); ++it) {
				QString important = it.value().important ? " !important" : "";
				qDebug() << "    " << it.key() << ":" << it.value().value << important;
			}
		}
		qDebug() << "=== End Debug Output ===";
	}

	bool CSSRule::CheckRule(QWidget* widget) {
		const QString& id = widget->property("jsid").isValid() ?
			widget->property("jsid").toString() : "";
		const QString& classValue = widget->property("class").isValid() ?
			widget->property("class").toString() : "";
		const QString& classType = LinkBridge::QClassToHtmlClass(widget->metaObject()->className());
		const QList<QString>& selectors = selector.split(":");
		if (selectors.count() > 0) {
			const QString& key = selectors[0];
			qint32 index = 0;
			QString mid = "";
			QList<QString> filter = { "#",".",":", "" };
			if (id != "") {
				index = LinkBridge::FindSubstringEndIndex(key, "#" + id);
				if (index != -1) {
					mid = index >= key.length() - 1 ? "" : key.mid(index, 1);
					if (filter.contains(mid)) return true;
				}
			}
			if (classValue != "") {
				const QStringList& classList = classValue.split(" ", Qt::SkipEmptyParts);
				for (const QString& singleClass : classList) {
					if (!singleClass.trimmed().isEmpty()) {
						index = LinkBridge::FindSubstringEndIndex(key, "." + singleClass.trimmed());
						if (index != -1) {
							mid = index >= key.length() - 1 ? "" : key.mid(index, 1);
							if (filter.contains(mid)) return true;
						}
					}
				}
			}
			if (classType != "") {
				index = LinkBridge::FindSubstringEndIndex(key, classType);
				if (index != -1) {
					mid = index >= key.length() - 1 ? "" : key.mid(index, 1);
					if (filter.contains(mid)) return true;
				}
			}
		}
		return false;
	}

	/// <summary>
	/// 返回选择器:后面的部分
	/// </summary>
	/// <returns></returns>
	QString CSSRule::GetSelectorHander() {
		const QStringList& parts = selector.split(":", Qt::SkipEmptyParts);
		return parts.size() >= 2 ? parts[1] : "";
	}
	QString CSSRule::GetPropertiesStyle() {
		QString result = "";
		for (const QString& key : properties.keys()) {
			auto& value = properties[key];
			result += QString("%1:%2").arg(value.name).arg(value.value);
		}
		return result;
	}
}