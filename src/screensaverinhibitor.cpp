#include "screensaverinhibitor.h"

ScreenSaverInhibitor::ScreenSaverInhibitor()
{
    if (!m_bus.isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
    }

    m_dbusInterface = new QDBusInterface("org.freedesktop.ScreenSaver",
                                "/ScreenSaver",
                                "org.freedesktop.ScreenSaver", m_bus);

}

ScreenSaverInhibitor::~ScreenSaverInhibitor()
{

}

void ScreenSaverInhibitor::inhibit(bool set)
{
    QDBusReply<uint> reply;
    if (set) {
        reply = m_dbusInterface->call("Inhibit", "videoloop-by-tempo", "fullscreen mode");
        if (!reply.isValid()) {
            QDBusError err = reply.error();
            qDebug() << "Inhibit screensaver error:" << err.message();
        }
        m_dbusCookie = reply.value();
    }
    else {
        m_dbusInterface->call("UnInhibit", m_dbusCookie);
        QDBusError err = m_dbusInterface->lastError();
        if (err.isValid()) {
            qDebug() << "UnInhibit screensaver error:" << err.message();
        }

    }
}
