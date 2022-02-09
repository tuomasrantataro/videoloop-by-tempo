#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtCore>

#include "types.h"

class DBManager : public QObject
{
    Q_OBJECT
public: 
    DBManager(bool saveTrackData, QString fileName = "trackdata.db");
    ~DBManager();

    double getTempo(QString trackId) { return m_bpmData[trackId]; };

public slots:
    void writeData(MyTypes::TrackData& data);
    void deleteTrack(QString trackId);

private:
    bool createConnection();

    void readBPMValues();

    QMap<QString, double> m_bpmData;
    bool m_saveTrackData;
    QString m_fileName;

    QString m_connectionName;
};

#endif