#ifndef OLLAMAPROVIDER_H
#define OLLAMAPROVIDER_H


#include <QNetworkAccessManager>

#include "src/providers/base/llmprovider.h"

class OllamaProvider : public LLMProvider
{
    Q_OBJECT
public:
    explicit OllamaProvider(QObject *parent = nullptr);

    QString name() const override { return "Ollama"; }

    void setBaseUrl(const QString &url);
    void setModel(const QString &model);

    void sendChatRequest(const QJsonArray &messages, bool stream = true, const QJsonArray &tools = QJsonArray()) override;
    void sendPrompt(const QString &prompt) override;

private:
    QNetworkAccessManager nam;
    QString baseUrl = "http://localhost:11434";
    QString model = "llama3";
};

#endif // OLLAMAPROVIDER_H
