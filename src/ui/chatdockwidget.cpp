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

    footer = new ChatFooterWidget(root);
    mainLayout->addWidget(footer);

    setWidget(root);

    connect(sendButton, &QPushButton::clicked, this, &ChatDockWidget::onSendClicked);

    llmManager = new LLMManager(this);
    
    // Connect settings changes to provider re-initialization
    // and also to footer update for context limit
    connect(&LLMSettings::instance(), &LLMSettings::settingsChanged, this, [this](){
        initProvider();
        if (footer) {
             footer->setTokenUsage(llmManager->history().estimateTokenCount(), LLMSettings::instance().contextLimit());
        }
    });
    
    auto editorManager = new CodeEditorManager(this);
    auto mcpServer = new MCPServer(editorManager, this);
    llmManager->setMCPServer(mcpServer);

    // New conversation button
    auto clearButton = new QPushButton("New Chat");
    bottomLayout->insertWidget(0, clearButton);
    connect(clearButton, &QPushButton::clicked, this, [this](){
        llmManager->clearHistory();
        
        // Remove all bubbles from UI
        QLayoutItem *child;
        while ((child = chatLayout->takeAt(0)) != nullptr) {
            if (child->layout()) {
                // It's a wrapper layout
                QLayoutItem *wrapperChild;
                while ((wrapperChild = child->layout()->takeAt(0)) != nullptr) {
                    if (wrapperChild->widget()) delete wrapperChild->widget();
                    delete wrapperChild;
                }
                delete child->layout();
            } else if (child->widget()) {
                delete child->widget();
            }
            delete child;
        }
        chatLayout->addStretch();
        currentAssistantBubble = nullptr;
    });

    initProvider();

    connect(llmManager, &LLMManager::historyUpdated, this, [this](){
        if (footer) {
            footer->setTokenUsage(llmManager->history().estimateTokenCount(), LLMSettings::instance().contextLimit());
        }
    });

    connect(llmManager, &LLMManager::modelInfoReceived, this, [this](const QString &model){
        if (footer) {
            qDebug() << "REC" << model;
            footer->setModelName(model);
        }
    });

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
        auto bubble = new ChatMessageWidget(ChatMessageWidget::Tool, "🔧 **Calling tool:** `" + name + "`...");
        auto wrapper = new QHBoxLayout;
        wrapper->addWidget(bubble);
        wrapper->addStretch();
        chatLayout->insertLayout(chatLayout->count()-1, wrapper);
        currentAssistantBubble = nullptr; 
    });

    connect(llmManager, &LLMManager::toolCallFinished, this, [this](const QString &name, const QString &result){
        auto bubble = new ChatMessageWidget(ChatMessageWidget::Tool, "✅ **Tool finished:** `" + name + "`\n\nResult:\n```json\n" + result + "\n```");
        auto wrapper = new QHBoxLayout;
        wrapper->addWidget(bubble);
        wrapper->addStretch();
        chatLayout->insertLayout(chatLayout->count()-1, wrapper);
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
    
    if (footer) {
        footer->setModelName(s.model());
    }
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
    
    // Check if provider settings changed - only if necessary
    // initProvider(); 
    
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
