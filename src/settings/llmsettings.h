#ifndef LLMSETTINGS_H
#define LLMSETTINGS_H


#include <QObject>


class LLMSettings : public QObject
{
    Q_OBJECT
public:
    static LLMSettings &instance();

    QString baseUrl() const;
    QString model() const;
    QString apiKey() const;
    QString providerType() const;

    int contextLimit() const;

    void setBaseUrl(const QString &v);
    void setModel(const QString &v);
    void setApiKey(const QString &v);
    void setProviderType(const QString &v);
    void setContextLimit(int v);

    void load();
    void save();

signals:
    void settingsChanged();

private:
    LLMSettings();
    QString baseUrl_;
    QString model_;
    QString apiKey_;
    QString providerType_;
    int contextLimit_ = 32000;
};
#endif // LLMSETTINGS_H
