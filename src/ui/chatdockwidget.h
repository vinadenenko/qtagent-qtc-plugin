#ifndef CHATDOCKWIDGET_H
#define CHATDOCKWIDGET_H

#include <QDockWidget>
#include <QVBoxLayout>


#include "src/llmmanager.h"
#include "src/ui/typingindicatorwidget.h"

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

    void startTypingAnimation();
    void stopTypingAnimation();
private:
    QVBoxLayout *chatLayout;
    // QTextEdit *chatView;
    QLineEdit *input;
    QPushButton *sendButton;

    TypingIndicatorWidget *typingIndicator_ = nullptr;

    LLMManager *llmManager;
};

#endif // CHATDOCKWIDGET_H
