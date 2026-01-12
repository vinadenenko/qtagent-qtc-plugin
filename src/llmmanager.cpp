#include "llmmanager.h"

LLMManager::LLMManager(QObject *parent) : QObject(parent) {}

void LLMManager::setProvider(LLMProvider *provider)
{
    if (current)
        disconnect(current, nullptr, this, nullptr);

    current = provider;

    connect(current, &LLMProvider::responseReady,
            this, &LLMManager::responseReady);

    connect(current, &LLMProvider::errorOccurred,
            this, &LLMManager::errorOccurred);
}

void LLMManager::sendPrompt(const QString &prompt)
{
    if (current)
        current->sendPrompt(prompt);
}
