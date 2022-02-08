
#include <QtCore>
#include <QTest>
#include <QSignalSpy>

#include "tempo.h"

//Q_DECLARE_METATYPE(Tempo)

class TestTempo : public QObject
{
    Q_OBJECT
private slots:
    void setTempoLimits_data();
    void setTempoLimits();
    void setEnableTempoLimits();
    void setTempoManual_data();
    void setTempoManual();
    void setTempoSmooth_data();
    void setTempoSmooth();
    void setTempoAutomatic_data();
    void setTempoAutomatic();
};

void TestTempo::setTempoLimits_data()
{
    QTest::addColumn<double>("lower_limit");
    QTest::addColumn<double>("upper_limit");
    QTest::addColumn<double>("correct_lower");
    QTest::addColumn<double>("correct_upper");

    QTest::addRow("valid limits") << 60.0 << 120.0 << 60.0 << 120.0;
    QTest::addRow("upper too low") << 60.0 << 110.0 << 60.0 << 120.0;
    QTest::addRow("upper lower than lower") << 110.0 << 60.0 << 110.0 << 220.0;
    QTest::addRow("zero limits") << 0.0 << 0.0 << 1.0 << 300.0;
    QTest::addRow("negative numbers") << -3.0 << -8.0 << 1.0 << 300.0;
}

void TestTempo::setTempoLimits()
{
    Tempo tempo(this);

    QFETCH(double, lower_limit);
    QFETCH(double, upper_limit);
    QFETCH(double, correct_lower);
    QFETCH(double, correct_upper);

    tempo.setTempoLowerLimit(lower_limit);
    tempo.setTempoUpperLimit(upper_limit);

    QCOMPARE(tempo.getTempoLowerLimit(), correct_lower);
    QCOMPARE(tempo.getTempoUpperLimit(), correct_upper);
}

void TestTempo::setEnableTempoLimits()
{
    Tempo tempo(this);

    double tempoVal = 65.0;

    tempo.setEnableTempoLimits(false);
    tempo.setTempoManual(tempoVal);

    QVERIFY2(tempo.getTempo() == tempoVal, "Tempo not correct when no tempo limits");
    
    tempo.setTempoLowerLimit(70.0);
    tempo.setTempoUpperLimit(150.0);

    QVERIFY2(tempo.getTempo() == tempoVal, "Tempo not correct when tempo outside limits but limits disabled");

    tempo.setEnableTempoLimits(true);

    QVERIFY2(tempo.getTempo() == 2*tempoVal, "Tempo not correct when tempo limits enabled");

    tempo.setEnableTempoLimits(false);

    QVERIFY2(tempo.getTempo() == tempoVal, "Tempo not correct after first enabling and the disabling tempo limits");

}


void TestTempo::setTempoManual_data()
{
    QTest::addColumn<double>("lower_limit");
    QTest::addColumn<double>("upper_limit");
    QTest::addColumn<double>("requested_tempo");
    QTest::addColumn<double>("correct_tempo");

    QTest::addRow("tempo between limits") << 70.0 << 145.0 << 110.0 << 110.0;
    QTest::addRow("tempo below limit") << 70.0 << 145.0 << 60.0 << 120.0;
    QTest::addRow("tempo above limit") << 70.0 << 145.0 << 160.0 << 80.0;
}

void TestTempo::setTempoManual()
{
    Tempo tempo(this);

    QFETCH(double, lower_limit);
    QFETCH(double, upper_limit);
    QFETCH(double, requested_tempo);
    QFETCH(double, correct_tempo);

    tempo.setEnableTempoLimits(true);
    tempo.setTempoLowerLimit(lower_limit);
    tempo.setTempoUpperLimit(upper_limit);
    
    QSignalSpy spy(&tempo, &Tempo::tempoChanged);
    QVERIFY(spy.isValid());

    tempo.setTempoManual(requested_tempo);

    QList<QVariant> arguments = spy.takeFirst();
    
    QCOMPARE(arguments.at(0).toDouble(), correct_tempo);

}

