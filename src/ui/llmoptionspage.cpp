#include "llmoptionspage.h"

#include "src/settings/llmsettings.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QWidget>

LLMOptionsPage::LLMOptionsPage() : Core::IOptionsPage(true)
{
    setId("LLM.Settings");
    setDisplayName("LLM Provider");
    setCategory("LLM");
    registerCategory(Utils::Id::generate(), "Test LLM", "");
    // setDisplayCategory("LLM Assistant");
}

QWidget *LLMOptionsPage::widget()
{
    if (widget_)
        return widget_;

    widget_ = new QWidget;

    baseUrlEdit = new QLineEdit(LLMSettings::instance().baseUrl());
    modelEdit = new QLineEdit(LLMSettings::instance().model());

    auto layout = new QFormLayout(widget_);
    layout->addRow("Base URL:", baseUrlEdit);
    layout->addRow("Model:", modelEdit);

    return widget_;
}

void LLMOptionsPage::apply()
{
    auto &s = LLMSettings::instance();
    s.setBaseUrl(baseUrlEdit->text());
    s.setModel(modelEdit->text());
    s.save();
}

void LLMOptionsPage::finish()
{
    delete widget_;
    widget_ = nullptr;
}
