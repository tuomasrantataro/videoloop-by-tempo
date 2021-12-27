#include "dbuswatcher.h"

DBusWatcher::DBusWatcher()
{
    if (!m_bus.isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }

    m_bus.connect("org.mpris.MediaPlayer2.spotify", "/org/mpris/MediaPlayer2",
        "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(propertiesChanged(QString, QMap<QString, QVariant>, QStringList)));

}

DBusWatcher::~DBusWatcher()
{

}

void DBusWatcher::propertiesChanged(QString interface, QMap<QString, QVariant> signalData, QStringList l)
{
    if (!signalData.contains("Metadata")) {
        qDebug("Spotify metadata not found when properties changed. Keys in map:");
        qDebug() << signalData.keys();
        return;
    }

    QVariant contents = signalData["Metadata"];
    if (QString("QDBusArgument").compare(contents.typeName())) {
        qDebug("Data is not a QDBusArgument");
        return;
    }

    const QDBusArgument &dbusargs = contents.value<QDBusArgument>();

    if (dbusargs.currentType() != 4) {
        // Type 4 indicates Map-like
        qDebug("Data contained in QDBusArgument is not Map-like: %d", dbusargs.currentType());
        return;
    }

    QMap<QString, QVariant> newMap;

    dbusargs.beginMap();
    while (!dbusargs.atEnd()) {
        QString key;
        QVariant arg;
        dbusargs.beginMapEntry();
        dbusargs >> key >> arg;
        dbusargs.endMapEntry();
        newMap.insert(key, arg);
    }

    /*
    QStringList keys = newMap.keys();
    for (auto key = keys.begin(); key != keys.end(); key++) {
        qDebug() << *key;
    }*/

    QString artist = newMap["xesam:artist"].toString();
    qDebug() << "Track artist: " << artist;

    QString title = newMap["xesam:title"].toString();
    qDebug() << "Track title: " << title;

    QString trackid = newMap["mpris:trackid"].toString();
    qDebug() << "trackid: " << trackid << '\n';
}