void TestTempo::setTempoSmooth_data()
{
    QTest::addColumn<double>("lower_limit");
    QTest::addColumn<double>("upper_limit");
    QTest::addColumn<double>("starting_tempo");
    QTest::addColumn<double>("requested_tempo");
    QTest::addColumn<double>("final_tempo");

    QTest::addRow("tempo between limits") << 60.0 << 150.0 << 110.0 << 140.0 << 140.0;
    QTest::addRow("tempo outside limits") << 60.0 << 150.0 << 110.0 << 160.0 << 80.0;
    QTest::addRow("rising tempo") << 30.0 << 300.0 << 160.0 << 70.0 << 70.0;
    QTest::addRow("falling tempo") << 30.0 << 300.0 << 70.0 << 160.0 << 160.0;
    QTest::addRow("new tempo double") << 30.0 << 300.0 << 70.0 << 140.0 << 140.0;
    QTest::addRow("new tempo half") << 30.0 << 300.0 << 140.0 << 70.0 << 70.0;

}

void TestTempo::setTempoSmooth()
{
    Tempo tempo(this);
    
    QFETCH(double, lower_limit);
    QFETCH(double, upper_limit);
    QFETCH(double, starting_tempo);
    QFETCH(double, requested_tempo);
    QFETCH(double, final_tempo);

    tempo.setEnableTempoLimits(true);
    tempo.setTempoLowerLimit(lower_limit);
    tempo.setTempoUpperLimit(upper_limit);

    tempo.setTempoManual(requested_tempo);
    double req = tempo.getTempo();  // getting what the request should be after applying limits
    
    tempo.setTempoManual(starting_tempo);

    QSignalSpy spy(&tempo, &Tempo::tempoChanged);
    QVERIFY(spy.isValid());

    tempo.setSmootheningTimerInterval(1);   // 1 msec

    tempo.setTempoSmooth(requested_tempo);

    QList<QVariant> arguments;

    double prev = tempo.getTempo();
    double starting = prev;
    
    for (int i = 0; i < 7; i++) {
        spy.wait();
        arguments = spy.takeFirst();
        double current = arguments.at(0).toDouble();
        if (req < starting) {
            QVERIFY(prev > current);
        }
        else {
            QVERIFY(prev < current);
        }
        prev = current;
        if(i == 6) {
            QCOMPARE(current, final_tempo);
        }
    }    
}

void TestTempo::setTempoAutomatic_data()
{
    QTest::addColumn<double>("lower_limit");
    QTest::addColumn<double>("upper_limit");
    QTest::addColumn<double>("starting_tempo");
    QTest::addColumn<std::list<double>>("input_tempos");
    QTest::addColumn<std::list<double>>("output_tempos");

    double starting_tempo = 130.0;

    std::list<double> inputs1 = { 96.5, 94.5, 97.4, 96.3, 96.4,
                                  93.5, 99.3, 98.4, 91.4, 94.2 };

    std::list<double> outputs1 = { 123.30, 116.20, 109.68, 102.94, 96.22,
                                   95.62, 96.58, 96.78, 95.80, 95.36 };

    std::list<double> outputs2 = { 246.60, 232.40, 219.36, 205.88, 192.44,
                                   191.24, 193.16, 193.56, 191.60, 190.72 };
    
    std::list<double> outputs3 = { 142.60, 154.40, 167.36, 179.88, 192.44,
                                   191.24, 193.16, 193.56, 191.60, 190.72 };
                                   

    QTest::addRow("between limits") << 30.0 << 180.0 << starting_tempo << inputs1 << outputs1;
    QTest::addRow("outside limits") << 140.0 << 280.0 << starting_tempo << inputs1 << outputs2;
    QTest::addRow("on 2 sides of limit") << 110.0 << 220.0 << starting_tempo << inputs1 << outputs3;

}

void TestTempo::setTempoAutomatic()
{
    QFETCH(double, lower_limit);
    QFETCH(double, upper_limit);
    QFETCH(double, starting_tempo);
    QFETCH(std::list<double>, input_tempos);
    QFETCH(std::list<double>, output_tempos);

    Tempo tempo(this);

    tempo.setEnableTempoLimits(true);
    tempo.setTempoLowerLimit(lower_limit);
    tempo.setTempoUpperLimit(upper_limit);
    tempo.setTempoManual(starting_tempo);

    QSignalSpy spy(&tempo, &Tempo::tempoChanged);
    QVERIFY(spy.isValid());

    QList<QVariant> arguments;
    
    for (auto item : input_tempos) {
        MyTypes::TempoData data;
        data.BPM = item;
        data.confidence = 3.0;
        tempo.setTempoAutomatic(data);
        arguments = spy.takeFirst();

        QVERIFY(abs(arguments.at(0).toDouble() - output_tempos.front()) < 0.01);
        output_tempos.pop_front();
    }    

}

QTEST_MAIN(TestTempo)
#include "tst_tempo.moc"