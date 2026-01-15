#include <QtTest>
#include "../src/llmmanager.h"
#include "../src/providers/base/llmprovider.h"
#include "../src/mcp/mcpserver.h"
#include "../src/core/codeeditormanager.h"

class MockProvider : public LLMProvider {
public:
    QString name() const override { return "Mock"; }
    void sendChatRequest(const QJsonArray &messages, bool stream = true, const QJsonArray &tools = QJsonArray()) override {
        if (messages.last().toObject()["role"].toString() == "user") {
            // Simulate tool call
            QJsonArray toolCalls;
            QJsonObject call;
            call["name"] = "test_tool";
            call["id"] = "123";
            call["arguments"] = QJsonObject();
            toolCalls.append(call);
            emit toolCallsReceived(toolCalls);
        } else if (messages.last().toObject()["role"].toString() == "tool") {
            // Final response after tool
            emit responseReady("Tool worked");
        }
    }
};

class MockEditor : public CodeEditorManager {
public:
    // MCPServer needs this to list tools, we'll just use the default MCPServer implementation
    // but MCPServer calls callTool which might use editor.
};

class TestLLMManager : public QObject
{
    Q_OBJECT

private slots:
    void testToolLoop() {
        LLMManager manager;
        MockProvider *provider = new MockProvider();
        manager.setProvider(provider);
        
        CodeEditorManager *editor = new CodeEditorManager(); // Real one but won't be called much
        MCPServer *server = new MCPServer(editor);
        manager.setMCPServer(server);
        
        QString finalResponse;
        connect(&manager, &LLMManager::responseReady, [&](const QString &text) {
            finalResponse = text;
        });
        
        manager.sendChatRequest("Run tool");
        
        QTRY_COMPARE_WITH_TIMEOUT(finalResponse, QString("Tool worked"), 2000);
        QVERIFY(manager.history().messageCount() >= 3); // User, ToolCall (Assistant), ToolResult, Final Assistant
    }
};

QTEST_MAIN(TestLLMManager)
#include "tst_llmmanager.moc"
