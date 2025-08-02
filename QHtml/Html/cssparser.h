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
namespace ysp::qt::html {
    // CSS���Խṹ
    struct CSSProperty {
        QString name;       // ������
        QString value;      // ����ֵ
        bool important;     // �Ƿ���Ҫ

        CSSProperty() : important(false) {}
        CSSProperty(const QString& n, const QString& v, bool imp = false)
            : name(n), value(v), important(imp) {}
    };

    // CSS����ṹ
    struct CSSRule {
        QString selector;                           // ѡ����
        QMap<QString, CSSProperty> properties;     // ����ӳ��

        CSSRule() {}
        CSSRule(const QString& sel) : selector(sel) {}
    };

    // CSS��������
    class CSSParser : public QObject
    {
        Q_OBJECT

    public:
        explicit CSSParser(QObject* parent = nullptr);
        ~CSSParser();

        // ����CSS�ַ���
        bool parseCSS(const QString& cssString);

        // ��ȡ�������
        QList<CSSRule> getRules() const { return m_rules; }

        // ����ѡ�������ҹ���
        QList<CSSRule> findRulesBySelector(const QString& selector) const;

        // ��ȡ�ض�ѡ����������
        QMap<QString, QString> getProperties(const QString& selector) const;

        // ������й���
        void clear();

        // ��ӹ���
        void addRule(const CSSRule& rule);

        // �������
        void debugPrint() const;

    private:
        QList<CSSRule> m_rules;

        // ��������CSS����
        CSSRule parseRule(const QString& ruleText);

        // ����ѡ����
        QString parseSelector(const QString& selectorText);

        // ������������
        QMap<QString, CSSProperty> parseProperties(const QString& propertiesText);

        // ������������
        CSSProperty parseProperty(const QString& propertyText);

        // ����ͱ�׼��CSS�ı�
        QString cleanCSS(const QString& cssText);

        // �ָ�CSS����
        QStringList splitRules(const QString& cssText);

        // �ָ���������
        QStringList splitProperties(const QString& propertiesText);
    };


}