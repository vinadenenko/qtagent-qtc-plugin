#include "enhancedchatmessagewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

EnhancedChatMessageWidget::EnhancedChatMessageWidget(Role role, const QString &text, QWidget *parent)
    : QFrame(parent), messageText(text)
{
    setObjectName("ChatBubble");
    setStyleSheet(role == User
                      ? "#ChatBubble { background:#2d3b55; border-radius:8px; padding:6px; }"
                      : "#ChatBubble { background:#2b2b2b; border-radius:8px; padding:6px; }");

    auto textLabel = new QLabel(text);
    textLabel->setWordWrap(true);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(textLabel);

    if (role == Assistant) {
        actionParser = new CodeActionParser(this);
        detectedActions = actionParser->parseActions(text);
        
        buttonLayout = new QVBoxLayout;
        
        setupBasicActions();
        setupCodeActions();
        
        if (buttonLayout->count() > 0) {
            mainLayout->addLayout(buttonLayout);
        }
    }
}

void EnhancedChatMessageWidget::setupBasicActions()
{
    auto basicButtonLayout = new QHBoxLayout;
    
    auto copyBtn = new QPushButton("Copy");
    auto insertBtn = new QPushButton("Insert");
    auto replaceBtn = new QPushButton("Replace");

    basicButtonLayout->addStretch();
    basicButtonLayout->addWidget(copyBtn);
    basicButtonLayout->addWidget(insertBtn);
    basicButtonLayout->addWidget(replaceBtn);

    buttonLayout->addLayout(basicButtonLayout);

    connect(copyBtn, &QPushButton::clicked, this, [=]{ emit copyRequested(messageText); });
    connect(insertBtn, &QPushButton::clicked, this, [=]{ emit insertRequested(messageText); });
    connect(replaceBtn, &QPushButton::clicked, this, [=]{ emit replaceRequested(messageText); });
}

void EnhancedChatMessageWidget::setupCodeActions()
{
    if (detectedActions.isEmpty()) {
        return;
    }
    
    auto actionLabel = new QLabel("Detected Actions:");
    actionLabel->setStyleSheet("color: #888; font-size: 11px;");
    buttonLayout->addWidget(actionLabel);
    
    for (const auto &action : detectedActions) {
        QString buttonText = action.description;
        
        if (action.type == CodeAction::CreateFile) {
            addActionButton(buttonText, "create_file", QString("%1|%2").arg(action.filePath, action.content));
        } else if (action.type == CodeAction::UpdateFile) {
            addActionButton(buttonText, "update_file", QString("%1|%2").arg(action.filePath, action.content));
        } else if (action.type == CodeAction::DeleteFile) {
            addActionButton(buttonText, "delete_file", action.filePath);
        } else if (action.type == CodeAction::InsertCode) {
            addActionButton(buttonText, "insert_code", action.content);
        } else if (action.type == CodeAction::ReplaceCode) {
            addActionButton(buttonText, "replace_code", action.content);
        }
    }
}

void EnhancedChatMessageWidget::addActionButton(const QString &text, const QString &action, const QString &data)
{
    auto actionButtonLayout = new QHBoxLayout;
    
    auto actionBtn = new QPushButton(text);
    actionBtn->setStyleSheet("QPushButton { background: #3a3a3a; border: 1px solid #555; border-radius: 4px; padding: 4px 8px; font-size: 11px; }"
                              "QPushButton:hover { background: #4a4a4a; }");
    
    actionButtonLayout->addStretch();
    actionButtonLayout->addWidget(actionBtn);
    
    buttonLayout->addLayout(actionButtonLayout);
    
    actionBtn->setProperty("action", action);
    actionBtn->setProperty("data", data);
    
    connect(actionBtn, &QPushButton::clicked, this, &EnhancedChatMessageWidget::onActionClicked);
}

void EnhancedChatMessageWidget::onActionClicked()
{
    auto button = qobject_cast<QPushButton*>(sender());
    if (!button) {
        return;
    }
    
    QString action = button->property("action").toString();
    QString data = button->property("data").toString();
    
    emit actionRequested(action, data);
}