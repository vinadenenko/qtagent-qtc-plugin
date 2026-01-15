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
    QString fullUrl = baseUrl;
    
    // Most OpenAI-compatible providers expect /v1/chat/completions
    // If the user provided http://host:port, we should check if they missed /v1
    if (!fullUrl.contains("/v1") && !fullUrl.contains("/chat/completions")) {
        if (!fullUrl.endsWith("/")) fullUrl += "/";
        fullUrl += "v1";
    }

    if (!fullUrl.endsWith("/chat/completions")) {
        if (!fullUrl.endsWith("/")) fullUrl += "/";
        fullUrl += "chat/completions";
    }
    QUrl url(fullUrl);
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
                    if (data == "[DONE]") continue;

                    QJsonDocument doc = QJsonDocument::fromJson(data);
                    if (doc.isNull()) {
                        if (!data.isEmpty()) {
                            emit partialResponse("\n**System Notification:** " + QString::fromUtf8(data) + "\n");
                        }
                        continue;
                    }

                    QJsonObject obj = doc.object();
                    if (obj.contains("model")) {
                        QString reportedModel = obj["model"].toString();
                        if (reportedModel != m_lastReportedModel) {
                            m_lastReportedModel = reportedModel;
                            emit modelInfoReceived(reportedModel);
                        }
                    }
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
                                emit partialResponse("<thought>\n" + reasoning + "\n</thought>\n");
                            }
                        }

                        if (delta.contains("tool_calls")) {
                            QJsonArray toolCalls = delta["tool_calls"].toArray();
                            for (const auto &tcVal : toolCalls) {
                                QJsonObject tc = tcVal.toObject();
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

            if (!sseFound && !buffer.isEmpty()) {
                // If we got data but none of it looked like SSE, it might be a raw error message
                // even if status code was 200.
                if (buffer.contains("Unexpected endpoint") || buffer.contains("error")) {
                     emit partialResponse("\n**System Notification:** " + QString::fromUtf8(buffer).trimmed() + "\n");
                }
            }
        });
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, stream]{
        if (reply->error() != QNetworkReply::NoError) {
            QByteArray errorData = reply->readAll();
            QString errorMsg = reply->errorString();
            if (!errorData.isEmpty()) {
                errorMsg += " - " + QString::fromUtf8(errorData);
            }
            emit errorOccurred(errorMsg);
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
            if (obj.contains("model")) {
                QString reportedModel = obj["model"].toString();
                if (reportedModel != m_lastReportedModel) {
                    m_lastReportedModel = reportedModel;
                    emit modelInfoReceived(reportedModel);
                }
            }
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
