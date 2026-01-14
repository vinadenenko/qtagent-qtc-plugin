#include "llmmanager.h"

LLMManager::LLMManager(QObject *parent) : QObject(parent) {}

void LLMManager::setProvider(LLMProvider *provider)
{
    if (current)
        disconnect(current, nullptr, this, nullptr);

    current = provider;

    if (current) {
        connect(current, &LLMProvider::responseReady, this, [this](const QString &text) {
            m_history.addMessage(Message::Assistant, text);
            emit responseReady(text);
        });

        connect(current, &LLMProvider::partialResponse, this, [this](const QString &delta) {
            m_currentAssistantResponse += delta;
            emit partialResponse(delta);
        });

        connect(current, &LLMProvider::streamFinished, this, [this]() {
            if (!m_currentAssistantResponse.isEmpty()) {
                m_history.addMessage(Message::Assistant, m_currentAssistantResponse);
                m_currentAssistantResponse.clear();
            }
            emit streamFinished();
        });

        connect(current, &LLMProvider::toolCallsReceived, this, [this](const QJsonArray &toolCalls) {
            m_history.addMessage(Message::Assistant, m_currentAssistantResponse, QString(), toolCalls);
            m_currentAssistantResponse.clear();
            handleToolCalls(toolCalls);
        });

        connect(current, &LLMProvider::errorOccurred, this, &LLMManager::errorOccurred);
    }
}

void LLMManager::sendPrompt(const QString &prompt)
{
    if (current)
        current->sendPrompt(prompt);
}

void LLMManager::setMCPServer(MCPServer *server)
{
    m_mcpServer = server;
}

void LLMManager::sendChatRequest(const QString &prompt)
{
    if (!current) {
        emit errorOccurred("No LLM provider set");
        return;
    }

    // Add automatic context if history is empty or starting new interaction
    if (m_history.messageCount() == 0) {
        QString systemPrompt = "You are an AI assistant integrated into Qt Creator. "
                               "You have access to the project files and editor through tools.";
        
        if (m_mcpServer) {
            auto context = m_mcpServer->readResource("file://current");
            QJsonArray contents = context["contents"].toArray();
            if (!contents.isEmpty()) {
                QString currentFile = contents[0].toObject()["text"].toString();
                QString uri = contents[0].toObject()["uri"].toString();
                systemPrompt += "\n\nCurrently open file (" + uri + "):\n" + currentFile;
            }
        }
        
        m_history.addMessage(Message::System, systemPrompt);
    }

    if (!prompt.isEmpty()) {
        m_history.addMessage(Message::User, prompt);
    }
    
    // Simple token management - keep it under 32k estimated tokens
    m_history.trim(32000);

    m_currentAssistantResponse.clear();
    
    QJsonArray tools;
    if (m_mcpServer) {
        tools = m_mcpServer->listTools();
    }
    
    current->sendChatRequest(m_history.toJsonArray(), true);
}

void LLMManager::handleToolCalls(const QJsonArray &toolCalls)
{
    if (!m_mcpServer) return;

    for (const auto &callVal : toolCalls) {
        QJsonObject call = callVal.toObject();
        QString name = call["name"].toString();
        QString id = call["id"].toString();
        QJsonObject args = call["arguments"].toObject();

        emit toolCallStarted(name);
        
        QJsonObject result = m_mcpServer->callTool(name, args);
        QString resultStr = QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
        
        emit toolCallFinished(name, resultStr);
        
        m_history.addMessage(Message::Tool, resultStr, id);
    }

    // After all tool calls, request next response from LLM
    current->sendChatRequest(m_history.toJsonArray(), true);
}

void LLMManager::clearHistory()
{
    m_history.clear();
}
