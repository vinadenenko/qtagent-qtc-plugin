#include <QtTest>
#include "../src/core/conversationhistory.h"

class TestConversationHistory : public QObject
{
    Q_OBJECT

private slots:
    void testAddMessage() {
        ConversationHistory history;
        history.addMessage(Message::User, "Hello");
        QCOMPARE(history.messageCount(), 1);
        QCOMPARE(history.messages().first().content, QString("Hello"));
        QCOMPARE(history.messages().first().role, Message::User);
    }

    void testToJson() {
        ConversationHistory history;
        history.addMessage(Message::System, "System prompt");
        history.addMessage(Message::User, "User query");
        
        QJsonArray arr = history.toJsonArray();
        QCOMPARE(arr.size(), 2);
        QCOMPARE(arr[0].toObject()["role"].toString(), QString("system"));
        QCOMPARE(arr[1].toObject()["role"].toString(), QString("user"));
    }

    void testEstimateTokenCount() {
        ConversationHistory history;
        history.addMessage(Message::User, "12345678"); // 8 chars -> ~2 tokens + 10 overhead = 12
        int count = history.estimateTokenCount();
        QCOMPARE(count, 12);
    }

    void testTrim() {
        ConversationHistory history;
        history.addMessage(Message::System, "System"); // 6/4 + 10 = 11
        history.addMessage(Message::User, "User 1");   // 6/4 + 10 = 11
        history.addMessage(Message::User, "User 2");   // 6/4 + 10 = 11
        
        // Total ~33
        QVERIFY(history.estimateTokenCount() > 30);
        
        history.trim(25); // Should remove User 1, keep System and User 2
        QCOMPARE(history.messageCount(), 2);
        QCOMPARE(history.messages()[0].role, Message::System);
        QCOMPARE(history.messages()[1].content, QString("User 2"));
    }
};

QTEST_MAIN(TestConversationHistory)
#include "tst_conversationhistory.moc"
