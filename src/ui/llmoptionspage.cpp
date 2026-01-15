#include "llmoptionspage.h"

#include "src/settings/llmsettings.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QWidget>
#include <QObject>

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

    connect(providerCombo, &QComboBox::currentTextChanged, this, &LLMOptionsPage::onProviderChanged);

    baseUrlEdit = new QLineEdit(s.baseUrl());
    modelEdit = new QLineEdit(s.model());
    apiKeyEdit = new QLineEdit(s.apiKey());
    apiKeyEdit->setEchoMode(QLineEdit::Password);

    contextLimitEdit = new QLineEdit(QString::number(s.contextLimit()));

    auto layout = new QFormLayout(widget_);
    layout->addRow("Provider:", providerCombo);
    layout->addRow("Base URL:", baseUrlEdit);
    layout->addRow("Model:", modelEdit);
    layout->addRow("API Key:", apiKeyEdit);
    layout->addRow("Context Limit:", contextLimitEdit);

    return widget_;
}

void LLMOptionsPage::apply()
{
    auto &s = LLMSettings::instance();
    s.setProviderType(providerCombo->currentText());
    s.setBaseUrl(baseUrlEdit->text());
    s.setModel(modelEdit->text());
    s.setApiKey(apiKeyEdit->text());
    s.setContextLimit(contextLimitEdit->text().toInt());
    s.save();
}

void LLMOptionsPage::onProviderChanged(const QString &type)
{
    if (baseUrlEdit->text().isEmpty() || 
        baseUrlEdit->text() == "http://localhost:11434" || 
        baseUrlEdit->text() == "https://api.openai.com/v1" || 
        baseUrlEdit->text() == "https://api.anthropic.com/v1") 
    {
        if (type == "Ollama") baseUrlEdit->setText("http://localhost:11434");
        else if (type == "OpenAI") baseUrlEdit->setText("https://api.openai.com/v1");
        else if (type == "Claude") baseUrlEdit->setText("https://api.anthropic.com/v1");
    }
}

void LLMOptionsPage::finish()
{
    delete widget_;
    widget_ = nullptr;
}
