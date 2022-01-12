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
    // interfaceName: the interface where properties changed (org.mpris.MediaPlayer2.Player)
    // signalData: map of information received. structure of the map:
    // QVariantMap {
    //      {
    //          key: QString "Metadata"
    //          QVariant: type Map<QString, QVariant>
    //              {
    //                  key: QString "mpris:trackid"
    //                  QVariant: QString "spotify:track:3E4pe9Bzbt6emadOMBieoJ"
    //              }
    //              {
    //                  key: QString "mpris:length"
    //                  QVariant:    qlonglong 238510000
    //              }
    //              {
    //                  key: QString "xesam:title"
    //                  QVariant:    QString "Song title"
    //              }
    //              {
    //                  key: QString "xesam:artist"
    //                  QVariant:    QString "Artist name"
    //              }
    //
    //              ... other entries
    //      }
    //      {
    //          key: QString "PlaybackStatus"
    //          QVariant: QString "Playing"
    //      }
    // }
    //
    //
    // All properties exported by Spotify:
    // "mpris:artUrl"
    // "mpris:length"
    // "mpris:trackid"
    // "xesam:album"
    // "xesam:albumArtist"
    // "xesam:artist"
    // "xesam:autoRating"
    // "xesam:discNumber"
    // "xesam:title"
    // "xesam:trackNumber"
    // "xesam:url"
    //
    //

    if (!signalData.contains("Metadata")) {
        qDebug("Spotify metadata not found when properties changed. Keys in map:");
        qDebug() << signalData.keys();
        return;
    }

    QVariant playbackStatus = signalData["PlaybackStatus"];
    if (!QString("Paused").compare(playbackStatus.toString())) {
        // Invalidate song audio data if it was paused
        emit invalidateData();
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

    QString newTrackId = newMap["mpris:trackid"].toString();
    QString newArtist = newMap["xesam:artist"].toString();
    QString newTitle = newMap["xesam:title"].toString();

    if (newTrackId.compare(m_oldTrackId)) {

        emit trackChanged(m_oldTrackId, m_oldArtist, m_oldTitle, newTrackId);

        quint64 previousTrackChange = m_lastTrackChange;
        m_lastTrackChange = QDateTime::currentMSecsSinceEpoch();
        quint64 trackLength = m_lastTrackChange - previousTrackChange;
        
        qint64 diff = abs((long)trackLength - (long)m_spotifyLength);

        if (diff > 1500) {
            emit invalidateData();
        }

        m_spotifyLength = newMap["mpris:length"].toULongLong()/1000;   // change from usec to msec

        m_oldTrackId = newTrackId;
        m_oldArtist = newArtist;
        m_oldTitle = newTitle;

    }
}