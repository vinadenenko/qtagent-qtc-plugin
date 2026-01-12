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

void OllamaProvider::sendPrompt(const QString &prompt)
{
    QUrl url(baseUrl + "/v1/chat/completions");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject msg;
    msg["role"] = "user";
    msg["content"] = prompt;

    QJsonArray messages;
    messages.append(msg);

    QJsonObject root;
    root["model"] = model;
    root["messages"] = messages;
    root["stream"] = false;

    auto reply = nam.post(req, QJsonDocument(root).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]{
        if (reply->error() != QNetworkReply::NoError)
        {
            emit errorOccurred(reply->errorString());
            reply->deleteLater();
            return;
        }

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

        reply->deleteLater();
    });
}
