#ifndef LLMMANAGER_H
#define LLMMANAGER_H

#include <QObject>
#include "src/providers/base/llmprovider.h"
#include "src/core/conversationhistory.h"
#include "src/mcp/mcpserver.h"

class LLMManager : public QObject
{
    Q_OBJECT
public:
    explicit LLMManager(QObject *parent = nullptr);

    void setProvider(LLMProvider *provider);
    void setMCPServer(MCPServer *server);
    void sendPrompt(const QString &prompt);
    void sendChatRequest(const QString &prompt);

    ConversationHistory& history() { return m_history; }
    void clearHistory();

signals:
    void responseReady(const QString &text);
    void partialResponse(const QString &delta);
    void streamFinished();
    void errorOccurred(const QString &error);
    void toolCallStarted(const QString &name);
    void toolCallFinished(const QString &name, const QString &result);
    void modelInfoReceived(const QString &actualModel);
    void historyUpdated();

private:
    void handleToolCalls(const QJsonArray &toolCalls);

    LLMProvider *current = nullptr;
    MCPServer *m_mcpServer = nullptr;
    ConversationHistory m_history;
    QString m_currentAssistantResponse;
};
#endif // LLMMANAGER_H
