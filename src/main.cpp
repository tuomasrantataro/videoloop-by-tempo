#include "gui.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication app(argc, argv);
    app.setApplicationName("Video Looper");
    QIcon icon("icon_256px.png");
    app.setWindowIcon(icon);

    MainWindow mainWindow;

    return app.exec();
}