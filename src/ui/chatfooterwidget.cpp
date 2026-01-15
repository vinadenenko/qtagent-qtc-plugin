#include "chatfooterwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>

ChatFooterWidget::ChatFooterWidget(QWidget *parent)
    : QFrame(parent)
{
    setFixedHeight(28);
    setStyleSheet("ChatFooterWidget { background: #1e1e1e; border-top: 1px solid #333; color: #aaa; font-size: 10px; } "
                  "QLabel { color: #aaa; } "
                  "QProgressBar { background: #333; border: none; border-radius: 2px; text-align: center; height: 4px; } "
                  "QProgressBar::chunk { background: #4a9eff; }");

    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(15);

    modelLabel = new QLabel("Model: None");
    tokenLabel = new QLabel("Tokens: 0 / 32k");
    modelLabel->setMaximumWidth(100);
    
    usageBar = new QProgressBar;
    usageBar->setRange(0, 100);
    usageBar->setValue(0);
    usageBar->setTextVisible(false);
    usageBar->setMaximumWidth(100);
    usageBar->setFixedHeight(4);

    layout->addWidget(modelLabel);
    layout->addStretch();
    layout->addWidget(tokenLabel);
    layout->addWidget(usageBar);
}

void ChatFooterWidget::setModelName(const QString &name)
{
    modelLabel->setText("Model: " + name + name + name);
}

void ChatFooterWidget::setTokenUsage(int current, int max)
{
    float maxK = max / 1000.0f;
    tokenLabel->setText(QString("Tokens: %1 / %2k").arg(current).arg(QString::number(maxK, 'f', 1)));
    int percent = (max > 0) ? (current * 100 / max) : 0;
    usageBar->setValue(qMin(100, percent));
    
    if (percent > 90) {
        usageBar->setStyleSheet("QProgressBar::chunk { background: #ff4444; }");
    } else if (percent > 70) {
        usageBar->setStyleSheet("QProgressBar::chunk { background: #ffaa00; }");
    } else {
        usageBar->setStyleSheet("QProgressBar::chunk { background: #4a9eff; }");
    }
}
