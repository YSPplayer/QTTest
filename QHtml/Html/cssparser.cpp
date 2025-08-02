/*
    Created By YSP
    2025.8.2
*/
#include "cssparser.h"
namespace ysp::qt::html {
#include "cssparser.h"

    CSSParser::CSSParser(QObject* parent)
        : QObject(parent)
    {
    }

    CSSParser::~CSSParser()
    {
    }

    bool CSSParser::parseCSS(const QString& cssString)
    {
        clear();

        try {
            QString cleanedCSS = cleanCSS(cssString);
            QStringList rules = splitRules(cleanedCSS);

            for (const QString& ruleText : rules) {
                if (!ruleText.trimmed().isEmpty()) {
                    CSSRule rule = parseRule(ruleText);
                    if (!rule.selector.isEmpty()) {
                        m_rules.append(rule);
                    }
                }
            }

            return true;
        }
        catch (const std::exception& e) {
            qWarning() << "CSS parsing error:" << e.what();
            return false;
        }
    }

    QString CSSParser::cleanCSS(const QString& cssText)
    {
        QString cleaned = cssText;

        // �Ƴ�ע��
        QRegularExpression commentRegex(R"(\/\*.*?\*\/)", QRegularExpression::DotMatchesEverythingOption);
        cleaned.remove(commentRegex);

        // �Ƴ�����Ŀհ��ַ�
        cleaned = cleaned.simplified();

        // ��׼�����з�
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

    CSSRule CSSParser::parseRule(const QString& ruleText)
    {
        CSSRule rule;

        // ����ѡ���������Բ���
        int braceStart = ruleText.indexOf('{');
        int braceEnd = ruleText.lastIndexOf('}');

        if (braceStart == -1 || braceEnd == -1) {
            return rule;
        }

        // ����ѡ����
        QString selectorText = ruleText.left(braceStart).trimmed();
        rule.selector = parseSelector(selectorText);

        // ��������
        QString propertiesText = ruleText.mid(braceStart + 1, braceEnd - braceStart - 1);
        rule.properties = parseProperties(propertiesText);

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

        // ������һ������
        if (!currentProperty.trimmed().isEmpty()) {
            properties.append(currentProperty.trimmed());
        }

        return properties;
    }

    CSSProperty CSSParser::parseProperty(const QString& propertyText)
    {
        CSSProperty property;

        // ����ð�ŷָ���
        int colonIndex = propertyText.indexOf(':');
        if (colonIndex == -1) {
            return property;
        }

        // ����������
        property.name = propertyText.left(colonIndex).trimmed();

        // ��������ֵ
        QString valueText = propertyText.mid(colonIndex + 1).trimmed();

        // ����Ƿ���� !important
        if (valueText.contains("!important", Qt::CaseInsensitive)) {
            property.important = true;
            valueText.remove(QRegularExpression(R"(\s*!important\s*$)", QRegularExpression::CaseInsensitiveOption));
        }

        property.value = valueText.trimmed();

        return property;
    }

    QList<CSSRule> CSSParser::findRulesBySelector(const QString& selector) const
    {
        QList<CSSRule> foundRules;

        for (const CSSRule& rule : m_rules) {
            if (rule.selector.contains(selector, Qt::CaseInsensitive)) {
                foundRules.append(rule);
            }
        }

        return foundRules;
    }

    QMap<QString, QString> CSSParser::getProperties(const QString& selector) const
    {
        QMap<QString, QString> properties;

        for (const CSSRule& rule : m_rules) {
            if (rule.selector.contains(selector, Qt::CaseInsensitive)) {
                for (auto it = rule.properties.begin(); it != rule.properties.end(); ++it) {
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

    void CSSParser::addRule(const CSSRule& rule)
    {
        m_rules.append(rule);
    }

    void CSSParser::debugPrint() const
    {
        qDebug() << "=== CSS Parser Debug Output ===";
        qDebug() << "Total rules:" << m_rules.size();

        for (int i = 0; i < m_rules.size(); ++i) {
            const CSSRule& rule = m_rules[i];
            qDebug() << "\nRule" << (i + 1) << ":";
            qDebug() << "  Selector:" << rule.selector;
            qDebug() << "  Properties:";

            for (auto it = rule.properties.begin(); it != rule.properties.end(); ++it) {
                QString important = it.value().important ? " !important" : "";
                qDebug() << "    " << it.key() << ":" << it.value().value << important;
            }
        }
        qDebug() << "=== End Debug Output ===";
    }
}