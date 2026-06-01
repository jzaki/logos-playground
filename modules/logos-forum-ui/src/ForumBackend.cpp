#include "ForumBackend.h"
#include "ForumConfig.h"

#include <QDebug>
#include <QUuid>
#include <QTimer>

ForumBackend::ForumBackend(LogosAPI* logosAPI, QObject* parent)
    : ForumBackendSimpleSource(parent)
    , m_logosAPI(logosAPI ? logosAPI : new LogosAPI("forum_ui", this))
    , m_logos(new LogosModules(m_logosAPI))
    , m_topicModel(new TopicListModel(this))
    , m_postModel(new PostListModel(this))
{
    setForumStatus(ForumBackendSimpleSource::Disconnected);
    setMyIdentity(QString());
    setStatusMessage(QStringLiteral("Ready"));
    setCurrentForumName(QString());
    setCurrentTopicId(QString());

    // Generate a stable per-session identity
    setMyIdentity(QUuid::createUuid().toString(QUuid::WithoutBraces).left(12));

    QTimer::singleShot(0, this, [this]() {
        setupEventHandlers();
        initForum();
    });
}

ForumBackend::~ForumBackend()
{
    if (forumStatus() == ForumBackendSimpleSource::Connected && m_logos)
        m_logos->forum_comms_module.stop();
    delete m_logos;
}

TopicListModel* ForumBackend::topicModel() const { return m_topicModel; }
PostListModel*  ForumBackend::postModel()  const { return m_postModel; }

// ── Slot implementations ─────────────────────────────────────────────────────

void ForumBackend::initForum()
{
    if (!m_logos) {
        emit error(QStringLiteral("LogosAPI not available"));
        return;
    }

    if (forumStatus() != ForumBackendSimpleSource::Disconnected) {
        setStatusMessage(QStringLiteral("Forum already initialised"));
        return;
    }

    const QString config = ForumConfig::buildDeliveryConfigJson();
    qDebug() << "ForumBackend: Starting with config:" << config;

    setForumStatus(ForumBackendSimpleSource::Connecting);
    setStatusMessage(QStringLiteral("Connecting to network..."));

    LogosResult r = m_logos->forum_comms_module.initDelivery(config);
    if (!r.success) {
        setForumStatus(ForumBackendSimpleSource::Disconnected);
        setStatusMessage(QStringLiteral("Failed to start delivery"));
        emit error(QStringLiteral("Failed to initialise delivery node"));
        return;
    }

    // Subscribe to the default forum; connection events confirm when we're live.
    const QString forumName = ForumConfig::defaultForumName();
    LogosResult sub = m_logos->forum_comms_module.subscribeToForum(forumName);
    if (!sub.success)
        qWarning() << "ForumBackend: Failed to subscribe to forum:" << forumName;
    setCurrentForumName(forumName);
    setStatusMessage(QString("Subscribing to %1...").arg(forumName));
}

void ForumBackend::stopForum()
{
    if (!m_logos) return;

    if (forumStatus() == ForumBackendSimpleSource::Disconnected) return;

    setForumStatus(ForumBackendSimpleSource::Stopping);
    setStatusMessage(QStringLiteral("Stopping..."));
    m_logos->forum_comms_module.stop();
    setForumStatus(ForumBackendSimpleSource::Disconnected);
    setStatusMessage(QStringLiteral("Disconnected"));
}

void ForumBackend::selectTopic(QString topicId)
{
    if (topicId == currentTopicId()) return;

    setCurrentTopicId(topicId);
    m_topicModel->clearUnread(topicId);
    loadTopicPosts(topicId);

    // Subscribe to this topic's reply feed if not already done
    if (!currentForumName().isEmpty()) {
        LogosResult sub = m_logos->forum_comms_module.subscribeToTopic(currentForumName(), topicId);
        if (!sub.success)
            qWarning() << "ForumBackend: Failed to subscribe to topic:" << topicId;
    }
}

void ForumBackend::startNewTopic(QString topicTitle, QString content)
{
    if (forumStatus() != ForumBackendSimpleSource::Connected || !m_logos) {
        emit error(QStringLiteral("Not connected to network"));
        return;
    }

    if (topicTitle.trimmed().isEmpty() || content.trimmed().isEmpty()) {
        emit error(QStringLiteral("Title and content cannot be empty"));
        return;
    }

    qDebug() << "ForumBackend: Starting new topic:" << topicTitle;

    LogosResult r = m_logos->forum_comms_module.publishNewTopic(
        currentForumName(), topicTitle, myIdentity(), content);

    if (!r.success) {
        emit error(QStringLiteral("Failed to publish new topic"));
        return;
    }

    setStatusMessage(QStringLiteral("Publishing new topic..."));
}

void ForumBackend::postReply(QString content)
{
    if (forumStatus() != ForumBackendSimpleSource::Connected || !m_logos) {
        emit error(QStringLiteral("Not connected to network"));
        return;
    }

    if (currentTopicId().isEmpty()) {
        emit error(QStringLiteral("No topic selected"));
        return;
    }

    if (content.trimmed().isEmpty()) return;

    qDebug() << "ForumBackend: Posting reply to topic:" << currentTopicId();

    // Optimistic local insert
    const QDateTime now = QDateTime::currentDateTime();
    const QString tempPostId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_posts[currentTopicId()].append({ tempPostId, myIdentity(), content, now, true });
    m_postModel->addPost(tempPostId, myIdentity(), content, now, true);
    m_topicModel->bumpActivity(currentTopicId(), now);

    LogosResult r = m_logos->forum_comms_module.publishPost(
        currentForumName(), currentTopicId(), myIdentity(), content);

    if (!r.success)
        emit error(QStringLiteral("Failed to send reply"));
}

