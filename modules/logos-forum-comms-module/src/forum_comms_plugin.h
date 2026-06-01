#pragma once

#include <QObject>
#include <QMap>
#include "forum_comms_interface.h"
#include "logos_api.h"
#include "logos_sdk.h"
#include "logos_types.h"

/**
 * @class ForumCommsPlugin
 * @brief Adapts logos-delivery into forum-level publish/subscribe messaging.
 *
 * Wraps the delivery_module to handle forum content topics and
 * encode/decode forum post payloads. Other modules (e.g. forum_ui) interact
 * with this module rather than directly with the delivery layer.
 *
 * **Startup sequence:**
 * 1. Logos Core calls @ref initLogos — injects the API bridge.
 * 2. Caller calls @ref initDelivery(configJson) — creates and starts the node.
 * 3. Caller calls @ref subscribeToForum(forumName) — receives new topics.
 * 4. On receiving a new_topic post, optionally call
 *    @ref subscribeToTopic(forumName, topicId) to receive replies.
 *
 * **Events emitted** (positional QVariantList indices):
 *
 * | Event | Indices |
 * |---|---|
 * | `forumPostReceived` | [0] postId, [1] forumName, [2] topicId, [3] topicTitle, [4] authorId, [5] content, [6] timestamp, [7] type |
 * | `forumPostSent`     | [0] requestId, [1] postId, [2] timestamp |
 * | `forumPostError`    | [0] requestId, [1] errorMsg, [2] timestamp |
 * | `forumConnectionChanged` | [0] status, [1] timestamp |
 */
class ForumCommsPlugin : public QObject,
                         public ForumCommsInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ForumCommsInterface_iid FILE "../metadata.json")
    Q_INTERFACES(ForumCommsInterface PluginInterface)

public:
    explicit ForumCommsPlugin(QObject* parent = nullptr);
    ~ForumCommsPlugin() override;

    QString name()    const override { return QStringLiteral("forum_comms_module"); }
    QString version() const override { return QStringLiteral("1.0.0"); }

    Q_INVOKABLE void initLogos(LogosAPI* api);

    Q_INVOKABLE LogosResult initDelivery(const QString& configJson) override;
    Q_INVOKABLE LogosResult stop() override;

    Q_INVOKABLE LogosResult subscribeToForum(const QString& forumName) override;
    Q_INVOKABLE LogosResult subscribeToTopic(const QString& forumName, const QString& topicId) override;
    Q_INVOKABLE LogosResult unsubscribeFromForum(const QString& forumName) override;
    Q_INVOKABLE LogosResult unsubscribeFromTopic(const QString& forumName, const QString& topicId) override;

    Q_INVOKABLE LogosResult publishNewTopic(const QString& forumName, const QString& topicTitle,
                                            const QString& authorId, const QString& content) override;
    Q_INVOKABLE LogosResult publishPost(const QString& forumName, const QString& topicId,
                                        const QString& authorId, const QString& content) override;

signals:
    void eventResponse(const QString& eventName, const QVariantList& data);

private:
    void setupDeliveryEventHandlers();

    // Handlers for delivery_module events
    void onMessageReceived(const QVariantList& data);
    void onMessageSent(const QVariantList& data);
    void onMessageError(const QVariantList& data);
    void onConnectionChanged(const QVariantList& data);

    void emitEvent(const QString& eventName, const QVariantList& data);

    LogosAPI*      m_logosAPI = nullptr;
    LogosModules*  m_logos    = nullptr;

    // Maps delivery requestId -> postId for sent messages
    QMap<QString, QString> m_pendingRequests;

    // Reverse map: content topic -> forumName (for decoding received messages)
    QMap<QString, QString> m_topicToForum;
};
