#include "typingindicatorwidget.h"

TypingIndicatorWidget::TypingIndicatorWidget(QWidget *parent)
    : QLabel(parent)
{
    setText("●");
    setStyleSheet("color: gray; font-size: 18px;");
    timer.setInterval(400);

    connect(&timer, &QTimer::timeout, this, [this]{
        state = (state + 1) % 3;
        setText(QString("●").repeated(state + 1));
    });

    timer.start();
}
