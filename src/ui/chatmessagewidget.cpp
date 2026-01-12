#include "chatmessagewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ChatMessageWidget::ChatMessageWidget(Role role, const QString &text, QWidget *parent)
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

    if (role == Assistant)
    {
        auto buttonLayout = new QHBoxLayout;

        auto copyBtn = new QPushButton("Copy");
        auto insertBtn = new QPushButton("Insert");
        auto replaceBtn = new QPushButton("Replace");

        buttonLayout->addStretch();
        buttonLayout->addWidget(copyBtn);
        buttonLayout->addWidget(insertBtn);
        buttonLayout->addWidget(replaceBtn);

        mainLayout->addLayout(buttonLayout);

        connect(copyBtn, &QPushButton::clicked, this, [=]{ emit copyRequested(text); });
        connect(insertBtn, &QPushButton::clicked, this, [=]{ emit insertRequested(text); });
        connect(replaceBtn, &QPushButton::clicked, this, [=]{ emit replaceRequested(text); });
    }
}
