#include "dbmanager.h"

DBManager::DBManager(bool saveTrackData) : m_saveTrackData(saveTrackData)
{
    if (!createConnection()) {
        qWarning("Database connection failed. Is the QT SQLITE driver installed?");
        return;
    }

    readBPMValues();
}

DBManager::~DBManager()
{

}


bool DBManager::createConnection()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tracktempos.db");

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

        return true;
    }

}

void DBManager::writeBPM(MyTypes::TrackData& data)
{

    // Do not save data if flag '-n' is used
    if (!m_saveTrackData) {
        return;
    }

    QSqlQuery query;
    QString track = data.trackId;
    query.prepare("SELECT * FROM trackdata WHERE EXISTS ( SELECT trackid FROM trackdata WHERE trackid=:trackid );");
    query.bindValue(":trackid", track);
    query.exec();

    query.next();

    // Do not overwrite already existing track data
    if (query.isValid()) {
        return;
    }

    query.clear();

    query.prepare("INSERT INTO trackdata (trackid, bpm, confidence, peak1, weight1, spread1, peak2, weight2, spread2, artist, title) "
               "VALUES (:trackid, :bpm, :confidence, :peak1, :weight1, :spread1, :peak2, :weight2, :spread2, :artist, :title);");
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
}

void DBManager::readBPMValues()
{
    QSqlQuery query;
    query.prepare("SELECT trackid, bpm FROM trackdata;");
    query.exec();

    while (query.next()) {
        QString key = query.value(0).toString();
        float bpm = query.value(1).toFloat();
        m_bpmData.insert(key, bpm);
    }
}