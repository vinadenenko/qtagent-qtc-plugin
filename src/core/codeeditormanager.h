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
    
    virtual EditorContext getCurrentEditorContext() const;
    
    virtual bool insertText(const QString &text);
    virtual bool replaceSelectedText(const QString &text);
    virtual bool insertTextAtPosition(const QString &text, int position);
    
    virtual bool createFile(const QString &filePath, const QString &content);
    virtual bool openFile(const QString &filePath);
    virtual bool readFile(const QString &filePath, QString &content);
    virtual bool writeFile(const QString &filePath, const QString &content);
    virtual bool deleteFile(const QString &filePath);
    virtual bool fileExists(const QString &filePath) const;
    
    virtual QString getProjectPath() const;
    virtual QString resolvePath(const QString &relativePath) const;

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
