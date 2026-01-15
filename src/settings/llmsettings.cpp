#include "llmsettings.h"




#include <QSettings>

LLMSettings &LLMSettings::instance()
{
    static LLMSettings s;
    return s;
}

LLMSettings::LLMSettings()
{
    load();
}

void LLMSettings::load()
{
    QSettings s;
    providerType_ = s.value("LLM/providerType", "Ollama").toString();
    
    QString defaultUrl = "http://localhost:11434";
    if (providerType_ == "OpenAI") defaultUrl = "https://api.openai.com/v1";
    else if (providerType_ == "Claude") defaultUrl = "https://api.anthropic.com/v1";

    baseUrl_      = s.value("LLM/baseUrl", defaultUrl).toString();
    model_        = s.value("LLM/model", "llama3").toString();
    apiKey_       = s.value("LLM/apiKey", "").toString();
    contextLimit_ = s.value("LLM/contextLimit", 32000).toInt();
}

void LLMSettings::save()
{
    QSettings s;
    s.setValue("LLM/baseUrl", baseUrl_);
    s.setValue("LLM/model", model_);
    s.setValue("LLM/apiKey", apiKey_);
    s.setValue("LLM/providerType", providerType_);
    s.setValue("LLM/contextLimit", contextLimit_);
    emit settingsChanged();
}

QString LLMSettings::baseUrl() const { return baseUrl_; }
QString LLMSettings::model() const { return model_; }
QString LLMSettings::apiKey() const { return apiKey_; }
QString LLMSettings::providerType() const { return providerType_; }
int LLMSettings::contextLimit() const { return contextLimit_; }

void LLMSettings::setBaseUrl(const QString &v) { baseUrl_ = v; }
void LLMSettings::setModel(const QString &v) { model_ = v; }
void LLMSettings::setApiKey(const QString &v) { apiKey_ = v; }
void LLMSettings::setProviderType(const QString &v) { providerType_ = v; }
void LLMSettings::setContextLimit(int v) { contextLimit_ = v; }
