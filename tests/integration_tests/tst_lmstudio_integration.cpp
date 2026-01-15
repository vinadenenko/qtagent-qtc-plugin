#include <QtTest>
#include <QSignalSpy>
#include "src/providers/openai/openaiprovider.h"
#include <QJsonArray>
#include <QJsonObject>

class LmStudioIntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        m_provider = new OpenAIProvider(this);
        m_provider->setBaseUrl("http://192.168.56.1:1234/v1");
        m_provider->setModel("local-model"); // LM-Studio usually ignores this or uses the loaded one
    }

    void testBasicChatRequest()
    {
        QJsonArray messages;
        QJsonObject msg;
        msg["role"] = "user";
        msg["content"] = "Hello, respond with exactly 'PONG'";
        messages.append(msg);

        QSignalSpy responseSpy(m_provider, &OpenAIProvider::responseReady);
        QSignalSpy partialSpy(m_provider, &OpenAIProvider::partialResponse);
        QSignalSpy finishedSpy(m_provider, &OpenAIProvider::streamFinished);
        QSignalSpy errorSpy(m_provider, &OpenAIProvider::errorOccurred);

        m_provider->sendChatRequest(messages, false); // Non-streaming for simplicity in first test

        // Wait for up to 30 seconds for the model to respond
        QVERIFY(responseSpy.wait(30000));
        
        if (!errorSpy.isEmpty()) {
            QFAIL(qPrintable("Error occurred: " + errorSpy.first().at(0).toString()));
        }

        QString response = responseSpy.first().at(0).toString();
        qDebug() << "Received response:" << response;
        QVERIFY(!response.isEmpty());
    }

    void testStreamingChatRequest()
    {
        QJsonArray messages;
        QJsonObject msg;
        msg["role"] = "user";
        msg["content"] = "Count from 1 to 5";
        messages.append(msg);

        QSignalSpy partialSpy(m_provider, &OpenAIProvider::partialResponse);
        QSignalSpy finishedSpy(m_provider, &OpenAIProvider::streamFinished);
        QSignalSpy errorSpy(m_provider, &OpenAIProvider::errorOccurred);

        m_provider->sendChatRequest(messages, true); // Streaming

        // Wait for stream to finish
        QVERIFY(finishedSpy.wait(30000));

        if (!errorSpy.isEmpty()) {
            QFAIL(qPrintable("Error occurred: " + errorSpy.first().at(0).toString()));
        }

        QVERIFY(!partialSpy.isEmpty());
        
        QString fullText;
        for (int i = 0; i < partialSpy.count(); ++i) {
            fullText += partialSpy.at(i).at(0).toString();
        }
        
        qDebug() << "Received streaming response:" << fullText;
        QVERIFY(!fullText.isEmpty());
    }

private:
    OpenAIProvider *m_provider = nullptr;
};

QTEST_MAIN(LmStudioIntegrationTest)
#include "tst_lmstudio_integration.moc"
