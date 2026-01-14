#include <QtTest>
#include <QHttpServer>
#include <QHttpServerResponse>
#include "../src/providers/openai/openaiprovider.h"

class TestOpenAIProvider : public QObject
{
    Q_OBJECT

private slots:
    void testStreaming() {
        QHttpServer server;
        server.route("/chat/completions", []() {
            return QHttpServerResponse("data: {\"choices\":[{\"delta\":{\"content\":\"Hello\"}}]}\n\ndata: {\"choices\":[{\"delta\":{\"content\":\" world\"}}]}\n\ndata: [DONE]\n", QHttpServerResponse::StatusCode::Ok);
        });
        
        quint16 port = server.listen(QHostAddress::LocalHost);
        QVERIFY(port != 0);

        OpenAIProvider provider;
        provider.setBaseUrl(QString("http://localhost:%1").arg(port));
        
        QString fullResponse;
        bool finished = false;
        
        connect(&provider, &OpenAIProvider::partialResponse, [&](const QString &delta) {
            fullResponse += delta;
        });
        connect(&provider, &OpenAIProvider::streamFinished, [&]() {
            finished = true;
        });

        QJsonArray messages;
        QJsonObject msg;
        msg["role"] = "user";
        msg["content"] = "test";
        messages.append(msg);

        provider.sendChatRequest(messages, true);

        QTRY_VERIFY_WITH_TIMEOUT(finished, 5000);
        QCOMPARE(fullResponse, QString("Hello world"));
    }

    void testToolCalls() {
        QHttpServer server;
        server.route("/chat/completions", []() {
            return QHttpServerResponse("data: {\"choices\":[{\"delta\":{\"tool_calls\":[{\"name\":\"test_tool\",\"id\":\"1\",\"arguments\":{}}]}}]}\n\ndata: [DONE]\n", QHttpServerResponse::StatusCode::Ok);
        });
        
        quint16 port = server.listen(QHostAddress::LocalHost);
        QVERIFY(port != 0);

        OpenAIProvider provider;
        provider.setBaseUrl(QString("http://localhost:%1").arg(port));
        
        QJsonArray receivedToolCalls;
        bool finished = false;
        
        connect(&provider, &OpenAIProvider::toolCallsReceived, [&](const QJsonArray &toolCalls) {
            receivedToolCalls = toolCalls;
        });
        connect(&provider, &OpenAIProvider::streamFinished, [&]() {
            finished = true;
        });

        QJsonArray messages;
        provider.sendChatRequest(messages, true);

        QTRY_VERIFY_WITH_TIMEOUT(finished, 5000);
        QCOMPARE(receivedToolCalls.size(), 1);
        QCOMPARE(receivedToolCalls[0].toObject()["name"].toString(), QString("test_tool"));
    }
};

QTEST_MAIN(TestOpenAIProvider)
#include "tst_openaiprovider.moc"
