#ifndef POST_LIST_MODEL_H
#define POST_LIST_MODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>
#include <QVector>

struct PostItem {
    QString   postId;
    QString   authorId;
    QString   content;
    QDateTime timestamp;
    bool      isMe = false;
};

class PostListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        PostIdRole    = Qt::UserRole + 1,
        AuthorIdRole,
        ContentRole,
        TimestampRole,
        IsMeRole
    };

    explicit PostListModel(QObject* parent = nullptr);

    int     rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addPost(const QString& postId, const QString& authorId,
                 const QString& content, const QDateTime& timestamp, bool isMe);
    void addPosts(QVector<PostItem> items);
    void clear();

private:
    QVector<PostItem> m_items;
};

#endif
