#include <QtTest>
#include <QSignalSpy>
#include "src/llmmanager.h"
#include "src/providers/openai/openaiprovider.h"
#include "src/mcp/mcpserver.h"
#include "src/core/codeeditormanager.h"

// Mock Editor Manager for testing
class MockEditorManager : public CodeEditorManager {
public:
    EditorContext getCurrentEditorContext() const override {
        EditorContext ctx;
        ctx.filePath = "untitled.txt";
        ctx.content = ""; // Leave it empty so model doesn't see the secret code immediately
        ctx.isValid = true;
        return ctx;
    }

    bool readFile(const QString &filePath, QString &content) override {
        if (filePath == "test.txt") {
            content = "This is a secret code: 12345";
            return true;
        }
        return false;
    }
};

class ToolingIntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        m_provider = new OpenAIProvider(this);
        m_provider->setBaseUrl("http://192.168.56.1:1234/v1");
        // m_provider->setModel("local-model"); // Use whatever is configured or a generic name
        m_provider->setModel("qwen3-8b-deepseek-v3.2-speciale-distill"); // As per user logs

        m_mockEditor = new MockEditorManager();
        m_mcpServer = new MCPServer(m_mockEditor, this);

        m_llmManager = new LLMManager(this);
        m_llmManager->setProvider(m_provider);
        m_llmManager->setMCPServer(m_mcpServer);


    }

    void testToolCallingLoop()
    {
        // We ask the model to read a specific file and tell us what's inside.
        // This should trigger the 'read_file' tool.
        QString prompt = "Please read the file 'test.txt' and tell me the secret code found there.";

        QSignalSpy toolStartSpy(m_llmManager, &LLMManager::toolCallStarted);
        QSignalSpy toolFinishSpy(m_llmManager, &LLMManager::toolCallFinished);
        QSignalSpy responseSpy(m_llmManager, &LLMManager::responseReady);
        QSignalSpy errorSpy(m_llmManager, &LLMManager::errorOccurred);

        QSignalSpy partialSpy(m_provider, &OpenAIProvider::partialResponse);
        QSignalSpy finishedSpy(m_provider, &OpenAIProvider::streamFinished);

        // connect(m_provider, &OpenAIProvider::partialResponse, [](const QString &text){
        //     qDebug() << "Received streaming response:" << text;
        // });
        // connect(m_provider, &OpenAIProvider::responseReady, [](const QString &text){
        //     qDebug() << "Received 'unary' response:" << text;
        // });

        m_llmManager->sendChatRequest(prompt);


        QTimer *timer = new QTimer();
        timer->setInterval(25000);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, this, [&partialSpy](){
            QString fullText;
            for (int i = 0; i < partialSpy.count(); ++i) {
                fullText += partialSpy.at(i).at(0).toString();
            }

            qDebug() << "Received streaming response:" << fullText;
        });
        timer->start();

        // This might take a while as it involves two LLM passes:
        // 1. Model decides to call tool.
        // 2. Manager executes tool and sends result back.
        // 3. Model generates final answer.
        
        // Wait for tool call to start
        if (toolStartSpy.isEmpty()) {
            QVERIFY2(toolStartSpy.wait(30000), "Timeout waiting for toolCallStarted");
        }
        QCOMPARE(toolStartSpy.first().at(0).toString(), QString("read_file"));
        qDebug() << "Tool call started:" << toolStartSpy.first().at(0).toString();

        // Wait for tool call to finish
        if (toolFinishSpy.isEmpty()) {
            QVERIFY2(toolFinishSpy.wait(30000), "Timeout waiting for toolCallFinished");
        }
        qDebug() << "Tool call finished with result:" << toolFinishSpy.first().at(1).toString();

        // Wait for final response
        if (responseSpy.isEmpty()) {
            QVERIFY2(responseSpy.wait(30000), "Timeout waiting for responseReady");
        }
        
        if (!errorSpy.isEmpty()) {
            QFAIL(qPrintable("Error occurred: " + errorSpy.first().at(0).toString()));
        }

        QString finalResponse = responseSpy.first().at(0).toString();
        qDebug() << "Final response from model:" << finalResponse;
        
        // Verify the model actually saw the content from the tool
        QVERIFY(finalResponse.contains("12345"));
    }

private:
    OpenAIProvider *m_provider = nullptr;
    MockEditorManager *m_mockEditor = nullptr;
    MCPServer *m_mcpServer = nullptr;
    LLMManager *m_llmManager = nullptr;
};

QTEST_MAIN(ToolingIntegrationTest)
#include "tst_tooling_integration.moc"
