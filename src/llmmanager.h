#ifndef LLMMANAGER_H
#define LLMMANAGER_H

#include <QObject>
#include "src/providers/base/llmprovider.h"

class LLMManager : public QObject
{
    Q_OBJECT
public:
    explicit LLMManager(QObject *parent = nullptr);

    void setProvider(LLMProvider *provider);
    void sendPrompt(const QString &prompt);

signals:
    void responseReady(const QString &text);
    void errorOccurred(const QString &error);

private:
    LLMProvider *current = nullptr;
};
#endif // LLMMANAGER_H
