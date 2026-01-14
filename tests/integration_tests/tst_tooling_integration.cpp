#include <QtTest>
#include <QSignalSpy>
#include "src/llmmanager.h"
#include "src/providers/openai/openaiprovider.h"
#include "src/mcp/mcpserver.h"
#include "src/core/codeeditormanager.h"

// Mock Editor Manager for testing
class MockEditorManager : public CodeEditorManager {
public:
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
        m_provider->setModel("local-model");

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

        m_llmManager->sendChatRequest(prompt);

        // This might take a while as it involves two LLM passes:
        // 1. Model decides to call tool.
        // 2. Manager executes tool and sends result back.
        // 3. Model generates final answer.
        
        // Wait for tool call to start
        QVERIFY(toolStartSpy.wait(30000));
        QCOMPARE(toolStartSpy.first().at(0).toString(), QString("read_file"));
        qDebug() << "Tool call started:" << toolStartSpy.first().at(0).toString();

        // Wait for tool call to finish
        QVERIFY(toolFinishSpy.wait(5000));
        qDebug() << "Tool call finished with result:" << toolFinishSpy.first().at(1).toString();

        // Wait for final response
        QVERIFY(responseSpy.wait(30000));
        
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
