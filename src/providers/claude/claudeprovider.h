#ifndef CLAUDEPROVIDER_H
#define CLAUDEPROVIDER_H

#include <QNetworkAccessManager>
#include "src/providers/base/llmprovider.h"

class ClaudeProvider : public LLMProvider
{
    Q_OBJECT
public:
    explicit ClaudeProvider(QObject *parent = nullptr);

    QString name() const override { return "Claude"; }

    void setBaseUrl(const QString &url);
    void setModel(const QString &model);
    void setApiKey(const QString &key);

    void sendChatRequest(const QJsonArray &messages, bool stream = true, const QJsonArray &tools = QJsonArray()) override;

private:
    QNetworkAccessManager nam;
    QString baseUrl = "https://api.anthropic.com/v1";
    QString model = "claude-3-5-sonnet-20240620";
    QString apiKey;
};

#endif // CLAUDEPROVIDER_H
