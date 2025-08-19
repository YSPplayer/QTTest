/*
	Created By YSP
	2025.8.2
*/
#pragma once
#include <QObject>
#include <QString>
#include <QMap>
#include <QList>
#include <QRegularExpression>
#include <QDebug>
#include <QWidget>
namespace ysp::qt::html {
    // CSS属性结构
    struct CSSProperty {
        QString name;       // 属性名
        QString value;      // 属性值
        bool important;     // 是否重要

        CSSProperty() : important(false) {}
        CSSProperty(const QString& n, const QString& v, bool imp = false)
            : name(n), value(v), important(imp) {}
    };

    // CSS规则结构
    struct CSSRule {
        QString selector;                           // 选择器
        QMap<QString, CSSProperty> properties;     // 属性映射
        CSSRule() {}
        CSSRule(const QString& sel) : selector(sel) {}
        bool CheckRule(QWidget* widget);
        QString GetSelectorHander();
        QString GetPropertiesStyle();
    };

    // CSS解析器类
    class CSSParser : public QObject
    {
        Q_OBJECT

    public:
        explicit CSSParser(QObject* parent = nullptr);
        ~CSSParser();

        // 解析CSS字符串
        bool parseCSS(const QString& cssString);

        // 获取解析结果
        QList<CSSRule*> getRules() const { return m_rules; }
        // 根据选择器查找规则
        QList<CSSRule*> findRulesBySelector(const QString& selector) const;

        // 获取特定选择器的属性
        QMap<QString, QString> getProperties(const QString& selector) const;

        // 清除所有规则
        void clear();

        // 添加规则
        void addRule(CSSRule* rule);

        // 调试输出
        void debugPrint() const;

    private:
        QList<CSSRule*> m_rules;

        // 解析单个CSS规则
        CSSRule* parseRule(const QString& ruleText);

        // 解析选择器
        QString parseSelector(const QString& selectorText);

        // 解析属性声明
        QMap<QString, CSSProperty> parseProperties(const QString& propertiesText);

        // 解析单个属性
        CSSProperty parseProperty(const QString& propertyText);

        // 清理和标准化CSS文本
        QString cleanCSS(const QString& cssText);

        // 分割CSS规则
        QStringList splitRules(const QString& cssText);

        // 分割属性声明
        QStringList splitProperties(const QString& propertiesText);
    };


}