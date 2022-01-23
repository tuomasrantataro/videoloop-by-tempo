#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtCore>
#include <QtSql>

#include "types.h"

class DBManager : public QObject
{
    Q_OBJECT
public: 
    DBManager(bool saveTrackData);
    ~DBManager();

    bool createConnection();

    float getBPM(QString trackId) { return m_bpmData[trackId]; };

public slots:
    void writeBPM(MyTypes::TrackData& data);
    void deleteTrack(QString trackId);

private:
    void readBPMValues();

    QMap<QString, float> m_bpmData;
    bool m_saveTrackData;
};

#endif