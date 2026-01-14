#include "openaiprovider.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "src/settings/llmsettings.h"

OpenAIProvider::OpenAIProvider(QObject *parent)
    : LLMProvider(parent)
{
    // In a real implementation, we'd pull these from LLMSettings
    // For now, let's just initialize them.
}

void OpenAIProvider::setBaseUrl(const QString &url) { baseUrl = url; }
void OpenAIProvider::setModel(const QString &m) { model = m; }
void OpenAIProvider::setApiKey(const QString &key) { apiKey = key; }

void OpenAIProvider::sendChatRequest(const QJsonArray &messages, bool stream)
{
    QUrl url(baseUrl + "/chat/completions");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!apiKey.isEmpty()) {
        req.setRawHeader("Authorization", "Bearer " + apiKey.toUtf8());
    }

    QJsonObject root;
    root["model"] = model;
    root["messages"] = messages;
    root["stream"] = stream;

    auto reply = nam.post(req, QJsonDocument(root).toJson());

    if (stream) {
        connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
            while (reply->canReadLine()) {
                QByteArray line = reply->readLine().trimmed();
                if (line.startsWith("data: ")) {
                    QByteArray data = line.mid(6);
                    if (data == "[DONE]") {
                        emit streamFinished();
                        return;
                    }

                    QJsonDocument doc = QJsonDocument::fromJson(data);
                    QJsonObject obj = doc.object();
                    QJsonArray choices = obj["choices"].toArray();
                    if (!choices.isEmpty()) {
                        QJsonObject delta = choices[0].toObject()["delta"].toObject();
                        if (delta.contains("content")) {
                            emit partialResponse(delta["content"].toString());
                        }
                        if (delta.contains("tool_calls")) {
                            // TODO: Buffer and emit tool calls when stream finished or delta complete
                        }
                    }
                }
            }
        });
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, stream]{
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            reply->deleteLater();
            return;
        }

        if (!stream) {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            const auto obj = doc.object();
            const auto choices = obj["choices"].toArray();
            if (!choices.isEmpty()) {
                const auto msgObj = choices[0].toObject()["message"].toObject();
                emit responseReady(msgObj["content"].toString());
            } else {
                emit errorOccurred("Empty OpenAI response");
            }
        } else {
            emit streamFinished();
        }
        reply->deleteLater();
    });
}
