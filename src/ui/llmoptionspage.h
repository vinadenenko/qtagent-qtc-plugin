#ifndef LLMOPTIONSPAGE_H
#define LLMOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

class QLineEdit;
class QComboBox;

class LLMOptionsPage : public Core::IOptionsPage
{
public:
    LLMOptionsPage();

    QWidget *widget() override;
    void apply() override;
    void finish() override;

private:
    QComboBox *providerCombo;
    QLineEdit *baseUrlEdit;
    QLineEdit *modelEdit;
    QLineEdit *apiKeyEdit;
    QWidget *widget_ = nullptr;
};

#endif // LLMOPTIONSPAGE_H
