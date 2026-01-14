#ifndef OPENAIPROVIDER_H
#define OPENAIPROVIDER_H

#include <QNetworkAccessManager>
#include "src/providers/base/llmprovider.h"

class OpenAIProvider : public LLMProvider
{
    Q_OBJECT
public:
    explicit OpenAIProvider(QObject *parent = nullptr);

    QString name() const override { return "OpenAI"; }

    void setBaseUrl(const QString &url);
    void setModel(const QString &model);
    void setApiKey(const QString &key);

    void sendChatRequest(const QJsonArray &messages, bool stream = true) override;

private:
    QNetworkAccessManager nam;
    QString baseUrl = "https://api.openai.com/v1";
    QString model = "gpt-4o";
    QString apiKey;
};

#endif // OPENAIPROVIDER_H
