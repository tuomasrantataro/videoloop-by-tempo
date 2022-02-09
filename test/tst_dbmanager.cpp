#include <QtCore>
#include <QTest>

#include "dbmanager.h"

class TestDBManager : public QObject
{
    Q_OBJECT
private slots:
    void getTempo_data();
    void getTempo();
    void writeData_data();
    void writeData();
    void deleteTrack();

    void cleanup();
};

void TestDBManager::getTempo_data()
{
    QTest::addColumn<QString>("db_filename");
    QTest::addColumn<QString>("track_id");
    QTest::addColumn<double>("tempo");

    QTest::addRow("track found") << "test/data/trackdata.db" << "spotify:track:6lupPYC7m8khmxwFqfaiUQ" << 125.945953369141;
    QTest::addRow("track not found") << "test/data/trackdata.db" << "spotify:track:not_a_real_id" << 0.0;
    QTest::addRow("creating new file") << "test/data/new_trackdata.db" << "spotify:track:6lupPYC7m8khmxwFqfaiUQ" << 0.0;
}

void TestDBManager::getTempo()
{
    QFETCH(QString, db_filename);
    QFETCH(QString, track_id);
    QFETCH(double, tempo);

    DBManager db_manager(true, db_filename);
    QVERIFY2(abs(db_manager.getTempo(track_id) - tempo) < 0.01, "Wrong track tempo");
}

void TestDBManager::writeData_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("track_id_1");
    QTest::addColumn<double>("tempo_1");
    QTest::addColumn<QString>("track_id_2");
    QTest::addColumn<double>("tempo_2");
    QTest::addColumn<bool>("write_enabled");


    QTest::addRow("write enabled")  << "test/data/new_trackdata.db" << "my_own_id_1"
                                    << 136.0 << "my_own_id_2" << 104.3 << true;
    QTest::addRow("write disabled") << "test/data/new_trackdata.db" << "my_own_id_1"
                                    << 136.0 << "my_own_id_2" << 104.3 << false;
}

void TestDBManager::writeData()
{
    QFETCH(QString, filename);
    QFETCH(QString, track_id_1);
    QFETCH(double, tempo_1);
    QFETCH(QString, track_id_2);
    QFETCH(double, tempo_2);
    QFETCH(bool, write_enabled);

    MyTypes::TrackData data1, data2;
    data1.BPM = tempo_1;
    data1.trackId = track_id_1;
    data2.BPM = tempo_2;
    data2.trackId = track_id_2;

    DBManager dbManager(write_enabled, filename);

    dbManager.writeData(data1);
    dbManager.writeData(data2);

    if(!write_enabled) {
        QVERIFY2(dbManager.getTempo(track_id_1) == 0.0, "Tempo data written when it shouldn't");
        return;
    }

    QVERIFY2(abs(dbManager.getTempo(track_id_1) - tempo_1) < 0.01, "Wrong track 1 tempo");
    QVERIFY2(abs(dbManager.getTempo(track_id_2) - tempo_2) < 0.01, "Wrong track 2 tempo");

    data1.BPM = tempo_2;
    dbManager.writeData(data1);
    QVERIFY2(abs(dbManager.getTempo(track_id_1) - tempo_2) < 0.01, "Wrong track 1 tempo after update");
}

void TestDBManager::deleteTrack()
{
    DBManager dbManager(true, "test/data/new_trackdata.db");

    MyTypes::TrackData data1, data2;
    data1.BPM = 136.0;
    data1.trackId = "track_id_1";
    data1.confidence = 3.0;
    data2.BPM = 104.2;
    data2.trackId = "track_id_2";
    data2.confidence = 3.0;

    dbManager.writeData(data1);
    dbManager.writeData(data2);

    dbManager.deleteTrack(data1.trackId);

    QVERIFY2(dbManager.getTempo(data1.trackId) == 0.0, "Track not removed");

    // Test for deleting already deleted track
    dbManager.deleteTrack(data1.trackId);

    // Test that the other one still exists
    QVERIFY2(abs(dbManager.getTempo(data2.trackId) - 104.2) < 0.01, "Wrong data removed");

}

void TestDBManager::cleanup()
{
    QFile newDBFile("./test/data/new_trackdata.db");
    newDBFile.remove();
}

QTEST_MAIN(TestDBManager)
#include "tst_dbmanager.moc"