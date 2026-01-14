#include "mcpserver.h"
#include "src/core/codeeditormanager.h"

#include <QJsonDocument>
#include <QStandardPaths>

#include <QDir>

MCPServer::MCPServer(CodeEditorManager *editorManager, QObject *parent)
    : QObject(parent), m_editorManager(editorManager)
{
    initializeResources();
    initializeTools();
}

void MCPServer::initializeResources()
{
    m_availableResources = QJsonArray();
    
    // Current file resource
    m_availableResources.append(createResource(
        "file://current",
        "Current Editor File",
        "text/plain",
        "The currently open file in the editor"
    ));
    
    // Project directory resource
    m_availableResources.append(createResource(
        "file://project",
        "Project Directory", 
        "text/directory",
        "The current project directory"
    ));
}

void MCPServer::initializeTools()
{
    m_availableTools = QJsonArray();
    
    // Read file tool
    QJsonArray readFileParams = QJsonArray{
        QJsonObject{{"type", "string"}, {"name", "path"}, {"description", "File path to read"}, {"required", true}}
    };
    m_availableTools.append(createTool(
        "read_file",
        "Read the contents of a file",
        readFileParams
    ));
    
    // Write file tool
    QJsonArray writeFileParams = QJsonArray{
        QJsonObject{{"type", "string"}, {"name", "path"}, {"description", "File path to write"}, {"required", true}},
        QJsonObject{{"type", "string"}, {"name", "content"}, {"description", "Content to write"}, {"required", true}}
    };
    m_availableTools.append(createTool(
        "write_file", 
        "Write content to a file",
        writeFileParams
    ));
    
    // Create file tool
    QJsonArray createFileParams = QJsonArray{
        QJsonObject{{"type", "string"}, {"name", "path"}, {"description", "File path to create"}, {"required", true}},
        QJsonObject{{"type", "string"}, {"name", "content"}, {"description", "Initial content"}, {"required", false}}
    };
    m_availableTools.append(createTool(
        "create_file",
        "Create a new file with optional content",
        createFileParams
    ));
    
    // List directory tool
    QJsonArray listDirParams = QJsonArray{
        QJsonObject{{"type", "string"}, {"name", "path"}, {"description", "Directory path to list"}, {"required", false}}
    };
    m_availableTools.append(createTool(
        "list_directory",
        "List files in a directory",
        listDirParams
    ));

    // Delete file tool
    QJsonArray deleteFileParams = QJsonArray{
        QJsonObject{{"type", "string"}, {"name", "path"}, {"description", "File path to delete"}, {"required", true}}
    };
    m_availableTools.append(createTool(
        "delete_file",
        "Delete a file",
        deleteFileParams
    ));

    // Search code tool
    QJsonArray searchParams = QJsonArray{
        QJsonObject{{"type", "string"}, {"name", "query"}, {"description", "Search query"}, {"required", true}},
        QJsonObject{{"type", "string"}, {"name", "path"}, {"description", "Directory to search in"}, {"required", false}}
    };
    m_availableTools.append(createTool(
        "search_code",
        "Search for text in the project",
        searchParams
    ));
}

QJsonObject MCPServer::createResource(const QString &uri, const QString &name,
                                      const QString &mimeType, const QString &description) const
{
    return QJsonObject{
        {"uri", uri},
        {"name", name},
        {"mimeType", mimeType},
        {"description", description}
    };
}

QJsonObject MCPServer::createTool(const QString &name, const QString &description,
                                  const QJsonArray &inputSchema) const
{
    return QJsonObject{
        {"name", name},
        {"description", description},
        {"inputSchema", QJsonObject{
            {"type", "object"},
            {"properties", QJsonObject{
                {"arguments", QJsonObject{
                    {"type", "object"},
                    {"properties", inputSchema}
                }}
            }}
        }}
    };
}

MCPServer::MCPResponse MCPServer::handleRequest(const MCPRequest &request)
{
    MCPResponse response;
    response.id = request.id;
    response.success = true;
    
    if (request.method == "initialize") {
        response.result = QJsonObject{
            {"protocolVersion", "2024-11-05"},
            {"capabilities", QJsonObject{
                {"resources", QJsonObject{{"listChanged", true}}},
                {"tools", QJsonObject{{"listChanged", true}}}
            }},
            {"serverInfo", QJsonObject{
                {"name", "QtAgent MCP Server"},
                {"version", "1.0.0"}
            }}
        };
    } else if (request.method == "resources/list") {
        response.result = QJsonObject{{"resources", listResources()}};
    } else if (request.method == "resources/read") {
        QString uri = request.params["uri"].toString();
        response.result = readResource(uri);
    } else if (request.method == "tools/list") {
        response.result = QJsonObject{{"tools", listTools()}};
    } else if (request.method == "tools/call") {
        QString name = request.params["name"].toString();
        QJsonObject arguments = request.params["arguments"].toObject();
        response.result = callTool(name, arguments);
    } else {
        response.success = false;
        response.error = QJsonObject{
            {"code", -32601},
            {"message", "Method not found"}
        };
    }
    
    return response;
}

