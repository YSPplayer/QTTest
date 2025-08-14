/*
	Created By YSP
	2025.8.6
*/
#include "consolewindow.h"
namespace ysp::qt::html {
	ConsoleWindow::ConsoleWindow(QWidget* parent) {
        first = false;
		setWindowTitle("Console Window");
        resize(600, 400);
        QVBoxLayout* layout = new QVBoxLayout(this);
        textEdit = new QTextEdit(this);
        textEdit->setReadOnly(true); 
        textEdit->setFont(QFont("Consolas", 10)); 
        layout->addWidget(textEdit);
        clearButton = new QPushButton("Clear", this);
        connect(clearButton, &QPushButton::clicked, this, &ConsoleWindow::Clear);
        layout->addWidget(clearButton);
        connect(this, &ConsoleWindow::appendText, this, &ConsoleWindow::AppendText);
	}
    void ConsoleWindow::AppendText(const QString& text) {
        if (text.trimmed().contains("Error", Qt::CaseInsensitive)) {
            // 设置错误颜色（红色）
            QTextCharFormat errorFormat;
            errorFormat.setForeground(QColor(220, 53, 69)); // 红色
            // 应用格式并添加文本
            QTextCursor cursor = textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText((first ? "\n" : "") + text, errorFormat);
        }
        else {
            QTextCharFormat errorFormat;
            errorFormat.setForeground(Qt::black); 
            // 应用格式并添加文本
            QTextCursor cursor = textEdit->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText((first ? "\n" : "") + text, errorFormat);
        }
        // 自动滚动到底部
        QTextCursor cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        textEdit->setTextCursor(cursor);
        first = true;
    }

    void ConsoleWindow::Clear() {
        textEdit->clear();
    }
}
