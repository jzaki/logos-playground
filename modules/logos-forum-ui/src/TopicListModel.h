#ifndef TOPIC_LIST_MODEL_H
#define TOPIC_LIST_MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>
#include <QVector>

struct TopicItem {
    QString   topicId;
    QString   topicTitle;
    QString   authorId;
    QDateTime lastActivity;
    int       postCount   = 0;
    int       unreadCount = 0;
};

class TopicListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        TopicIdRole      = Qt::UserRole + 1,
        TopicTitleRole,
        AuthorIdRole,
        LastActivityRole,
        PostCountRole,
        UnreadCountRole
    };

    explicit TopicListModel(QObject* parent = nullptr);

    int     rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addTopic(const QString& topicId, const QString& topicTitle,
                  const QString& authorId, const QDateTime& lastActivity);
    void bumpActivity(const QString& topicId, const QDateTime& when);
    void incrementUnread(const QString& topicId);
    void clearUnread(const QString& topicId);
    bool contains(const QString& topicId) const;
    int  indexOf(const QString& topicId) const;

private:
    QVector<TopicItem> m_items;
};

#endif
