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

signals:
    void trackChanged(QString oldTrackId, QString oldArtist, QString oldTitle);
    void newTrackId(QString newTrackId);
    void invalidateData(QString reason);

private slots:
    void propertiesChanged(QString interface, QMap<QString, QVariant> signalData, QStringList l);

private:
    QDBusConnection m_bus = QDBusConnection::sessionBus();

    QString m_oldTrackId;
    QString m_oldArtist;
    QString m_oldTitle;
    QString m_newTrackId;

    qint64 m_lastTrackChange = QDateTime::currentMSecsSinceEpoch();
    qint64 m_spotifyLength = 0;
};

#endif