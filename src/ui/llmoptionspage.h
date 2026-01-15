#ifndef LLMOPTIONSPAGE_H
#define LLMOPTIONSPAGE_H

#include <coreplugin/dialogs/ioptionspage.h>
#include <QObject>

class QLineEdit;
class QComboBox;

class LLMOptionsPage : public QObject, public Core::IOptionsPage
{
    Q_OBJECT
public:
    LLMOptionsPage();

    QWidget *widget() override;
    void apply() override;
    void finish() override;

private slots:
    void onProviderChanged(const QString &type);

private:
    QComboBox *providerCombo;
    QLineEdit *baseUrlEdit;
    QLineEdit *modelEdit;
    QLineEdit *apiKeyEdit;
    QWidget *widget_ = nullptr;
};

#endif // LLMOPTIONSPAGE_H
