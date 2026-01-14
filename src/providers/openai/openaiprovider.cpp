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

void OpenAIProvider::sendChatRequest(const QJsonArray &messages, bool stream, const QJsonArray &tools)
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
    
    if (!tools.isEmpty()) {
        root["tools"] = tools;
    }

    auto reply = nam.post(req, QJsonDocument(root).toJson());

    if (stream) {
        connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
            while (reply->canReadLine()) {
                QByteArray line = reply->readLine().trimmed();
                if (line.startsWith("data: ")) {
                    QByteArray data = line.mid(6);
                    if (data == "[DONE]") {
                        return;
                    }

                    QJsonDocument doc = QJsonDocument::fromJson(data);
                    if (doc.isNull()) continue;

                    QJsonObject obj = doc.object();
                    QJsonArray choices = obj["choices"].toArray();
                    if (!choices.isEmpty()) {
                        QJsonObject delta = choices[0].toObject()["delta"].toObject();
                        if (delta.contains("content")) {
                            QString content = delta["content"].toString();
                            if (!content.isEmpty()) {
                                emit partialResponse(content);
                            }
                        }
                        
                        // Some providers use 'reasoning_content' field (e.g. DeepSeek)
                        if (delta.contains("reasoning_content")) {
                            QString reasoning = delta["reasoning_content"].toString();
                            if (!reasoning.isEmpty()) {
                                // For now, we emit it via partialResponse but we might want a separate signal
                                // Or we can wrap it in markers
                                emit partialResponse("<thought>\n" + reasoning + "\n</thought>\n");
                            }
                        }

                        if (delta.contains("tool_calls")) {
                            QJsonArray toolCalls = delta["tool_calls"].toArray();
                            for (const auto &tcVal : toolCalls) {
                                QJsonObject tc = tcVal.toObject();
                                // LM-Studio and some others might omit index if there's only one tool call
                                int index = 0;
                                if (tc.contains("index")) {
                                    index = tc["index"].toInt();
                                }
                                
                                if (!m_ongoingToolCalls.contains(index)) {
                                    m_ongoingToolCalls[index] = tc;
                                } else {
                                    QJsonObject existing = m_ongoingToolCalls[index];
                                    if (tc.contains("function")) {
                                        QJsonObject existingFunc = existing["function"].toObject();
                                        QJsonObject newFunc = tc["function"].toObject();
                                        
                                        if (newFunc.contains("arguments")) {
                                            QString args = existingFunc["arguments"].toString();
                                            args += newFunc["arguments"].toString();
                                            existingFunc["arguments"] = args;
                                        }
                                        if (newFunc.contains("name")) {
                                            QString name = existingFunc["name"].toString();
                                            name += newFunc["name"].toString();
                                            existingFunc["name"] = name;
                                        }
                                        existing["function"] = existingFunc;
                                    }
                                    if (tc.contains("id")) {
                                        existing["id"] = tc["id"];
                                    }
                                    if (tc.contains("type")) {
                                        existing["type"] = tc["type"];
                                    }
                                    m_ongoingToolCalls[index] = existing;
                                }
                            }
                        }
                    }
                }
            }
        });
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, stream]{
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            m_ongoingToolCalls.clear();
            reply->deleteLater();
            return;
        }

        if (!m_ongoingToolCalls.isEmpty()) {
            QJsonArray finalToolCalls;
            QList<int> keys = m_ongoingToolCalls.keys();
            std::sort(keys.begin(), keys.end());
            for (int key : keys) {
                finalToolCalls.append(m_ongoingToolCalls[key]);
            }
            emit toolCallsReceived(finalToolCalls);
            m_ongoingToolCalls.clear();
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
