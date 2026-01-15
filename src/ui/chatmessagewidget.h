#ifndef CHATMESSAGEWIDGET_H
#define CHATMESSAGEWIDGET_H


#include <QFrame>

class QTextBrowser;
class QPushButton;

class ChatMessageWidget : public QFrame
{
    Q_OBJECT
public:
    enum Role { User, Assistant };

    ChatMessageWidget(Role role, const QString &text, QWidget *parent = nullptr);

    QString text() const { return messageText; }
    void setText(const QString &text);

signals:
    void copyRequested(const QString &text);
    void insertRequested(const QString &text);
    void replaceRequested(const QString &text);

private:
    void updateDisplay();

    QString messageText;
    QTextBrowser *textBrowser;
};
#endif // CHATMESSAGEWIDGET_H
