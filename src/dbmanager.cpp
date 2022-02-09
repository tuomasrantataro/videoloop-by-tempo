#include "dbmanager.h"

#include <QtSql>

DBManager::DBManager(bool saveTrackData, QString fileName)
    : m_saveTrackData(saveTrackData), m_fileName(fileName)
{
    if (!createConnection()) {
        qWarning("Database connection failed. Is the QT SQLITE driver installed?");
        return;
    }

    readBPMValues();
}

DBManager::~DBManager()
{
    QSqlDatabase::removeDatabase(m_connectionName);
}


bool DBManager::createConnection()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(m_fileName);

    if (!db.open()) {
        return false;
    }
    else {
        QSqlQuery query;
        query.exec("CREATE TABLE trackdata "
                   "(trackid TEXT, "
                   "bpm REAL, "
                   "confidence REAL, "
                   "peak1 REAL, "
                   "weight1 REAL, "
                   "spread1 REAL, "
                   "peak2 REAL, "
                   "weight2 REAL, "
                   "spread2 REAL, "
                   "artist TEXT, "
                   "title TEXT)");

        m_connectionName = db.connectionName();
        return true;
    }

}

void DBManager::writeData(MyTypes::TrackData& data)
{

    // Do not save data if flag '-n' is used
    if (!m_saveTrackData) {
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT confidence FROM trackdata WHERE trackid=:trackid;");
    query.bindValue(":trackid", data.trackId);
    query.exec();

    query.next();

    bool update = false;
    if (query.isValid()) {
        double dbConfidence = query.value(0).toDouble();

        if (data.confidence < dbConfidence) {
            return;
        }
        else {
            update = true;
        }
    }

    query.clear();

    qDebug("Adding tempo data to database:\n"
            "  %s | %.1f bpm | confidence: %.2f\n  %s - %s",
            qPrintable(data.trackId),
            data.BPM,
            data.confidence,
            qPrintable(data.artist),
            qPrintable(data.title));

    if (!update) {
        query.prepare("INSERT INTO trackdata (trackid, bpm, confidence, peak1, weight1, spread1, peak2, weight2, spread2, artist, title) "
               "VALUES (:trackid, :bpm, :confidence, :peak1, :weight1, :spread1, :peak2, :weight2, :spread2, :artist, :title);");
    }
    else {
        query.prepare("UPDATE trackdata SET "
                "trackid=:trackid, "
                "bpm=:bpm, "
                "confidence=:confidence, "
                "peak1=:peak1, "
                "weight1=:weight1, "
                "spread1=:spread1, "
                "peak2=:peak2, "
                "weight2=:weight2, "
                "spread2=:spread2, "
                "artist=:artist, "
                "title=:title "
                "WHERE trackid=:trackid;");
    }
    query.bindValue(":trackid", data.trackId);
    query.bindValue(":bpm", data.BPM);
    query.bindValue(":confidence", data.confidence);
    query.bindValue(":peak1", data.peak1);
    query.bindValue(":weight1", data.power1);
    query.bindValue(":spread1", data.spread1);
    query.bindValue(":peak2", data.peak2);
    query.bindValue(":weight2", data.power2);
    query.bindValue(":spread2", data.spread2);
    query.bindValue(":artist", data.artist);
    query.bindValue(":title", data.title);

    bool ret = query.exec();
    
    if (!ret) {
        qWarning("Error executing the track data insert query:\n\t%s", qPrintable(query.lastQuery()));
        qWarning("Error:\n\t%s", qPrintable(query.lastError().text()));
    }
    else {
        readBPMValues();    // update m_bpmData with the newest entry
    }
}

void DBManager::readBPMValues()
{
    QSqlQuery query;
    query.prepare("SELECT trackid, bpm FROM trackdata;");
    query.exec();

    m_bpmData.clear();

    while (query.next()) {
        QString key = query.value(0).toString();
        double bpm = query.value(1).toDouble();
        m_bpmData.insert(key, bpm);
    }
}

void DBManager::deleteTrack(QString trackId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM trackdata WHERE trackid=:trackid;");
    query.bindValue(":trackid", trackId);

    bool ret = query.exec();

    if (!ret) {
        qWarning("Error executing data deletion query:\n\t%s", qPrintable(query.lastQuery()));
        qWarning("Error:\n\t%s", qPrintable(query.lastError().text()));
    }
    else {
        readBPMValues();    // remove the track also from m_bpmData
    }
}