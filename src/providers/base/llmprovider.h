#ifndef LLMPROVIDER_H
#define LLMPROVIDER_H

#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>

class LLMProvider : public QObject
{
    Q_OBJECT
public:
    explicit LLMProvider(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~LLMProvider() = default;

    virtual QString name() const = 0;

    // Standardized interface for chat completions
    virtual void sendChatRequest(const QJsonArray &messages, bool stream = true) = 0;

    // Legacy method, can be implemented in terms of sendChatRequest if needed
    virtual void sendPrompt(const QString &prompt) {
        QJsonArray messages;
        QJsonObject msg;
        msg["role"] = "user";
        msg["content"] = prompt;
        messages.append(msg);
        sendChatRequest(messages, false);
    }

signals:
    void responseReady(const QString &text);
    void partialResponse(const QString &delta);
    void streamFinished();
    void errorOccurred(const QString &error);
    void toolCallsReceived(const QJsonArray &toolCalls);
};

#endif // LLMPROVIDER_H
