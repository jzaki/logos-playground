#include "forum_comms_plugin.h"
#include "ForumCodec.h"

#include <QDebug>
#include <QDateTime>
#include <QMetaObject>

namespace {
LogosResult ok()    { return { true,  {}, {} }; }
LogosResult fail(const QString& msg) { return { false, {}, msg }; }
} // namespace

ForumCommsPlugin::ForumCommsPlugin(QObject* parent)
    : QObject(parent)
{
}

ForumCommsPlugin::~ForumCommsPlugin()
{
    delete m_logos;
}

void ForumCommsPlugin::initLogos(LogosAPI* api)
{
    if (m_logosAPI) return;
    m_logosAPI = api;
    m_logos = new LogosModules(m_logosAPI);
    setupDeliveryEventHandlers();
    qDebug() << "ForumCommsPlugin: Logos API injected";
}

LogosResult ForumCommsPlugin::initDelivery(const QString& configJson)
{
    if (!m_logos)
        return fail(QStringLiteral("initDelivery called before initLogos"));

    qDebug() << "ForumCommsPlugin: Creating delivery node";
    LogosResult r = m_logos->delivery_module.createNode(configJson);
    if (!r.success) {
        qWarning() << "ForumCommsPlugin: createNode failed:" << r.error.toString();
        return r;
    }

    r = m_logos->delivery_module.start();
    if (!r.success) {
        qWarning() << "ForumCommsPlugin: start failed:" << r.error.toString();
        return r;
    }

    qDebug() << "ForumCommsPlugin: Delivery started";
    return ok();
}

LogosResult ForumCommsPlugin::stop()
{
    if (!m_logos) return fail(QStringLiteral("Not initialised"));
    return m_logos->delivery_module.stop();
}

LogosResult ForumCommsPlugin::subscribeToForum(const QString& forumName)
{
    if (!m_logos) return fail(QStringLiteral("Not initialised"));
    const QString topic = ForumCodec::forumContentTopic(forumName);
    m_topicToForum[topic] = forumName;
    qDebug() << "ForumCommsPlugin: Subscribing to forum topic:" << topic;
    return m_logos->delivery_module.subscribe(topic);
}

LogosResult ForumCommsPlugin::subscribeToTopic(const QString& forumName, const QString& topicId)
{
    if (!m_logos) return fail(QStringLiteral("Not initialised"));
    const QString topic = ForumCodec::topicContentTopic(forumName, topicId);
    m_topicToForum[topic] = forumName;
    qDebug() << "ForumCommsPlugin: Subscribing to topic:" << topic;
    return m_logos->delivery_module.subscribe(topic);
}

LogosResult ForumCommsPlugin::unsubscribeFromForum(const QString& forumName)
{
    if (!m_logos) return fail(QStringLiteral("Not initialised"));
    const QString topic = ForumCodec::forumContentTopic(forumName);
    m_topicToForum.remove(topic);
    return m_logos->delivery_module.unsubscribe(topic);
}

LogosResult ForumCommsPlugin::unsubscribeFromTopic(const QString& forumName, const QString& topicId)
{
    if (!m_logos) return fail(QStringLiteral("Not initialised"));
    const QString topic = ForumCodec::topicContentTopic(forumName, topicId);
    m_topicToForum.remove(topic);
    return m_logos->delivery_module.unsubscribe(topic);
}

LogosResult ForumCommsPlugin::publishNewTopic(const QString& forumName, const QString& topicTitle,
                                               const QString& authorId, const QString& content)
{
    if (!m_logos) return fail(QStringLiteral("Not initialised"));

    const QString payload = ForumCodec::encodeNewTopic(forumName, topicTitle, authorId, content);
    const ForumCodec::Post p = ForumCodec::decode(payload);
    const QString contentTopic = ForumCodec::forumContentTopic(forumName);

    LogosResult r = m_logos->delivery_module.send(contentTopic, payload);
    if (!r.success) {
        qWarning() << "ForumCommsPlugin: send failed for new topic:" << r.error.toString();
        return r;
    }

    const QString requestId = r.getString();
    if (!requestId.isEmpty())
        m_pendingRequests[requestId] = p.postId;

    qDebug() << "ForumCommsPlugin: Published new topic, requestId:" << requestId;
    return ok();
}

