#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>
#include <QDateTime>
#include <QRegularExpression>

/**
 * Encode/decode forum posts to/from the JSON payload sent over logos-delivery.
 *
 * Content topics (LIP-23 format):
 *   Main forum:  /forums/1/{forumId}/posts       – new topics land here
 *   Topic feed:  /forums/1/{forumId}/{topicId}/posts  – replies land here
 *
 * Payload schema (version 1):
 * {
 *   "v":          1,
 *   "postId":     "<uuid>",
 *   "type":       "new_topic" | "reply",
 *   "forumId":    "<sanitized forum name>",
 *   "topicId":    "<uuid>",
 *   "topicTitle": "<string>",   // only for new_topic
 *   "authorId":   "<string>",
 *   "content":    "<string>",
 *   "timestamp":  "<ISO-8601>"
 * }
 */
namespace ForumCodec {

struct Post {
    QString postId;
    QString type;       // "new_topic" or "reply"
    QString forumId;
    QString topicId;
    QString topicTitle;
    QString authorId;
    QString content;
    QString timestamp;
    bool valid = false;
};

// Converts a human-readable forum name to a URL-safe identifier.
inline QString forumId(const QString& forumName)
{
    return forumName.toLower().replace(QRegularExpression(QStringLiteral("[^a-z0-9]")),
                                       QStringLiteral("-"));
}

inline QString forumContentTopic(const QString& forumName)
{
    return QStringLiteral("/forums/1/%1/posts").arg(forumId(forumName));
}

inline QString topicContentTopic(const QString& forumName, const QString& topicId)
{
    return QStringLiteral("/forums/1/%1/%2/posts").arg(forumId(forumName), topicId);
}

inline QString encodeNewTopic(const QString& forumName, const QString& topicTitle,
                               const QString& authorId, const QString& content)
{
    const QString tid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QJsonObject obj;
    obj["v"]          = 1;
    obj["postId"]     = QUuid::createUuid().toString(QUuid::WithoutBraces);
    obj["type"]       = QStringLiteral("new_topic");
    obj["forumId"]    = forumId(forumName);
    obj["topicId"]    = tid;
    obj["topicTitle"] = topicTitle;
    obj["authorId"]   = authorId;
    obj["content"]    = content;
    obj["timestamp"]  = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

inline QString encodeReply(const QString& forumName, const QString& topicId,
                            const QString& authorId, const QString& content)
{
    QJsonObject obj;
    obj["v"]         = 1;
    obj["postId"]    = QUuid::createUuid().toString(QUuid::WithoutBraces);
    obj["type"]      = QStringLiteral("reply");
    obj["forumId"]   = forumId(forumName);
    obj["topicId"]   = topicId;
    obj["authorId"]  = authorId;
    obj["content"]   = content;
    obj["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

inline Post decode(const QString& payload)
{
    Post p;
    const QJsonDocument doc = QJsonDocument::fromJson(payload.toUtf8());
    if (!doc.isObject()) return p;

    const QJsonObject obj = doc.object();
    if (obj["v"].toInt() != 1) return p;

    p.postId     = obj["postId"].toString();
    p.type       = obj["type"].toString();
    p.forumId    = obj["forumId"].toString();
    p.topicId    = obj["topicId"].toString();
    p.topicTitle = obj["topicTitle"].toString();
    p.authorId   = obj["authorId"].toString();
    p.content    = obj["content"].toString();
    p.timestamp  = obj["timestamp"].toString();
    p.valid      = !p.postId.isEmpty() && !p.type.isEmpty()
                   && !p.forumId.isEmpty() && !p.topicId.isEmpty()
                   && !p.authorId.isEmpty();
    return p;
}

} // namespace ForumCodec
