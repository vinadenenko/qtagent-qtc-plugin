#ifndef CODEEDITORMANAGER_H
#define CODEEDITORMANAGER_H

#include <QObject>
#include <QString>

class CodeEditorManager : public QObject
{
    Q_OBJECT

public:
    explicit CodeEditorManager(QObject *parent = nullptr);
    
    struct EditorContext {
        QString filePath;
        QString content;
        int cursorPosition;
        QString selectedText;
        int selectionStart;
        int selectionEnd;
        bool isValid;
    };
    
    EditorContext getCurrentEditorContext() const;
    
    bool insertText(const QString &text);
    bool replaceSelectedText(const QString &text);
    bool insertTextAtPosition(const QString &text, int position);
    
    bool createFile(const QString &filePath, const QString &content);
    bool openFile(const QString &filePath);
    bool readFile(const QString &filePath, QString &content);
    bool writeFile(const QString &filePath, const QString &content);
    bool deleteFile(const QString &filePath);
    bool fileExists(const QString &filePath) const;
    
    QString getProjectPath() const;
    QString resolvePath(const QString &relativePath) const;

signals:
    void editorContextChanged(const CodeEditorManager::EditorContext &context);
    void textInserted(const QString &text);
    void textReplaced(const QString &text);
    void fileCreated(const QString &filePath);

private:
    void setupEditorConnections();
    
    mutable EditorContext m_lastContext;
};

#endif // CODEEDITORMANAGER_H