LogosResult ForumCommsPlugin::publishPost(const QString& forumName, const QString& topicId,
                                           const QString& authorId, const QString& content)
{
    if (!m_logos) return fail(QStringLiteral("Not initialised"));

    const QString payload = ForumCodec::encodeReply(forumName, topicId, authorId, content);
    const ForumCodec::Post p = ForumCodec::decode(payload);
    const QString contentTopic = ForumCodec::topicContentTopic(forumName, topicId);

    LogosResult r = m_logos->delivery_module.send(contentTopic, payload);
    if (!r.success) {
        qWarning() << "ForumCommsPlugin: send failed for reply:" << r.error.toString();
        return r;
    }

    const QString requestId = r.getString();
    if (!requestId.isEmpty())
        m_pendingRequests[requestId] = p.postId;

    qDebug() << "ForumCommsPlugin: Published reply, requestId:" << requestId;
    return ok();
}

// ── Event handler setup ──────────────────────────────────────────────────────

void ForumCommsPlugin::setupDeliveryEventHandlers()
{
    if (!m_logos) return;

    auto safeInvoke = [this](auto handler) {
        return [this, handler](const QVariantList& data) {
            QMetaObject::invokeMethod(this, [this, handler, data]() {
                (this->*handler)(data);
            }, Qt::QueuedConnection);
        };
    };

    m_logos->delivery_module.on("messageReceived",        safeInvoke(&ForumCommsPlugin::onMessageReceived));
    m_logos->delivery_module.on("messageSent",            safeInvoke(&ForumCommsPlugin::onMessageSent));
    m_logos->delivery_module.on("messageError",           safeInvoke(&ForumCommsPlugin::onMessageError));
    m_logos->delivery_module.on("connectionStateChanged", safeInvoke(&ForumCommsPlugin::onConnectionChanged));

    qDebug() << "ForumCommsPlugin: Delivery event handlers registered";
}

// ── Delivery event handlers ──────────────────────────────────────────────────

void ForumCommsPlugin::onMessageReceived(const QVariantList& data)
{
    // delivery messageReceived: [0]=hash, [1]=contentTopic, [2]=payload(base64), [3]=timestamp_nanos
    if (data.size() < 3) return;

    const QString contentTopic  = data[1].toString();
    const QString base64Payload = data[2].toString();
    const QString payload = QString::fromUtf8(QByteArray::fromBase64(base64Payload.toLatin1()));

    qDebug() << "ForumCommsPlugin: Message on topic:" << contentTopic;

    const ForumCodec::Post post = ForumCodec::decode(payload);
    if (!post.valid) {
        qDebug() << "ForumCommsPlugin: Non-forum message ignored on topic:" << contentTopic;
        return;
    }

    const QString forumName = m_topicToForum.value(contentTopic, post.forumId);

    emitEvent(QStringLiteral("forumPostReceived"), {
        post.postId,
        forumName,
        post.topicId,
        post.topicTitle,
        post.authorId,
        post.content,
        post.timestamp,
        post.type
    });
}

void ForumCommsPlugin::onMessageSent(const QVariantList& data)
{
    // [0]=requestId, [1]=messageHash, [2]=timestamp
    if (data.isEmpty()) return;

    const QString requestId = data[0].toString();
    const QString timestamp = data.size() > 2 ? data[2].toString() : QString();
    const QString postId    = m_pendingRequests.take(requestId);

    if (postId.isEmpty()) return;

    emitEvent(QStringLiteral("forumPostSent"), { requestId, postId, timestamp });
}

void ForumCommsPlugin::onMessageError(const QVariantList& data)
{
    // [0]=requestId, [1]=messageHash, [2]=errorMsg, [3]=timestamp
    if (data.isEmpty()) return;

    const QString requestId = data[0].toString();
    const QString errorMsg  = data.size() > 2 ? data[2].toString() : QStringLiteral("Unknown error");
    const QString timestamp = data.size() > 3 ? data[3].toString() : QString();

    m_pendingRequests.remove(requestId);
    emitEvent(QStringLiteral("forumPostError"), { requestId, errorMsg, timestamp });
}

void ForumCommsPlugin::onConnectionChanged(const QVariantList& data)
{
    // [0]=status, [1]=timestamp
    if (data.isEmpty()) return;
    emitEvent(QStringLiteral("forumConnectionChanged"),
              { data[0], data.size() > 1 ? data[1] : QVariant() });
}

void ForumCommsPlugin::emitEvent(const QString& eventName, const QVariantList& data)
{
    emit eventResponse(eventName, data);
}
