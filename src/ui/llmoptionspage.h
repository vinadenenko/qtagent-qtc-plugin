#ifndef LLMOPTIONSPAGE_H
#define LLMOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>

class QLineEdit;

class LLMOptionsPage : public Core::IOptionsPage
{
public:
    LLMOptionsPage();

    QWidget *widget() override;
    void apply() override;
    void finish() override;

private:
    QLineEdit *baseUrlEdit;
    QLineEdit *modelEdit;
    QWidget *widget_ = nullptr;
};

#endif // LLMOPTIONSPAGE_H
