#ifndef MCPSERVER_H
#define MCPSERVER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// class CodeEditorManager;

#include "src/core/codeeditormanager.h"

class MCPServer : public QObject
{
    Q_OBJECT

public:
    explicit MCPServer(CodeEditorManager *editorManager, QObject *parent = nullptr);
    
    struct MCPRequest {
        QString method;
        QJsonObject params;
        QString id;
    };
    
    struct MCPResponse {
        QJsonObject result;
        QJsonObject error;
        QString id;
        bool success;
    };
    
    MCPResponse handleRequest(const MCPRequest &request);
    QString getServerInfo() const;
    
    // MCP Resource methods
    QJsonArray listResources() const;
    QJsonObject readResource(const QString &uri) const;
    
    // MCP Tool methods  
    QJsonArray listTools() const;
    QJsonObject callTool(const QString &name, const QJsonObject &arguments);

private:
    void initializeResources();
    void initializeTools();
    
    QJsonObject createResource(const QString &uri, const QString &name, 
                              const QString &mimeType, const QString &description) const;
    QJsonObject createTool(const QString &name, const QString &description,
                          const QJsonArray &inputSchema) const;
    
    CodeEditorManager *m_editorManager;
    QJsonArray m_availableResources;
    QJsonArray m_availableTools;
};

#endif // MCPSERVER_H
