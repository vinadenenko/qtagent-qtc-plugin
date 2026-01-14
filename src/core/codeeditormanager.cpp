#include "codeeditormanager.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <texteditor/texteditor.h>
#include <texteditor/textdocument.h>
// #include <texteditor/texteditorwidget.h>
#include <projectexplorer/projectmanager.h>
#include <projectexplorer/project.h>
#include <utils/fileutils.h>

#include <QFileInfo>
#include <QTextCursor>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

CodeEditorManager::CodeEditorManager(QObject *parent)
    : QObject(parent)
{
    setupEditorConnections();
}

void CodeEditorManager::setupEditorConnections()
{
    auto editorManager = Core::EditorManager::instance();
    // if (editorManager) {
    //     connect(editorManager, &Core::EditorManager::currentEditorChanged,
    //             this, &CodeEditorManager::editorContextChanged);
    // }
}

CodeEditorManager::EditorContext CodeEditorManager::getCurrentEditorContext() const
{
    EditorContext context;
    context.isValid = false;
    
    auto editorManager = Core::EditorManager::instance();
    if (!editorManager) {
        return context;
    }
    
    auto currentEditor = editorManager->currentEditor();
    if (!currentEditor) {
        return context;
    }
    
    auto textEditor = qobject_cast<TextEditor::BaseTextEditor*>(currentEditor);
    if (!textEditor) {
        return context;
    }
    
    auto editorWidget = textEditor->editorWidget();
    if (!editorWidget) {
        return context;
    }
    
    context.filePath = currentEditor->document()->filePath().toString();
    context.content = editorWidget->toPlainText();
    context.cursorPosition = editorWidget->textCursor().position();
    
    auto cursor = editorWidget->textCursor();
    if (cursor.hasSelection()) {
        context.selectedText = cursor.selectedText();
        context.selectionStart = cursor.selectionStart();
        context.selectionEnd = cursor.selectionEnd();
    } else {
        context.selectedText.clear();
        context.selectionStart = context.cursorPosition;
        context.selectionEnd = context.cursorPosition;
    }
    
    context.isValid = true;
    m_lastContext = context;
    
    return context;
}

bool CodeEditorManager::insertText(const QString &text)
{
    auto editorManager = Core::EditorManager::instance();
    if (!editorManager) {
        return false;
    }
    
    auto currentEditor = editorManager->currentEditor();
    if (!currentEditor) {
        return false;
    }
    
    auto textEditor = qobject_cast<TextEditor::BaseTextEditor*>(currentEditor);
    if (!textEditor) {
        return false;
    }

    auto editorWidget = textEditor->editorWidget();
    if (!editorWidget) {
        return false;
    }
    
    auto cursor = editorWidget->textCursor();
    cursor.insertText(text);
    editorWidget->setTextCursor(cursor);
    
    emit textInserted(text);
    return true;
}

bool CodeEditorManager::replaceSelectedText(const QString &text)
{
    auto editorManager = Core::EditorManager::instance();
    if (!editorManager) {
        return false;
    }
    
    auto currentEditor = editorManager->currentEditor();
    if (!currentEditor) {
        return false;
    }

    auto textEditor = qobject_cast<TextEditor::BaseTextEditor*>(currentEditor);
    if (!textEditor) {
        return false;
    }

    auto editorWidget = textEditor->editorWidget();
    if (!editorWidget) {
        return false;
    }
    
    auto cursor = editorWidget->textCursor();
    if (!cursor.hasSelection()) {
        return insertText(text);
    }
    
    cursor.insertText(text);
    editorWidget->setTextCursor(cursor);
    
    emit textReplaced(text);
    return true;
}

bool CodeEditorManager::insertTextAtPosition(const QString &text, int position)
{
    auto editorManager = Core::EditorManager::instance();
    if (!editorManager) {
        return false;
    }
    
    auto currentEditor = editorManager->currentEditor();
    if (!currentEditor) {
        return false;
    }

    auto textEditor = qobject_cast<TextEditor::BaseTextEditor*>(currentEditor);
    if (!textEditor) {
        return false;
    }

    auto editorWidget = textEditor->editorWidget();
    if (!editorWidget) {
        return false;
    }
    
    auto cursor = editorWidget->textCursor();
    cursor.setPosition(position);
    cursor.insertText(text);
    editorWidget->setTextCursor(cursor);
    
    emit textInserted(text);
    return true;
}

bool CodeEditorManager::createFile(const QString &filePath, const QString &content)
{
    if (writeFile(filePath, content)) {
        return openFile(filePath);
    }
    return false;
}

bool CodeEditorManager::openFile(const QString &filePath)
{
    auto editorManager = Core::EditorManager::instance();
    if (!editorManager) {
        return false;
    }
    
    auto document = editorManager->openEditor(Utils::FilePath::fromString(filePath));
    return document != nullptr;
}

bool CodeEditorManager::readFile(const QString &filePath, QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    content = QString::fromUtf8(file.readAll());
    file.close();
    return true;
}

bool CodeEditorManager::writeFile(const QString &filePath, const QString &content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << content;
    file.close();
    return true;
}

bool CodeEditorManager::deleteFile(const QString &filePath)
{
    return QFile::remove(filePath);
}

bool CodeEditorManager::fileExists(const QString &filePath) const
{
    return QFile::exists(filePath);
}

QString CodeEditorManager::getProjectPath() const
{
    auto projectManager = ProjectExplorer::ProjectManager::instance();
    if (!projectManager) {
        return QString();
    }
    
    auto currentProject = projectManager->startupProject();
    if (!currentProject) {
        return QString();
    }
    
    return currentProject->projectDirectory().toUrlishString();
}

QString CodeEditorManager::resolvePath(const QString &relativePath) const
{
    QString projectPath = getProjectPath();
    if (!projectPath.isEmpty()) {
        return QDir(projectPath).filePath(relativePath);
    }
    
    return relativePath;
}
