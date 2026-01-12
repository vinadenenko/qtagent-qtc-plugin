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

    void setBaseUrl(const QString &v);
    void setModel(const QString &v);

    void load();
    void save();

private:
    LLMSettings();
    QString baseUrl_;
    QString model_;
};
#endif // LLMSETTINGS_H
