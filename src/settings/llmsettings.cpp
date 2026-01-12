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
    baseUrl_ = s.value("LLM/baseUrl", "http://localhost:11434").toString();
    model_   = s.value("LLM/model", "llama3").toString();
}

void LLMSettings::save()
{
    QSettings s;
    s.setValue("LLM/baseUrl", baseUrl_);
    s.setValue("LLM/model", model_);
}

QString LLMSettings::baseUrl() const { return baseUrl_; }
QString LLMSettings::model() const { return model_; }

void LLMSettings::setBaseUrl(const QString &v) { baseUrl_ = v; }
void LLMSettings::setModel(const QString &v) { model_ = v; }
