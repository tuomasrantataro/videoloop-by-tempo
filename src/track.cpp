#include "track.h"

Track::Track(QObject* parent, bool saveTrackData, QString targetProgram, QStringList ignorePrograms)
    : m_parent(parent)
{
    m_dbManager = new DBManager(saveTrackData);
    connect(this, &Track::removeCurrentTrack, [=]{ m_dbManager->deleteTrack(m_currentTrackId); });

    m_pulseAudioWatcher = new PulseAudioWatcher(targetProgram, ignorePrograms);
    m_pulseAudioWatcher->startPolling(1000);
    connect(m_pulseAudioWatcher, &PulseAudioWatcher::invalidateData, this, &Track::invalidateTrackData);

    m_spotifyWatcher = new SpotifyWatcher();
    connect(m_spotifyWatcher, &SpotifyWatcher::trackChanged, [=] { trackCalculationNeeded(); });
    connect(m_spotifyWatcher, &SpotifyWatcher::trackChanged, this, &Track::updateTrackInfo);
    connect(m_spotifyWatcher, &SpotifyWatcher::newTrackId, this, &Track::findNewTempo);
    connect(m_spotifyWatcher, &SpotifyWatcher::invalidateData, this, &Track::invalidateTrackData);
}

Track::~Track()
{
    delete m_dbManager;
    delete m_pulseAudioWatcher;
    delete m_spotifyWatcher;
}

void Track::updateTrackInfo(QString oldTrackId, QString oldArtist, QString oldTitle)
{
    m_trackData.trackId = oldTrackId;
    m_trackData.artist = oldArtist;
    m_trackData.title = oldTitle;
}

void Track::findNewTempo(QString newTrackId)
{
    m_currentTrackId = newTrackId;
    double newTempo = m_dbManager->getTempo(m_currentTrackId);
    if (newTempo > 0.0) {
        emit disableAutoTempo(true);
        emit trackTempoFound(newTempo);
    }
    else {
        emit disableAutoTempo(false);
    }
}

void Track::invalidateTrackData(QString reason)
{
    m_invalidTrackData = true;
    m_trackDataInvalidationReasons.append(reason); 
}

void Track::saveTrackData()
{
    if (m_trackData.confidence < 0.5) {
        invalidateTrackData("Too low tempo analysis confidence");
    }
    
    if (!m_invalidTrackData && QString("").compare(m_trackData.trackId)) {
        m_dbManager->writeData(m_trackData);
    }
    else {
        m_trackDataInvalidationReasons.removeDuplicates();
        qDebug("Track data not saved. Reasons during last track:");
        for (auto item : m_trackDataInvalidationReasons) {
            qDebug() << " " << qPrintable(item);
        }
        m_trackDataInvalidationReasons.clear();
    }
    // Reset track data invalidation for the next track
    m_invalidTrackData = false;
}

void Track::updateTrackTempo(const MyTypes::TempoData& data)
{
    m_trackData = MyTypes::TrackData(data,
                                     m_trackData.trackId,
                                     m_trackData.artist,
                                     m_trackData.title);

    saveTrackData();
}