#ifndef CHATFOOTERWIDGET_H
#define CHATFOOTERWIDGET_H

#include <QFrame>

class QLabel;
class QProgressBar;

class ChatFooterWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ChatFooterWidget(QWidget *parent = nullptr);

    void setModelName(const QString &name);
    void setTokenUsage(int current, int max);

private:
    QLabel *modelLabel;
    QLabel *tokenLabel;
    QProgressBar *usageBar;
};

#endif // CHATFOOTERWIDGET_H
