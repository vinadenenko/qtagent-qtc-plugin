#ifndef LLMPROVIDER_H
#define LLMPROVIDER_H


#include <QObject>

class LLMProvider : public QObject
{
    Q_OBJECT
public:
    explicit LLMProvider(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~LLMProvider() = default;

    virtual QString name() const = 0;

    virtual void sendPrompt(const QString &prompt) = 0;

signals:
    void responseReady(const QString &text);
    void errorOccurred(const QString &error);
};

#endif // LLMPROVIDER_H
