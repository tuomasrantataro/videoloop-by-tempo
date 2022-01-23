#ifndef SPOTIFYWATCHER_H
#define SPOTIFYWATCHER_H

#include <QtCore>
#include <QtDBus>

#include "types.h"

class SpotifyWatcher : public QObject
{
    Q_OBJECT
public:
    SpotifyWatcher();
    ~SpotifyWatcher();

public slots:
    void propertiesChanged(QString interface, QMap<QString, QVariant> signalData, QStringList l);

signals:
    void trackChanged(QString oldTrackId, QString oldArtist, QString oldTitle, QString newTrackId);
    void invalidateData(QString reason);

private:
    QDBusConnection m_bus = QDBusConnection::sessionBus();

    QString m_oldTrackId;
    QString m_oldArtist;
    QString m_oldTitle;

    qint64 m_lastTrackChange = QDateTime::currentMSecsSinceEpoch();
    qint64 m_spotifyLength = 0;
};

#endif