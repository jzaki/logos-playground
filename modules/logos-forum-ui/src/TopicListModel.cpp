#include "TopicListModel.h"

TopicListModel::TopicListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int TopicListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return m_items.size();
}

QVariant TopicListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const auto& item = m_items.at(index.row());
    switch (role) {
    case TopicIdRole:      return item.topicId;
    case TopicTitleRole:   return item.topicTitle;
    case AuthorIdRole:     return item.authorId;
    case LastActivityRole: return item.lastActivity;
    case PostCountRole:    return item.postCount;
    case UnreadCountRole:  return item.unreadCount;
    default:               return {};
    }
}

QHash<int, QByteArray> TopicListModel::roleNames() const
{
    return {
        { TopicIdRole,      "topicId" },
        { TopicTitleRole,   "topicTitle" },
        { AuthorIdRole,     "authorId" },
        { LastActivityRole, "lastActivity" },
        { PostCountRole,    "postCount" },
        { UnreadCountRole,  "unreadCount" }
    };
}

void TopicListModel::addTopic(const QString& topicId, const QString& topicTitle,
                               const QString& authorId, const QDateTime& lastActivity)
{
    if (contains(topicId)) return;
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append({ topicId, topicTitle, authorId, lastActivity, 1, 1 });
    endInsertRows();
}

void TopicListModel::bumpActivity(const QString& topicId, const QDateTime& when)
{
    int idx = indexOf(topicId);
    if (idx < 0) return;
    m_items[idx].lastActivity = when;
    m_items[idx].postCount++;
    emit dataChanged(index(idx), index(idx), { LastActivityRole, PostCountRole });
}

void TopicListModel::incrementUnread(const QString& topicId)
{
    int idx = indexOf(topicId);
    if (idx < 0) return;
    m_items[idx].unreadCount++;
    emit dataChanged(index(idx), index(idx), { UnreadCountRole });
}

void TopicListModel::clearUnread(const QString& topicId)
{
    int idx = indexOf(topicId);
    if (idx < 0 || m_items[idx].unreadCount == 0) return;
    m_items[idx].unreadCount = 0;
    emit dataChanged(index(idx), index(idx), { UnreadCountRole });
}

bool TopicListModel::contains(const QString& topicId) const
{
    return indexOf(topicId) >= 0;
}

int TopicListModel::indexOf(const QString& topicId) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].topicId == topicId) return i;
    }
    return -1;
}
