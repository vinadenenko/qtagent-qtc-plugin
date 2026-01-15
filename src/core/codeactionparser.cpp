#include "codeactionparser.h"

CodeActionParser::CodeActionParser(QObject *parent)
    : QObject(parent)
{
    initializePatterns();
}

void CodeActionParser::initializePatterns()
{
    m_patterns.clear();
    
    // File creation patterns
    m_patterns << QRegularExpression(R"(create\s+(?:file\s+)?["`']?([^'"`\s]+)["`']?\s+(?:with\s+content|:)\s*```(?:\w+)?\n(.*?)\n```)",
                                          QRegularExpression::DotMatchesEverythingOption);
    
    m_patterns << QRegularExpression(R"(Create\s+file\s+["`']?([^'"`\s]+)["`']?\s*:\s*```(?:\w+)?\n(.*?)\n```)",
                                          QRegularExpression::DotMatchesEverythingOption);
    
    // File update patterns  
    m_patterns << QRegularExpression(R"(update\s+(?:file\s+)?["`']?([^'"`\s]+)["`']?\s+(?:with\s+content|:)\s*```(?:\w+)?\n(.*?)\n```)",
                                          QRegularExpression::DotMatchesEverythingOption);
    
    m_patterns << QRegularExpression(R"(Replace\s+content\s+of\s+["`']?([^'"`\s]+)["`']?\s+(?:with|:)\s*```(?:\w+)?\n(.*?)\n```)",
                                          QRegularExpression::DotMatchesEverythingOption);
    
    // File deletion patterns
    m_patterns << QRegularExpression(R"(delete\s+(?:file\s+)?["`']?([^'"`\s]+)["`']?)");
    m_patterns << QRegularExpression(R"(remove\s+(?:file\s+)?["`']?([^'"`\s]+)["`']?)");
    
    // Code insertion patterns
    m_patterns << QRegularExpression(R"(insert\s+(?:code|snippet)\s*:\s*```(?:\w+)?\n(.*?)\n```)",
                                          QRegularExpression::DotMatchesEverythingOption);
    
    m_patterns << QRegularExpression(R"(add\s+(?:code|snippet)\s+(?:at|to)\s+(?:line\s+)?(\d+)\s*:\s*```(?:\w+)?\n(.*?)\n```)",
                                          QRegularExpression::DotMatchesEverythingOption);
    
    // Code replacement patterns
    m_patterns << QRegularExpression(R"(replace\s+(?:selected\s+)?code\s*:\s*```(?:\w+)?\n(.*?)\n```)",
                                          QRegularExpression::DotMatchesEverythingOption);
    
    // File path detection
    m_patterns << QRegularExpression(R"(["`']?([^'"`\s]+\.(cpp|h|hpp|c|cc|cxx|py|js|ts|html|css|json|xml|yaml|yml|md|txt|cmake|pro|pri))["`']?)");
}

QList<CodeAction> CodeActionParser::parseActions(const QString &text) const
{
    QList<CodeAction> actions;
    
    for (const auto &pattern : m_patterns) {
        auto matches = pattern.globalMatch(text);
        while (matches.hasNext()) {
            auto match = matches.next();
            
            CodeAction action;
            
            // Determine action type based on pattern
            QString patternStr = pattern.pattern();
            if (patternStr.contains("create") || patternStr.contains("Create")) {
                action = parseFileCreation(match);
            } else if (patternStr.contains("update") || patternStr.contains("Replace")) {
                action = parseFileUpdate(match);
            } else if (patternStr.contains("delete") || patternStr.contains("remove")) {
                action = parseFileDeletion(match);
            } else if (patternStr.contains("insert") || patternStr.contains("add")) {
                action = parseCodeInsertion(match);
            } else if (patternStr.contains("replace") && !patternStr.contains("content")) {
                action = parseCodeReplacement(match);
            }
            
            if (action.isValid) {
                actions.append(action);
            }
        }
    }
    
    return actions;
}

QString CodeActionParser::extractCodeBlocks(const QString &text) const
{
    QRegularExpression codeBlockRegex(R"(```(\w+)?\n(.*?)\n```)", 
                                      QRegularExpression::DotMatchesEverythingOption);
    
    QString result;
    auto matches = codeBlockRegex.globalMatch(text);
    
    while (matches.hasNext()) {
        auto match = matches.next();
        if (!result.isEmpty()) {
            result += "\n\n";
        }
        result += match.captured(2); // The code content
    }
    
    return result;
}

QStringList CodeActionParser::detectFilePaths(const QString &text) const
{
    QStringList paths;
    
    QRegularExpression filePathRegex(R"(["`']?([^'"`\s]+\.[a-zA-Z0-9]+)["`']?)");
    auto matches = filePathRegex.globalMatch(text);
    
    while (matches.hasNext()) {
        auto match = matches.next();
        QString path = match.captured(1);
        if (!paths.contains(path)) {
            paths.append(path);
        }
    }
    
    return paths;
}

CodeAction CodeActionParser::parseFileCreation(const QRegularExpressionMatch &match) const
{
    CodeAction action;
    action.type = CodeAction::CreateFile;
    action.filePath = match.captured(1).trimmed();
    action.content = match.captured(2).trimmed();
    action.description = QString("Create file: %1").arg(action.filePath);
    action.isValid = !action.filePath.isEmpty() && !action.content.isEmpty();
    
    return action;
}

CodeAction CodeActionParser::parseFileUpdate(const QRegularExpressionMatch &match) const
{
    CodeAction action;
    action.type = CodeAction::UpdateFile;
    action.filePath = match.captured(1).trimmed();
    action.content = match.captured(2).trimmed();
    action.description = QString("Update file: %1").arg(action.filePath);
    action.isValid = !action.filePath.isEmpty() && !action.content.isEmpty();
    
    return action;
}

CodeAction CodeActionParser::parseFileDeletion(const QRegularExpressionMatch &match) const
{
    CodeAction action;
    action.type = CodeAction::DeleteFile;
    action.filePath = match.captured(1).trimmed();
    action.description = QString("Delete file: %1").arg(action.filePath);
    action.isValid = !action.filePath.isEmpty();
    
    return action;
}

CodeAction CodeActionParser::parseCodeInsertion(const QRegularExpressionMatch &match) const
{
    CodeAction action;
    action.type = CodeAction::InsertCode;
    action.content = match.captured(2).trimmed();
    
    bool ok = false;
    action.lineNumber = match.captured(1).toInt(&ok);
    if (!ok) {
        action.lineNumber = -1;
    }
    
    action.description = action.lineNumber > 0 
        ? QString("Insert code at line %1").arg(action.lineNumber)
        : QString("Insert code at cursor");
    
    action.isValid = !action.content.isEmpty();
    
    return action;
}

CodeAction CodeActionParser::parseCodeReplacement(const QRegularExpressionMatch &match) const
{
    CodeAction action;
    action.type = CodeAction::ReplaceCode;
    action.content = match.captured(1).trimmed();
    action.description = "Replace selected code";
    action.isValid = !action.content.isEmpty();
    
    return action;
}