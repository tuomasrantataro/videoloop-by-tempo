#ifndef DBUSWATCHER_H
#define DBUSWATCHER_H

#include <QtCore>
#include <QtDBus>

class DBusWatcher : public QObject
{
    Q_OBJECT
public:
    DBusWatcher();
    ~DBusWatcher();

public slots:
    void propertiesChanged(QString interface, QMap<QString, QVariant> signalData, QStringList l);

signals:
    void trackChanged(QString oldTrackId, QString oldArtist, QString oldTitle, QString newTrackId);
    void invalidateData();

private:
    QDBusConnection m_bus = QDBusConnection::sessionBus();

    QString m_oldTrackId;
    QString m_oldArtist;
    QString m_oldTitle;

    qint64 m_lastTrackChange = QDateTime::currentMSecsSinceEpoch();
    qint64 m_spotifyLength = 0;
};

#endif