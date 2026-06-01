#ifndef FORUM_UI_PLUGIN_H
#define FORUM_UI_PLUGIN_H

#include <QObject>
#include <QString>
#include <QtPlugin>
#include "forum_ui_interface.h"
#include "LogosViewPluginBase.h"

class LogosAPI;
class ForumBackend;

class ForumUiPlugin : public QObject,
                      public ForumUiInterface,
                      public ForumBackendViewPluginBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ForumUiInterface_iid FILE "../metadata.json")
    Q_INTERFACES(ForumUiInterface)

public:
    explicit ForumUiPlugin(QObject* parent = nullptr);
    ~ForumUiPlugin() override;

    QString name()    const override { return QStringLiteral("forum_ui"); }
    QString version() const override { return QStringLiteral("1.0.0"); }

    Q_INVOKABLE void initLogos(LogosAPI* api);

private:
    ForumBackend* m_backend = nullptr;
};

#endif
