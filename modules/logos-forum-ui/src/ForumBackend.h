#ifndef FORUM_BACKEND_H
#define FORUM_BACKEND_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include "rep_ForumBackend_source.h"
#include "logos_api.h"
#include "logos_sdk.h"
#include "logos_types.h"
#include "TopicListModel.h"
#include "PostListModel.h"

class ForumBackend : public ForumBackendSimpleSource
{
    Q_OBJECT
    Q_PROPERTY(TopicListModel* topicModel READ topicModel CONSTANT)
    Q_PROPERTY(PostListModel*  postModel  READ postModel  CONSTANT)

public:
    explicit ForumBackend(LogosAPI* logosAPI = nullptr, QObject* parent = nullptr);
    ~ForumBackend() override;

    TopicListModel* topicModel() const;
    PostListModel*  postModel()  const;

public slots:
    void initForum() override;
    void stopForum() override;
    void selectTopic(QString topicId) override;
    void startNewTopic(QString topicTitle, QString content) override;
    void postReply(QString content) override;

private:
    void setupEventHandlers();
    void loadTopicPosts(const QString& topicId);

    // forum_comms_module event handlers
    void onForumPostReceived(const QVariantList& data);
    void onForumPostSent(const QVariantList& data);
    void onForumPostError(const QVariantList& data);
    void onForumConnectionChanged(const QVariantList& data);

    LogosAPI*      m_logosAPI;
    LogosModules*  m_logos;

    TopicListModel* m_topicModel;
    PostListModel*  m_postModel;

    // Per-topic post storage for switching views
    struct PostInfo {
        QString   postId;
        QString   authorId;
        QString   content;
        QDateTime timestamp;
        bool      isMe;
    };
    QMap<QString, QList<PostInfo>> m_posts;
};

#endif
