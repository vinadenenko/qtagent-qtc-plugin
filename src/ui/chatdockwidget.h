#ifndef CHATDOCKWIDGET_H
#define CHATDOCKWIDGET_H

#include <QDockWidget>
#include <QVBoxLayout>

#include "src/llmmanager.h"
#include "src/ui/typingindicatorwidget.h"
#include "src/ui/chatmessagewidget.h"
#include "src/ui/chatfooterwidget.h"

class QTextEdit;
class QPushButton;

class ChatDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit ChatDockWidget(QWidget *parent = nullptr);

    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onSendClicked();

private:
    void addUserMessage(const QString &text);
    void addAssistantMessage(const QString &text);
    void updateAssistantMessage(const QString &delta);

    void startTypingAnimation();
    void stopTypingAnimation();
    void initProvider();

private:
    QVBoxLayout *chatLayout;
    QTextEdit *input;
    QPushButton *sendButton;

    TypingIndicatorWidget *typingIndicator_ = nullptr;
    ChatMessageWidget *currentAssistantBubble = nullptr;
    ChatFooterWidget *footer = nullptr;

    LLMManager *llmManager;
};

#endif // CHATDOCKWIDGET_H
