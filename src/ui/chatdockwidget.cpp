#include "chatdockwidget.h"

#include "chatmessagewidget.h"
#include "src/providers/ollama/ollamaprovider.h"
#include "src/providers/openai/openaiprovider.h"
#include "src/providers/claude/claudeprovider.h"
#include "src/settings/llmsettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>
#include <QPushButton>
#include <QWidget>
#include <QTimer>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>

#include "src/core/codeeditormanager.h"
#include "src/mcp/mcpserver.h"

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

    sendButton = new QPushButton("Send");

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(input);
    bottomLayout->addWidget(sendButton);

    auto mainLayout = new QVBoxLayout(root);
    mainLayout->addWidget(scroll);
    mainLayout->addLayout(bottomLayout);

    setWidget(root);

    connect(sendButton, &QPushButton::clicked, this, &ChatDockWidget::onSendClicked);

    llmManager = new LLMManager(this);
    
    auto editorManager = new CodeEditorManager(this);
    auto mcpServer = new MCPServer(editorManager, this);
    llmManager->setMCPServer(mcpServer);

    initProvider();

    connect(llmManager, &LLMManager::responseReady, this, [this](const QString &t){
        stopTypingAnimation();
        if (currentAssistantBubble) {
            currentAssistantBubble->setText(t);
            currentAssistantBubble = nullptr;
        } else {
            addAssistantMessage(t);
        }
    });

    connect(llmManager, &LLMManager::partialResponse, this, [this](const QString &delta){
        stopTypingAnimation();
        updateAssistantMessage(delta);
    });

    connect(llmManager, &LLMManager::streamFinished, this, [this](){
        stopTypingAnimation();
        currentAssistantBubble = nullptr;
    });

    connect(llmManager, &LLMManager::toolCallStarted, this, [this](const QString &name){
        addAssistantMessage("🔧 *Calling tool:* **" + name + "**...");
        currentAssistantBubble = nullptr; // Next message should be a new bubble or the result
    });

    connect(llmManager, &LLMManager::toolCallFinished, this, [this](const QString &name, const QString &result){
        // Optional: you could show the result in a collapsed way
        // addAssistantMessage("✅ *Tool* **" + name + "** *finished.*");
        currentAssistantBubble = nullptr;
    });

    connect(llmManager, &LLMManager::errorOccurred, this, [this](const QString &t){
        stopTypingAnimation();
        auto bubble = new ChatMessageWidget(ChatMessageWidget::Error, "**Error:** " + t);
        auto wrapper = new QHBoxLayout;
        wrapper->addWidget(bubble);
        wrapper->addStretch();
        chatLayout->insertLayout(chatLayout->count()-1, wrapper);
        currentAssistantBubble = nullptr;
    });
}

void ChatDockWidget::initProvider()
{
    auto &s = LLMSettings::instance();
    QString type = s.providerType();
    
    LLMProvider *provider = nullptr;
    if (type == "OpenAI") {
        auto p = new OpenAIProvider(this);
        p->setBaseUrl(s.baseUrl());
        p->setModel(s.model());
        p->setApiKey(s.apiKey());
        provider = p;
    } else if (type == "Claude") {
        auto p = new ClaudeProvider(this);
        p->setBaseUrl(s.baseUrl());
        p->setModel(s.model());
        p->setApiKey(s.apiKey());
        provider = p;
    } else {
        auto p = new OllamaProvider(this);
        p->setBaseUrl(s.baseUrl());
        p->setModel(s.model());
        provider = p;
    }
    
    llmManager->setProvider(provider);
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
    
    // Check if provider settings changed
    initProvider(); 
    
    llmManager->sendChatRequest(text);
    startTypingAnimation();
}

void ChatDockWidget::addUserMessage(const QString &text)
{
    auto bubble = new ChatMessageWidget(ChatMessageWidget::User, text);

    auto wrapper = new QHBoxLayout;
    wrapper->addStretch();
    wrapper->addWidget(bubble);

    chatLayout->insertLayout(chatLayout->count()-1, wrapper);
}

void ChatDockWidget::addAssistantMessage(const QString &text)
{
    auto bubble = new ChatMessageWidget(ChatMessageWidget::Assistant, text);

    connect(bubble, &ChatMessageWidget::copyRequested, this, [=](const QString &t){
        QApplication::clipboard()->setText(t);
    });

    connect(bubble, &ChatMessageWidget::insertRequested, this, [=](const QString &t){
        CodeEditorManager cem;
        cem.insertText(t);
    });

    connect(bubble, &ChatMessageWidget::replaceRequested, this, [=](const QString &t){
        CodeEditorManager cem;
        cem.replaceSelectedText(t);
    });

    auto wrapper = new QHBoxLayout;
    wrapper->addWidget(bubble);
    wrapper->addStretch();

    chatLayout->insertLayout(chatLayout->count()-1, wrapper);
}

void ChatDockWidget::updateAssistantMessage(const QString &delta)
{
    if (!currentAssistantBubble) {
        currentAssistantBubble = new ChatMessageWidget(ChatMessageWidget::Assistant, "");
        
        connect(currentAssistantBubble, &ChatMessageWidget::copyRequested, this, [=](const QString &t){
            QApplication::clipboard()->setText(t);
        });

        auto wrapper = new QHBoxLayout;
        wrapper->addWidget(currentAssistantBubble);
        wrapper->addStretch();

        chatLayout->insertLayout(chatLayout->count()-1, wrapper);
    }
    
    currentAssistantBubble->setText(currentAssistantBubble->text() + delta);
    
    // Scroll to bottom
    if (auto scroll = qobject_cast<QScrollArea*>(widget()->layout()->itemAt(0)->widget())) {
        scroll->verticalScrollBar()->setValue(scroll->verticalScrollBar()->maximum());
    }
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
