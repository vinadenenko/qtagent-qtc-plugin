#include <QtTest>
#include "../src/mcp/mcpserver.h"
#include "../src/core/codeeditormanager.h"

class MockEditorManager : public CodeEditorManager {
public:
    EditorContext getCurrentEditorContext() const override {
        EditorContext ctx;
        ctx.filePath = "test.cpp";
        ctx.content = "void main() {}";
        ctx.isValid = true;
        return ctx;
    }
    bool readFile(const QString &path, QString &content) override {
        if (path == "test.cpp") {
            content = "void main() {}";
            return true;
        }
        return false;
    }
    bool writeFile(const QString &path, const QString &content) override {
        return true;
    }
};

class TestMCPServer : public QObject
{
    Q_OBJECT

private slots:
    void testListTools() {
        MockEditorManager mock;
        MCPServer server(&mock);
        QJsonArray tools = server.listTools();
        QVERIFY(tools.size() > 0);
        
        bool foundRead = false;
        for (const auto &t : tools) {
            if (t.toObject()["name"].toString() == "read_file") foundRead = true;
        }
        QVERIFY(foundRead);
    }

    void testCallToolRead() {
        MockEditorManager mock;
        MCPServer server(&mock);
        
        QJsonObject args;
        args["path"] = "test.cpp";
        
        QJsonObject result = server.callTool("read_file", args);
        QCOMPARE(result["content"].toString(), QString("void main() {}"));
    }

    void testHandleRequest() {
        MockEditorManager mock;
        MCPServer server(&mock);
        
        MCPServer::MCPRequest req;
        req.method = "tools/list";
        req.id = "1";
        
        MCPServer::MCPResponse resp = server.handleRequest(req);
        QVERIFY(resp.success);
        QCOMPARE(resp.id, QString("1"));
        QVERIFY(resp.result.contains("tools"));
    }
};

QTEST_MAIN(TestMCPServer)
#include "tst_mcpserver.moc"
