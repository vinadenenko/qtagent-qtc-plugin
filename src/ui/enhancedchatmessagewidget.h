#ifndef ENHANCEDCHATMESSAGEWIDGET_H
#define ENHANCEDCHATMESSAGEWIDGET_H

#include <QFrame>
#include <QVBoxLayout>
#include "src/core/codeactionparser.h"



class EnhancedChatMessageWidget : public QFrame
{
    Q_OBJECT
public:
    enum Role { User, Assistant };

    EnhancedChatMessageWidget(Role role, const QString &text, QWidget *parent = nullptr);

signals:
    void copyRequested(const QString &text);
    void insertRequested(const QString &text);
    void replaceRequested(const QString &text);
    void actionRequested(const QString &action, const QString &data);

private slots:
    void onActionClicked();
    // void updateActions();

private:
    void setupBasicActions();
    void setupCodeActions();
    void addActionButton(const QString &text, const QString &action, const QString &data);
    
    QString messageText;
    CodeActionParser *actionParser;
    QList<CodeAction> detectedActions;
    QVBoxLayout *buttonLayout;
};

#endif // ENHANCEDCHATMESSAGEWIDGET_H
