#include "PostListModel.h"

PostListModel::PostListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int PostListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant PostListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const auto& item = m_items.at(index.row());
    switch (role) {
    case PostIdRole:    return item.postId;
    case AuthorIdRole:  return item.authorId;
    case ContentRole:   return item.content;
    case TimestampRole: return item.timestamp;
    case IsMeRole:      return item.isMe;
    default:            return {};
    }
}

QHash<int, QByteArray> PostListModel::roleNames() const
{
    return {
        { PostIdRole,    "postId" },
        { AuthorIdRole,  "authorId" },
        { ContentRole,   "content" },
        { TimestampRole, "timestamp" },
        { IsMeRole,      "isMe" }
    };
}

void PostListModel::addPost(const QString& postId, const QString& authorId,
                             const QString& content, const QDateTime& timestamp, bool isMe)
{
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append({ postId, authorId, content, timestamp, isMe });
    endInsertRows();
}

void PostListModel::addPosts(QVector<PostItem> items)
{
    const int n = items.size();
    if (n == 0) return;
    const int firstRow = m_items.size();
    beginInsertRows(QModelIndex(), firstRow, firstRow + n - 1);
    m_items += std::move(items);
    endInsertRows();
}

void PostListModel::clear()
{
    if (m_items.isEmpty()) return;
    beginResetModel();
    m_items.clear();
    endResetModel();
}
