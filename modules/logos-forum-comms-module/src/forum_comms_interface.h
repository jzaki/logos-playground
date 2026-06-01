#ifndef FORUM_COMMS_INTERFACE_H
#define FORUM_COMMS_INTERFACE_H

#include "interface.h"
#include "logos_types.h"

class ForumCommsInterface : public PluginInterface
{
public:
    virtual ~ForumCommsInterface() = default;

    // Delivery lifecycle
    virtual LogosResult initDelivery(const QString& configJson) = 0;
    virtual LogosResult stop() = 0;

    // Subscription management
    virtual LogosResult subscribeToForum(const QString& forumName) = 0;
    virtual LogosResult subscribeToTopic(const QString& forumName, const QString& topicId) = 0;
    virtual LogosResult unsubscribeFromForum(const QString& forumName) = 0;
    virtual LogosResult unsubscribeFromTopic(const QString& forumName, const QString& topicId) = 0;

    // Publishing
    virtual LogosResult publishNewTopic(const QString& forumName, const QString& topicTitle,
                                        const QString& authorId, const QString& content) = 0;
    virtual LogosResult publishPost(const QString& forumName, const QString& topicId,
                                    const QString& authorId, const QString& content) = 0;
};

#define ForumCommsInterface_iid "org.logos.ForumCommsInterface"
Q_DECLARE_INTERFACE(ForumCommsInterface, ForumCommsInterface_iid)

#endif
