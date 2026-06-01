#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <cstdlib>

/**
 * Delivery node configuration for the Forum app.
 *
 * Defaults use the "logos.dev" preset (Logos Dev Network, cluster 2).
 * Override via environment variables:
 *   FORUM_LOG_LEVEL  – delivery log level (default: "INFO")
 *   FORUM_NODE_MODE  – delivery node mode (default: "Core")
 *   FORUM_PRESET     – network preset (default: "logos.dev")
 */
namespace ForumConfig {

inline QString getEnv(const char* name, const QString& fallback)
{
    const char* v = std::getenv(name);
    return v ? QString::fromUtf8(v) : fallback;
}

inline QString defaultForumName()
{
    return QStringLiteral("Logos Forum");
}

inline QString buildDeliveryConfigJson()
{
    QJsonObject cfg;
    cfg["logLevel"] = getEnv("FORUM_LOG_LEVEL", QStringLiteral("INFO"));
    cfg["mode"]     = getEnv("FORUM_NODE_MODE",  QStringLiteral("Core"));
    cfg["preset"]   = getEnv("FORUM_PRESET",     QStringLiteral("logos.dev"));
    return QJsonDocument(cfg).toJson(QJsonDocument::Compact);
}

} // namespace ForumConfig
