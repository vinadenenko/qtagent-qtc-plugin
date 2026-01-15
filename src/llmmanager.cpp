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
            // If we get a partial response, it means we are receiving content from the assistant.
            m_currentAssistantResponse += delta;
            emit partialResponse(delta);
        });

        connect(current, &LLMProvider::streamFinished, this, [this]() {
            if (!m_currentAssistantResponse.isEmpty()) {
                m_history.addMessage(Message::Assistant, m_currentAssistantResponse);
                emit responseReady(m_currentAssistantResponse);
                m_currentAssistantResponse.clear();
            }
            emit streamFinished();
        });

        connect(current, &LLMProvider::toolCallsReceived, this, [this](const QJsonArray &toolCalls) {
            // Check if there's actual content or just tool calls
            QString content = m_currentAssistantResponse;
            
            // If the model output a lot of "thought" or content before calling tools,
            // we should probably store it.
            m_history.addMessage(Message::Assistant, content, QString(), toolCalls);
            
            // We emitted content via partialResponse, so the UI should already show it.
            
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
                               "You have access to the project files and editor through tools. "
                               "When you need to read, write, or list files, ALWAYS use the provided tools. "
                               "Do NOT guess or assume the content of files you have not read yet. "
                               "Wait for the tool output before providing information based on a file.";
        
        if (m_mcpServer) {
            QString projectPath;
            auto projectContext = m_mcpServer->readResource("file://project");
            QJsonArray projectContents = projectContext["contents"].toArray();
            if (!projectContents.isEmpty()) {
                projectPath = projectContents[0].toObject()["text"].toString();
                if (!projectPath.isEmpty()) {
                    systemPrompt += "\n\nProject root: " + projectPath;
                    
                    // List top-level directory to give the AI an idea of the project structure
                    QJsonObject listResult = m_mcpServer->callTool("list_directory", {{"path", projectPath}});
                    if (listResult.contains("files")) {
                        systemPrompt += "\n\nProject structure (root):\n";
                        QJsonArray files = listResult["files"].toArray();
                        for (const auto &fileVal : files) {
                            QJsonObject fileObj = fileVal.toObject();
                            systemPrompt += "- " + fileObj["name"].toString() + " (" + fileObj["type"].toString() + ")\n";
                        }
                    }
                }
            }

            auto context = m_mcpServer->readResource("file://current");
            QJsonArray contents = context["contents"].toArray();
            if (!contents.isEmpty()) {
                QString currentFile = contents[0].toObject()["text"].toString();
                QString uri = contents[0].toObject()["uri"].toString();
                // ONLY add current file context if it's NOT empty. 
                // In some tests, we might want to start with a clean slate.
                if (!currentFile.isEmpty()) {
                    systemPrompt += "\n\nCurrently open file (" + uri + "):\n" + currentFile;
                }
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
    
    current->sendChatRequest(m_history.toJsonArray(), true, tools);
}

void LLMManager::handleToolCalls(const QJsonArray &toolCalls)
{
    if (!m_mcpServer) return;

    for (const auto &callVal : toolCalls) {
        QJsonObject call = callVal.toObject();
        
        // Handle both direct tool call and OpenAI-style nested function call
        QString name;
        QJsonObject args;
        QString id = call["id"].toString();

        if (call.contains("function")) {
            QJsonObject function = call["function"].toObject();
            name = function["name"].toString();
            QString argsStr = function["arguments"].toString();
            args = QJsonDocument::fromJson(argsStr.toUtf8()).object();
        } else {
            name = call["name"].toString();
            args = call["arguments"].toObject();
        }

        if (name.isEmpty()) continue;

        emit toolCallStarted(name);
        
        QJsonObject result = m_mcpServer->callTool(name, args);
        QString resultStr = QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
        
        emit toolCallFinished(name, resultStr);
        
        m_history.addMessage(Message::Tool, resultStr, id);
    }

    // After all tool calls, request next response from LLM
    // We clear currentAssistantResponse to ensure we don't carry over content from the tool-deciding turn
    m_currentAssistantResponse.clear();
    current->sendChatRequest(m_history.toJsonArray(), true);
}

void LLMManager::clearHistory()
{
    m_history.clear();
}
