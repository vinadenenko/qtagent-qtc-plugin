#ifndef CONVERSATIONHISTORY_H
#define CONVERSATIONHISTORY_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

struct Message {
    enum Role {
        System,
        User,
        Assistant,
        Tool
    };

    Role role;
    QString content;
    QString toolCallId; // For tool call responses
    QJsonArray toolCalls; // For assistant messages calling tools
    QDateTime timestamp;

    static QString roleToString(Role role) {
        switch (role) {
            case System: return "system";
            case User: return "user";
            case Assistant: return "assistant";
            case Tool: return "tool";
        }
        return "user";
    }

    static Role stringToRole(const QString &role) {
        if (role == "system") return System;
        if (role == "assistant") return Assistant;
        if (role == "tool") return Tool;
        return User;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["role"] = roleToString(role);
        
        if (role == Tool) {
            obj["content"] = content;
            if (!toolCallId.isEmpty()) obj["tool_call_id"] = toolCallId;
        } else if (role == Assistant && !toolCalls.isEmpty()) {
            // Assistant message with tool calls often shouldn't have content, or it's null
            if (content.isEmpty()) {
                obj["content"] = QJsonValue::Null;
            } else {
                obj["content"] = content;
            }
            obj["tool_calls"] = toolCalls;
        } else {
            obj["content"] = content;
        }
        
        return obj;
    }
};

class ConversationHistory {
public:
    void addMessage(Message::Role role, const QString &content, const QString &toolCallId = QString(), const QJsonArray &toolCalls = QJsonArray()) {
        m_messages.append({role, content, toolCallId, toolCalls, QDateTime::currentDateTime()});
    }

    void addMessage(const Message &msg) {
        m_messages.append(msg);
    }

    const QList<Message>& messages() const {
        return m_messages;
    }

    void clear() {
        m_messages.clear();
    }

    QJsonArray toJsonArray() const {
        QJsonArray arr;
        for (const auto &msg : m_messages) {
            arr.append(msg.toJson());
        }
        return arr;
    }

    int messageCount() const {
        return m_messages.size();
    }

    // Basic token estimation (characters / 4 as a rough proxy if no tokenizer available)
    int estimateTokenCount() const {
        int count = 0;
        for (const auto &msg : m_messages) {
            count += msg.content.length() / 4 + 10; // +10 for metadata overhead
        }
        return count;
    }

    void trim(int maxTokens) {
        while (estimateTokenCount() > maxTokens && m_messages.size() > 1) {
            // Keep system message if it's the first one
            if (m_messages.first().role == Message::System && m_messages.size() > 2) {
                m_messages.removeAt(1);
            } else {
                m_messages.removeFirst();
            }
        }
    }

private:
    QList<Message> m_messages;
};

#endif // CONVERSATIONHISTORY_H
