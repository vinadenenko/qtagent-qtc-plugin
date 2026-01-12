#ifndef TYPINGINDICATORWIDGET_H
#define TYPINGINDICATORWIDGET_H

#include <QLabel>
#include <QTimer>

class TypingIndicatorWidget : public QLabel
{
    Q_OBJECT
public:
    explicit TypingIndicatorWidget(QWidget *parent=nullptr);

private:
    QTimer timer;
    int state = 0;
};
#endif // TYPINGINDICATORWIDGET_H
