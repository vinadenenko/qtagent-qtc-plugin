#ifndef CHATMESSAGEWIDGET_H
#define CHATMESSAGEWIDGET_H


#include <QFrame>

class QLabel;
class QPushButton;

class ChatMessageWidget : public QFrame
{
    Q_OBJECT
public:
    enum Role { User, Assistant };

    ChatMessageWidget(Role role, const QString &text, QWidget *parent = nullptr);

signals:
    void copyRequested(const QString &text);
    void insertRequested(const QString &text);
    void replaceRequested(const QString &text);

private:
    QString messageText;
};
#endif // CHATMESSAGEWIDGET_H
