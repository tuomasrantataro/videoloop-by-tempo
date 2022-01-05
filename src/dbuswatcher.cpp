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
    }

    QString artist = newMap["xesam:artist"].toString();
    qDebug() << "Track artist: " << artist;

    QString title = newMap["xesam:title"].toString();
    qDebug() << "Track title: " << title;
    */
    QString trackid = newMap["mpris:trackid"].toString();
    
    //qDebug() << "trackid: " << trackid << '\n';
    

    if (trackid.compare(m_trackid)) {
        emit trackChanged(m_trackid);

        quint64 previousTrackChange = m_lastTrackChange;
        m_lastTrackChange = QDateTime::currentMSecsSinceEpoch();
        quint64 trackLength = m_lastTrackChange - previousTrackChange;
        
        quint64 oldLength = m_spotifyLength;
        
        //qDebug() << "time diff: " << trackLength << " spotify said: " << oldLength;;
        //qDebug() << "Track changed from " << m_trackid << " to " << trackid;
        
        m_spotifyLength = newMap["mpris:length"].toULongLong()/1000;   // change from usec to msec

        QFile saveFile("tracklengthdata.csv");

        if (saveFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&saveFile);

            stream << m_trackid << ',' << trackLength << ',' << oldLength << '\n';

            saveFile.close();

        }

        m_trackid = trackid;

    }
}