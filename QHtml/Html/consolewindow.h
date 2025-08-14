/*
    Created By YSP
    2025.8.6
*/
#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QPushButton>
namespace ysp::qt::html {
    class ConsoleWindow : public QWidget {
        Q_OBJECT

    public:
        ConsoleWindow(QWidget* parent = nullptr);
        
        void Clear();
    signals:
        void appendText(const QString& text);
    private:
        void AppendText(const QString& text);
        bool first;
        QTextEdit* textEdit;
        QPushButton* clearButton;
    };
}