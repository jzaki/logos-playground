#include "forum_ui_plugin.h"
#include "ForumBackend.h"

#include <QDebug>

ForumUiPlugin::ForumUiPlugin(QObject* parent)
    : QObject(parent)
{
}

ForumUiPlugin::~ForumUiPlugin()
{
    if (m_backend)
        m_backend->stopForum();
}

void ForumUiPlugin::initLogos(LogosAPI* api)
{
    if (m_backend) return;
    m_backend = new ForumBackend(api, this);
    setBackend(m_backend);
    qDebug() << "ForumUiPlugin: backend initialised";
}
