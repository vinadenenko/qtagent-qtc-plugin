#include "chatmessagewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>

ChatMessageWidget::ChatMessageWidget(Role role, const QString &text, QWidget *parent)
    : QFrame(parent), messageText(text)
{
    setObjectName("ChatBubble");
    setStyleSheet(role == User
                      ? "#ChatBubble { background:#2d3b55; border-radius:8px; padding:6px; }"
                      : "#ChatBubble { background:#2b2b2b; border-radius:8px; padding:6px; }");

    textBrowser = new QTextBrowser();
    textBrowser->setOpenExternalLinks(true);
    textBrowser->setReadOnly(true);
    textBrowser->setFrameShape(QFrame::NoFrame);
    textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textBrowser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    // Background should match the bubble
    QPalette pal = textBrowser->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    textBrowser->setPalette(pal);

    updateDisplay();

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(textBrowser);

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

        connect(copyBtn, &QPushButton::clicked, this, [=]{ emit copyRequested(messageText); });
        connect(insertBtn, &QPushButton::clicked, this, [=]{ emit insertRequested(messageText); });
        connect(replaceBtn, &QPushButton::clicked, this, [=]{ emit replaceRequested(messageText); });
    }
}

void ChatMessageWidget::setText(const QString &text)
{
    messageText = text;
    updateDisplay();
}

void ChatMessageWidget::updateDisplay()
{
    // Simple markdown-to-html-ish conversion for code blocks
    // In a real app we'd use a proper library.
    QString html = messageText;
    html.replace("\n", "<br>");
    
    // Minimal code block highlighting attempt
    if (html.contains("```")) {
        // This is very naive but better than nothing for "Part 3"
        // Ideally we use a real MD renderer.
    }

    textBrowser->setMarkdown(messageText);
    
    // Adjust height to content
    textBrowser->document()->setTextWidth(textBrowser->viewport()->width());
    int height = textBrowser->document()->size().height() + 10;
    textBrowser->setMinimumHeight(height);
    textBrowser->setMaximumHeight(height);
}