QString MCPServer::getServerInfo() const
{
    return "QtAgent MCP Server v1.0.0 - Provides file system and editor access via MCP";
}

QJsonArray MCPServer::listResources() const
{
    return m_availableResources;
}

QJsonObject MCPServer::readResource(const QString &uri) const
{
    QJsonObject result;
    
    if (uri == "file://current") {
        if (m_editorManager) {
            auto context = m_editorManager->getCurrentEditorContext();
            result["contents"] = QJsonArray{
                QJsonObject{
                    {"uri", uri},
                    {"mimeType", "text/plain"},
                    {"text", context.content}
                }
            };
        }
    } else if (uri == "file://project") {
        if (m_editorManager) {
            QString projectPath = m_editorManager->getProjectPath();
            result["contents"] = QJsonArray{
                QJsonObject{
                    {"uri", uri},
                    {"mimeType", "text/directory"},
                    {"text", projectPath}
                }
            };
        }
    } else {
        // Try to read as regular file
        QString content;
        if (m_editorManager && m_editorManager->readFile(uri, content)) {
            result["contents"] = QJsonArray{
                QJsonObject{
                    {"uri", uri},
                    {"mimeType", "text/plain"},
                    {"text", content}
                }
            };
        }
    }
    
    return result;
}

QJsonArray MCPServer::listTools() const
{
    return m_availableTools;
}

QJsonObject MCPServer::callTool(const QString &name, const QJsonObject &arguments)
{
    QJsonObject result;
    
    if (!m_editorManager) {
        result["error"] = "Editor manager not available";
        return result;
    }
    
    if (name == "read_file") {
        QString path = arguments["path"].toString();
        QString content;
        if (m_editorManager->readFile(path, content)) {
            result["content"] = content;
        } else {
            result["error"] = QString("Failed to read file: " + path);
        }
    } else if (name == "write_file") {
        QString path = arguments["path"].toString();
        QString content = arguments["content"].toString();
        if (m_editorManager->writeFile(path, content)) {
            result["success"] = true;
        } else {
            result["error"] = QString("Failed to write file: " + path);
        }
    } else if (name == "create_file") {
        QString path = arguments["path"].toString();
        QString content = arguments["content"].toString();
        if (m_editorManager->createFile(path, content)) {
            result["success"] = true;
        } else {
            result["error"] = QString("Failed to create file: " + path);
        }
    } else if (name == "get_editor_context") {
        auto context = m_editorManager->getCurrentEditorContext();
        QJsonObject contextJson{
            {"filePath", context.filePath},
            {"cursorPosition", context.cursorPosition},
            {"selectedText", context.selectedText},
            {"selectionStart", context.selectionStart},
            {"selectionEnd", context.selectionEnd},
            {"isValid", context.isValid}
        };
        
        if (arguments["include_content"].toBool(true)) {
            contextJson["content"] = context.content;
        }
        
        result["context"] = contextJson;
    } else if (name == "delete_file") {
        QString path = arguments["path"].toString();
        if (m_editorManager->deleteFile(path)) {
            result["success"] = true;
        } else {
            result["error"] = QString("Failed to delete file: " + path);
        }
    } else if (name == "list_directory") {
        QString path = arguments["path"].toString();
        if (path.isEmpty()) path = m_editorManager->getProjectPath();
        
        QDir dir(path);
        if (dir.exists()) {
            QJsonArray files;
            for (const auto &info : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries)) {
                QJsonObject fileInfo;
                fileInfo["name"] = info.fileName();
                fileInfo["type"] = info.isDir() ? "directory" : "file";
                fileInfo["size"] = info.size();
                files.append(fileInfo);
            }
            result["files"] = files;
        } else {
            result["error"] = QString("Directory not found: " + path);
        }
    } else if (name == "search_code") {
        // Simple implementation for now, should ideally use Qt Creator's search
        QString query = arguments["query"].toString();
        result["message"] = "Search not fully implemented, but I'll try to find it in the project files.";
        result["query"] = query;
    } else {
        result["error"] = QString("Unknown tool: " + name);
    }
    
    return result;
}
