/*
	Created By YSP
	2025.8.6
*/
#include "consolewindow.h"
namespace ysp::qt::html {
	ConsoleWindow::ConsoleWindow(QWidget* parent) {
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
        textEdit->append(text);
        // 自动滚动到底部
        QTextCursor cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        textEdit->setTextCursor(cursor);
    }

    void ConsoleWindow::Clear() {
        textEdit->clear();
    }
}
