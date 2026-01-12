#ifndef CHATDOCKWIDGET_H
#define CHATDOCKWIDGET_H

#include <QDockWidget>
#include <QVBoxLayout>


#include "src/llmmanager.h"

class QTextEdit;
class QLineEdit;
class QPushButton;

class ChatDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit ChatDockWidget(QWidget *parent = nullptr);

private:
    void addUserMessage(const QString &text);
    void addAssistantMessage(const QString &text);
private:
    QVBoxLayout *chatLayout;
    // QTextEdit *chatView;
    QLineEdit *input;
    QPushButton *sendButton;

    LLMManager *llmManager;
};

#endif // CHATDOCKWIDGET_H