// ── Private helpers ──────────────────────────────────────────────────────────

void ForumBackend::setupEventHandlers()
{
    if (!m_logos) return;

    auto safeInvoke = [this](auto handler) {
        return [this, handler](const QVariantList& data) {
            QMetaObject::invokeMethod(this, [this, handler, data]() {
                (this->*handler)(data);
            }, Qt::QueuedConnection);
        };
    };

    m_logos->forum_comms_module.on("forumPostReceived",      safeInvoke(&ForumBackend::onForumPostReceived));
    m_logos->forum_comms_module.on("forumPostSent",          safeInvoke(&ForumBackend::onForumPostSent));
    m_logos->forum_comms_module.on("forumPostError",         safeInvoke(&ForumBackend::onForumPostError));
    m_logos->forum_comms_module.on("forumConnectionChanged", safeInvoke(&ForumBackend::onForumConnectionChanged));

    qDebug() << "ForumBackend: Event handlers registered";
}

void ForumBackend::loadTopicPosts(const QString& topicId)
{
    m_postModel->clear();
    if (!m_posts.contains(topicId)) return;

    const QList<PostInfo>& posts = m_posts[topicId];
    QVector<PostItem> rows;
    rows.reserve(posts.size());
    for (const PostInfo& p : posts)
        rows.append({ p.postId, p.authorId, p.content, p.timestamp, p.isMe });
    m_postModel->addPosts(std::move(rows));
}

// ── Event handlers ───────────────────────────────────────────────────────────

void ForumBackend::onForumPostReceived(const QVariantList& data)
{
    // [0] postId, [1] forumName, [2] topicId, [3] topicTitle,
    // [4] authorId, [5] content, [6] timestamp, [7] type
    if (data.size() < 8) return;

    const QString postId     = data[0].toString();
    const QString forumName  = data[1].toString();
    const QString topicId    = data[2].toString();
    const QString topicTitle = data[3].toString();
    const QString authorId   = data[4].toString();
    const QString content    = data[5].toString();
    const QString tsStr      = data[6].toString();
    const QString type       = data[7].toString();

    const QDateTime ts = tsStr.isEmpty()
        ? QDateTime::currentDateTime()
        : QDateTime::fromString(tsStr, Qt::ISODate);
    const bool isMe = (authorId == myIdentity());

    qDebug() << "ForumBackend: Post received, type:" << type << "topicId:" << topicId;

    if (type == QLatin1String("new_topic")) {
        if (!m_topicModel->contains(topicId)) {
            m_topicModel->addTopic(topicId, topicTitle, authorId, ts);
        }
        // Store the opening post under its topic
        m_posts[topicId].append({ postId, authorId, content, ts, isMe });

        if (topicId == currentTopicId())
            m_postModel->addPost(postId, authorId, content, ts, isMe);
        else
            m_topicModel->incrementUnread(topicId);

        emit topicCreated(topicId, topicTitle);
        setStatusMessage(QString("New topic: %1").arg(topicTitle));
    } else {
        // Reply to existing topic
        m_posts[topicId].append({ postId, authorId, content, ts, isMe });
        m_topicModel->bumpActivity(topicId, ts);

        if (topicId == currentTopicId())
            m_postModel->addPost(postId, authorId, content, ts, isMe);
        else
            m_topicModel->incrementUnread(topicId);

        setStatusMessage(QString("New reply in topic by %1").arg(authorId.left(8)));
    }
}

void ForumBackend::onForumPostSent(const QVariantList& data)
{
    qDebug() << "ForumBackend: Post sent:" << data;
    setStatusMessage(QStringLiteral("Post delivered"));
}

void ForumBackend::onForumPostError(const QVariantList& data)
{
    const QString errMsg = data.size() > 1 ? data[1].toString() : QStringLiteral("Send error");
    qWarning() << "ForumBackend: Post error:" << errMsg;
    setStatusMessage(QString("Post error: %1").arg(errMsg));
    emit error(errMsg);
}

void ForumBackend::onForumConnectionChanged(const QVariantList& data)
{
    const QString status = data.size() > 0 ? data[0].toString() : QString();
    qDebug() << "ForumBackend: Connection state:" << status;

    if (status.contains(QLatin1String("connected"), Qt::CaseInsensitive)
        || status.contains(QLatin1String("online"), Qt::CaseInsensitive)) {
        setForumStatus(ForumBackendSimpleSource::Connected);
        setStatusMessage(QStringLiteral("Connected to network"));
    } else if (status.contains(QLatin1String("disconnect"), Qt::CaseInsensitive)) {
        if (forumStatus() != ForumBackendSimpleSource::Stopping) {
            setForumStatus(ForumBackendSimpleSource::Disconnected);
            setStatusMessage(QStringLiteral("Disconnected"));
        }
    } else {
        setStatusMessage(QString("Network: %1").arg(status));
    }
}
