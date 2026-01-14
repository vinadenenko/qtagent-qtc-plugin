#include "llmoptionspage.h"

#include "src/settings/llmsettings.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QWidget>

LLMOptionsPage::LLMOptionsPage() : Core::IOptionsPage(true)
{
    setId("LLM.Settings");
    setDisplayName("LLM Provider");
    setCategory("LLM");
    registerCategory(Utils::Id::generate(), "Test LLM", "");
}

QWidget *LLMOptionsPage::widget()
{
    if (widget_)
        return widget_;

    widget_ = new QWidget;

    auto &s = LLMSettings::instance();
    
    providerCombo = new QComboBox;
    providerCombo->addItems({"Ollama", "OpenAI", "Claude"});
    providerCombo->setCurrentText(s.providerType());

    baseUrlEdit = new QLineEdit(s.baseUrl());
    modelEdit = new QLineEdit(s.model());
    apiKeyEdit = new QLineEdit(s.apiKey());
    apiKeyEdit->setEchoMode(QLineEdit::Password);

    auto layout = new QFormLayout(widget_);
    layout->addRow("Provider:", providerCombo);
    layout->addRow("Base URL:", baseUrlEdit);
    layout->addRow("Model:", modelEdit);
    layout->addRow("API Key:", apiKeyEdit);

    return widget_;
}

void LLMOptionsPage::apply()
{
    auto &s = LLMSettings::instance();
    s.setProviderType(providerCombo->currentText());
    s.setBaseUrl(baseUrlEdit->text());
    s.setModel(modelEdit->text());
    s.setApiKey(apiKeyEdit->text());
    s.save();
}

void LLMOptionsPage::finish()
{
    delete widget_;
    widget_ = nullptr;
}
