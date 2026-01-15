#include "ollamaprovider.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "src/settings/llmsettings.h"

OllamaProvider::OllamaProvider(QObject *parent)
    : LLMProvider(parent)
{
    auto &s = LLMSettings::instance();
    baseUrl = s.baseUrl();
    model = s.model();
}

void OllamaProvider::setBaseUrl(const QString &url)
{
    baseUrl = url;
}

void OllamaProvider::setModel(const QString &m)
{
    model = m;
}

void OllamaProvider::sendChatRequest(const QJsonArray &messages, bool stream, const QJsonArray &tools)
{
    QString fullUrl = baseUrl;
    
    // Check if user missed /v1 or /api/chat
    if (!fullUrl.contains("/v1") && !fullUrl.contains("/api/chat")) {
        if (!fullUrl.endsWith("/")) fullUrl += "/";
        fullUrl += "v1";
    }

    if (!fullUrl.endsWith("/chat/completions") && !fullUrl.endsWith("/api/chat")) {
        // Default to OpenAI-compatible endpoint if not specified
        if (!fullUrl.endsWith("/")) fullUrl += "/";
        fullUrl += "chat/completions";
    }
    QUrl url(fullUrl);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject root;
    root["model"] = model;
    root["messages"] = messages;
    root["stream"] = stream;
    
    if (!tools.isEmpty()) {
        root["tools"] = tools;
    }

    auto reply = nam.post(req, QJsonDocument(root).toJson());

    if (stream) {
        connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
            if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() >= 400) return;
            
            bool sseFound = false;
            QByteArray buffer = reply->readAll();
            QList<QByteArray> lines = buffer.split('\n');

            for (QByteArray line : lines) {
                line = line.trimmed();
                if (line.isEmpty()) continue;

                if (line.startsWith("data: ")) {
                    sseFound = true;
                    QByteArray data = line.mid(6).trimmed();
                    if (data == "[DONE]") {
                        emit streamFinished();
                        return;
                    }

                    QJsonDocument doc = QJsonDocument::fromJson(data);
                    if (doc.isNull()) {
                        if (!data.isEmpty()) {
                            emit partialResponse("\n**System Notification:** " + QString::fromUtf8(data) + "\n");
                        }
                        continue;
                    }
                    QJsonObject obj = doc.object();
                    QJsonArray choices = obj["choices"].toArray();
                    if (!choices.isEmpty()) {
                        QJsonObject delta = choices[0].toObject()["delta"].toObject();
                        if (delta.contains("content")) {
                            emit partialResponse(delta["content"].toString());
                        }
                    }
                }
            }

            if (!sseFound && !buffer.isEmpty()) {
                if (buffer.contains("Unexpected endpoint") || buffer.contains("error")) {
                     emit partialResponse("\n**System Notification:** " + QString::fromUtf8(buffer).trimmed() + "\n");
                }
            }
        });
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, stream]{
        if (reply->error() != QNetworkReply::NoError)
        {
            QByteArray errorData = reply->readAll();
            QString errorMsg = reply->errorString();
            if (!errorData.isEmpty()) {
                errorMsg += " - " + QString::fromUtf8(errorData);
            }
            emit errorOccurred(errorMsg);
            reply->deleteLater();
            return;
        }

        if (!stream) {
            const auto data = reply->readAll();
            const auto doc = QJsonDocument::fromJson(data);
            const auto obj = doc.object();
            const auto choices = obj["choices"].toArray();
            if (!choices.isEmpty())
            {
                const auto msgObj = choices[0].toObject()["message"].toObject();
                emit responseReady(msgObj["content"].toString());
            }
            else
            {
                emit errorOccurred("Empty LLM response");
            }
        } else {
            emit streamFinished();
        }

        reply->deleteLater();
    });
}

void OllamaProvider::sendPrompt(const QString &prompt)
{
    LLMProvider::sendPrompt(prompt);
}
