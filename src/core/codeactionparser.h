#ifndef CODEACTIONPARSER_H
#define CODEACTIONPARSER_H

#include <QObject>
#include <QString>
#include <QRegularExpression>

struct CodeAction {
    enum Type {
        None,
        CreateFile,
        UpdateFile,
        DeleteFile,
        InsertCode,
        ReplaceCode,
        OpenFile,
        RunCommand
    };
    
    Type type = None;
    QString filePath;
    QString content;
    QString description;
    int lineNumber = -1;
    bool isValid = false;
};

class CodeActionParser : public QObject
{
    Q_OBJECT

public:
    explicit CodeActionParser(QObject *parent = nullptr);
    
    QList<CodeAction> parseActions(const QString &text) const;
    QString extractCodeBlocks(const QString &text) const;
    QStringList detectFilePaths(const QString &text) const;
    
private:
    CodeAction parseFileCreation(const QRegularExpressionMatch &match) const;
    CodeAction parseFileUpdate(const QRegularExpressionMatch &match) const;
    CodeAction parseFileDeletion(const QRegularExpressionMatch &match) const;
    CodeAction parseCodeInsertion(const QRegularExpressionMatch &match) const;
    CodeAction parseCodeReplacement(const QRegularExpressionMatch &match) const;
    
    mutable QList<QRegularExpression> m_patterns;
    void initializePatterns();
};

#endif // CODEACTIONPARSER_H