#ifndef TRACK_H
#define TRACK_H

#include <QtCore>
#include "dbmanager.h"
#include "pulseaudiowatcher.h"
#include "spotifywatcher.h"


class Track : public QObject
{
    Q_OBJECT
public:
    Track(QObject* parent, bool saveTrackData, QString targetProgram, QStringList ignorePrograms);
    ~Track();

signals:
    void trackCalculationNeeded();
    void disableAutoTempo(bool disable);
    void trackTempoFound(double tempo);
    void removeCurrentTrack();

public slots:
    void invalidateTrackData(QString reason);
    void updateTrackTempo(const MyTypes::TempoData& data);

private slots:
    void updateTrackInfo(QString oldTrackId, QString oldArtist, QString oldTitle); //, QString newTrackId);
    void findNewTempo(QString newTrackId);

private:
    QObject* m_parent;
    bool m_saveTrackData;

    DBManager* m_dbManager;
    PulseAudioWatcher* m_pulseAudioWatcher;
    SpotifyWatcher* m_spotifyWatcher;

    bool m_invalidTrackData;
    QStringList m_trackDataInvalidationReasons;

    void saveTrackData();
    MyTypes::TrackData m_trackData;

    QString m_currentTrackId;
};

#endif