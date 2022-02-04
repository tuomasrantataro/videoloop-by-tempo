#include <QTest>
#include "settings.h"

class TestSettings: public QObject
{
    Q_OBJECT
private slots:
    void readSettings_data();
    void readSettings();

    void writeSettings();

    void getLoopInfo_data();
    void getLoopInfo();

    void setLoopSettings_data();
    void setLoopSettings();

    void cleanupTestCase();
};

void TestSettings::readSettings_data()
{
    // Test only some settings of different types
    QTest::addColumn<QString>("filename");
    QTest::addColumn<double>("threshold");
    QTest::addColumn<bool>("limit_tempo");
    QTest::addColumn<QStringList>("ignore_list");
    QTest::addColumn<QString>("video_name");
    QTest::addColumn<QJsonObject>("loop_settings");

    QStringList twoItems;
    twoItems.append("VLC");
    twoItems.append("Firefox");
    
    QTest::newRow("valid file") << "./test/data/settings_valid.JSON" << 3.5 << true << twoItems << "gandalf";
    QTest::newRow("wrong kind of file") << "./test/data/settings_invalid.txt" << 3.0 << false << QStringList() << "";
    QTest::newRow("invalid datatype") << "./test/data/settings_invalid_datatype.JSON" << 3.0 << true << QStringList() << "gandalf";
    QTest::newRow("missing file") << "./test/data/file_does_not_exists.exe" << 3.0 << false << QStringList() << "";
}

void TestSettings::readSettings()
{
    Settings settings;
    QFETCH(QString, filename);
    QFETCH(double, threshold);
    QFETCH(bool, limit_tempo);
    QFETCH(QStringList, ignore_list);
    QFETCH(QString, video_name);

    settings.readSettings(filename);

    QCOMPARE(settings.getConfidenceThreshold(), threshold);
    QCOMPARE(settings.getLimitTempo(), limit_tempo);
    QCOMPARE(settings.getPulseAudioIgnoreList(), ignore_list);
    QCOMPARE(settings.getVideoLoopName(), video_name);
}

void TestSettings::getLoopInfo_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<bool>("add_reverse");
    QTest::addColumn<double>("multiplier");

    QTest::newRow("valid data") << "./test/data/settings_valid.JSON" << true << 0.5;
    QTest::newRow("invalid data") << "./test/data/settings_invalid.JSON" << false << 1.0;
    QTest::newRow("wrong datatype") << "./test/data/settings_invalid_datatype.JSON" << false << 1.0;
}

void TestSettings::getLoopInfo()
{
    Settings settings;
    QFETCH(QString, filename);
    QFETCH(bool, add_reverse);
    QFETCH(double, multiplier);

    settings.readSettings(filename);

    QCOMPARE(settings.getLoopAddReversedFrames("gandalf"), add_reverse);
    QCOMPARE(settings.getLoopTempoMultiplier("gandalf"), multiplier);
}

void TestSettings::setLoopSettings_data()
{
    QTest::addColumn<QString>("loop_name");
    QTest::addColumn<bool>("add_reverse");
    QTest::addColumn<double>("multiplier");

    QTest::addRow("first insert") << "fun_video" << false << 0.5;
    //QTest::addRow("another insert") << "another_video" << true << 4.0;
    //QTest::addRow("update existing data") << "fun_video" << true << 2.0;
}

void TestSettings::setLoopSettings()
{
    Settings settings;
    QFETCH(QString, loop_name);
    QFETCH(bool, add_reverse);
    QFETCH(double, multiplier);

    settings.setVideoLoopName(loop_name);

    settings.setLoopAddReversedFrames(loop_name, add_reverse);
    settings.setLoopTempoMultiplier(loop_name, multiplier);
    QCOMPARE(settings.getLoopAddReversedFrames(loop_name), add_reverse);
    QCOMPARE(settings.getLoopTempoMultiplier(loop_name), multiplier);

    settings.setLoopAddReversedFrames("another_video", true);
    settings.setLoopTempoMultiplier("another_video", 2.0);
    QCOMPARE(settings.getLoopAddReversedFrames(loop_name), add_reverse);
    QCOMPARE(settings.getLoopTempoMultiplier(loop_name), multiplier);

    settings.setLoopAddReversedFrames(loop_name, true);
    settings.setLoopTempoMultiplier(loop_name, 1.0);
    QCOMPARE(settings.getLoopAddReversedFrames(loop_name), true);
    QCOMPARE(settings.getLoopTempoMultiplier(loop_name), 1.0);
}

void TestSettings::writeSettings()
{
    Settings settings;

    QStringList pulseIgnore;
    pulseIgnore.append("VLC");
    pulseIgnore.append("Firefox");

    QString videoLoop = "hyle";
    bool startAsFullscreen = false;
    double confidenceThreshold = 2.5;

    settings.setPulseAudioIgnoreList(pulseIgnore);
    settings.setVideoLoopName(videoLoop);
    settings.setStartAsFullscreen(startAsFullscreen);
    settings.setConfidenceThreshold(confidenceThreshold);

    QString fileName = "./test/data/write_settings.JSON";
    settings.writeSettings(fileName);

    QFile saved(fileName);
    QVERIFY2(saved.exists() == true, "save file does not exists");

    Settings read;
    read.readSettings("./test/data/write_settings.JSON");

    QVERIFY2(read.getPulseAudioIgnoreList() == pulseIgnore, "wrong pulseaudio ignore list");
    QVERIFY2(read.getVideoLoopName() == videoLoop, "wrong video loop name");
    QVERIFY2(read.getStartAsFullscreen() == startAsFullscreen, "wrong start as fullscreen");
    QVERIFY2(read.getConfidenceThreshold() == confidenceThreshold, "wrong confidence threshold");

}

void TestSettings::cleanupTestCase()
{
    QFile writeSettings_file("./test/data/write_settings.JSON");
    writeSettings_file.remove();
}


QTEST_MAIN(TestSettings)
#include "tst_settings.moc"