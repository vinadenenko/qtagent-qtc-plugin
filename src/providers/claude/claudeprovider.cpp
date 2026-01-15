#include "claudeprovider.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ClaudeProvider::ClaudeProvider(QObject *parent)
    : LLMProvider(parent)
{
}

void ClaudeProvider::setBaseUrl(const QString &url) { baseUrl = url; }
void ClaudeProvider::setModel(const QString &m) { model = m; }
void ClaudeProvider::setApiKey(const QString &key) { apiKey = key; }

void ClaudeProvider::sendChatRequest(const QJsonArray &messages, bool stream, const QJsonArray &tools)
{
    QString fullUrl = baseUrl;
    if (!fullUrl.endsWith("/messages")) {
        if (!fullUrl.endsWith("/")) fullUrl += "/";
        fullUrl += "messages";
    }
    QUrl url(fullUrl);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("x-api-key", apiKey.toUtf8());
    req.setRawHeader("anthropic-version", "2023-06-01");

    QJsonObject root;
    root["model"] = model;
    
    // Anthropic handles system prompt separately
    QJsonArray anthropicMessages;
    QString systemPrompt;
    for (const auto &mVal : messages) {
        QJsonObject m = mVal.toObject();
        if (m["role"] == "system") {
            systemPrompt = m["content"].toString();
        } else {
            anthropicMessages.append(m);
        }
    }
    
    if (!systemPrompt.isEmpty()) {
        root["system"] = systemPrompt;
    }
    root["messages"] = anthropicMessages;
    root["stream"] = stream;
    root["max_tokens"] = 4096;
    
    if (!tools.isEmpty()) {
        // Claude tools conversion would go here if needed
        // root["tools"] = tools; 
    }

    auto reply = nam.post(req, QJsonDocument(root).toJson());

    if (stream) {
        connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
            while (reply->canReadLine()) {
                QByteArray line = reply->readLine().trimmed();
                if (line.startsWith("data: ")) {
                    QByteArray data = line.mid(6);
                    QJsonDocument doc = QJsonDocument::fromJson(data);
                    QJsonObject obj = doc.object();
                    QString type = obj["type"].toString();
                    
                    if (type == "content_block_delta") {
                        QJsonObject delta = obj["delta"].toObject();
                        if (delta["type"] == "text_delta") {
                            emit partialResponse(delta["text"].toString());
                        }
                    } else if (type == "message_stop") {
                        emit streamFinished();
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
            const auto content = obj["content"].toArray();
            if (!content.isEmpty()) {
                const auto textObj = content[0].toObject();
                emit responseReady(textObj["text"].toString());
            } else {
                emit errorOccurred("Empty Claude response");
            }
        } else {
            emit streamFinished();
        }
        reply->deleteLater();
    });
}
