#ifndef DBUSWATCHER_H
#define DBUSWATCHER_H

#include <QtCore>
#include <QtDBus>

class DBusWatcher : public QObject
{
    Q_OBJECT
public:
    DBusWatcher();
    ~DBusWatcher();

public slots:
    void propertiesChanged(QString interface, QMap<QString, QVariant> signalData, QStringList l);

private:
    QDBusConnection m_bus = QDBusConnection::sessionBus();
};

#endif