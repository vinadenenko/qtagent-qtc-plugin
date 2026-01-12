#include "chatdockwidget.h"

#include "chatmessagewidget.h"
#include "src/providers/ollama/ollamaprovider.h"


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QWidget>
#include <QTimer>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>

ChatDockWidget::ChatDockWidget(QWidget *parent)
    : QDockWidget("LLM Assistant", parent)
{
    auto root = new QWidget(this);

    auto scroll = new QScrollArea;
    scroll->setWidgetResizable(true);

    auto chatContainer = new QWidget;
    chatLayout = new QVBoxLayout(chatContainer);
    chatLayout->addStretch();

    scroll->setWidget(chatContainer);

    input = new QTextEdit;
    input->setFixedHeight(70);
    input->setAcceptRichText(false);
    input->setPlaceholderText("Ask something...");

    input->installEventFilter(this);

    auto sendBtn = new QPushButton("Send");

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(input);
    bottomLayout->addWidget(sendBtn);

    auto mainLayout = new QVBoxLayout(root);
    mainLayout->addWidget(scroll);
    mainLayout->addLayout(bottomLayout);

    setWidget(root);

    connect(sendBtn, &QPushButton::clicked, this, &ChatDockWidget::onSendClicked);

    llmManager = new LLMManager(this);

    auto ollama = new OllamaProvider(this);
    ollama->setBaseUrl("http://localhost:11434");
    ollama->setModel("llama3");

    llmManager->setProvider(ollama);

    connect(llmManager, &LLMManager::responseReady, this, [this](const QString &t){
        stopTypingAnimation();
        addAssistantMessage(t);
    });

    connect(llmManager, &LLMManager::errorOccurred, this, [this](const QString &t){
        stopTypingAnimation();
        addAssistantMessage("Error: " + t);
    });
}

bool ChatDockWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == input && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                return false; // insert newline
            }
            onSendClicked(); // send message
            return true;
        }
    }
    return QDockWidget::eventFilter(obj, event);
}

void ChatDockWidget::onSendClicked()
{
    const QString text = input->toPlainText().trimmed();
    if (text.isEmpty()) return;

    input->clear();
    addUserMessage(text);

    // MOCK LLM RESPONSE
    // QTimer::singleShot(600, this, [=]{
    //     addAssistantMessage("Mock LLM response for:\n" + text);
    // });
}

void ChatDockWidget::addUserMessage(const QString &text)
{
    auto bubble = new ChatMessageWidget(ChatMessageWidget::User, text);

    auto wrapper = new QHBoxLayout;
    wrapper->addStretch();
    wrapper->addWidget(bubble);

    chatLayout->insertLayout(chatLayout->count()-1, wrapper);

    llmManager->sendPrompt(text);

    startTypingAnimation();
}

void ChatDockWidget::addAssistantMessage(const QString &text)
{
    auto bubble = new ChatMessageWidget(ChatMessageWidget::Assistant, text);

    connect(bubble, &ChatMessageWidget::copyRequested, this, [=](const QString &t){
        QApplication::clipboard()->setText(t);
    });

    connect(bubble, &ChatMessageWidget::insertRequested, this, [=](const QString &t){
        // placeholder — editor insertion later
        QApplication::clipboard()->setText(t);
    });

    connect(bubble, &ChatMessageWidget::replaceRequested, this, [=](const QString &t){
        // placeholder — editor replace later
        QApplication::clipboard()->setText(t);
    });

    auto wrapper = new QHBoxLayout;
    wrapper->addWidget(bubble);
    wrapper->addStretch();

    chatLayout->insertLayout(chatLayout->count()-1, wrapper);
}

void ChatDockWidget::startTypingAnimation()
{
    typingIndicator_ = new TypingIndicatorWidget;

    auto qwe = new QHBoxLayout;
    qwe->addWidget(typingIndicator_);
    qwe->addStretch();

    chatLayout->insertLayout(chatLayout->count()-1, qwe);
}

void ChatDockWidget::stopTypingAnimation()
{
    delete typingIndicator_;
    typingIndicator_ = nullptr;
}
