#ifndef SCREENSAVERINHIBITOR_H
#define SCREENSAVERINHIBITOR_H

#include <QtCore>
#include <QtDBus>

#include "types.h"

class ScreenSaverInhibitor : public QObject
{
    Q_OBJECT
public:
    ScreenSaverInhibitor();
    ~ScreenSaverInhibitor();

public slots:
    void inhibit(bool set);

private:
    QDBusConnection m_bus = QDBusConnection::sessionBus();
    QDBusInterface *m_dbusInterface;

    QString m_dBusName = "";

    uint m_dbusCookie = 0;
};

#endif