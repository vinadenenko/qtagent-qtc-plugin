#include "chatdockwidget.h"

#include "chatmessagewidget.h"
#include "src/providers/ollama/ollamaprovider.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

// ChatDockWidget::ChatDockWidget(QWidget *parent)
//     : QDockWidget("LLM Assistant", parent)
// {
//     auto container = new QWidget(this);

//     chatView = new QTextEdit;
//     chatView->setReadOnly(true);
//     chatView->setPlaceholderText("LLM conversation will appear here...");

//     input = new QLineEdit;
//     input->setPlaceholderText("Ask something...");

//     sendButton = new QPushButton("Send");

//     auto bottomLayout = new QHBoxLayout;
//     bottomLayout->addWidget(input);
//     bottomLayout->addWidget(sendButton);

//     auto mainLayout = new QVBoxLayout(container);
//     mainLayout->addWidget(chatView);
//     mainLayout->addLayout(bottomLayout);

//     container->setLayout(mainLayout);
//     setWidget(container);
// }

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QTimer>
#include <QClipboard>
#include <QApplication>

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

    input = new QLineEdit;
    input->setPlaceholderText("Ask something...");

    auto sendBtn = new QPushButton("Send");

    auto bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(input);
    bottomLayout->addWidget(sendBtn);

    auto mainLayout = new QVBoxLayout(root);
    mainLayout->addWidget(scroll);
    mainLayout->addLayout(bottomLayout);

    setWidget(root);

    connect(sendBtn, &QPushButton::clicked, this, [=]{
        const QString text = input->text().trimmed();
        if (text.isEmpty()) return;

        input->clear();
        addUserMessage(text);

        // MOCK LLM RESPONSE
        // QTimer::singleShot(600, this, [=]{
        //     addAssistantMessage("Mock LLM response for:\n" + text);
        // });
    });

    llmManager = new LLMManager(this);

    auto ollama = new OllamaProvider(this);
    ollama->setBaseUrl("http://localhost:11434");
    ollama->setModel("llama3");

    llmManager->setProvider(ollama);

    connect(llmManager, &LLMManager::responseReady,
            this, &ChatDockWidget::addAssistantMessage);

    connect(llmManager, &LLMManager::errorOccurred,
            this, &ChatDockWidget::addAssistantMessage);
}

void ChatDockWidget::addUserMessage(const QString &text)
{
    auto bubble = new ChatMessageWidget(ChatMessageWidget::User, text);

    auto wrapper = new QHBoxLayout;
    wrapper->addStretch();
    wrapper->addWidget(bubble);

    chatLayout->insertLayout(chatLayout->count()-1, wrapper);

    llmManager->sendPrompt(text);
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
