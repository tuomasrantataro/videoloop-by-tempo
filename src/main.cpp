#include <QApplication>
#include <QCommandLineParser>

#include "gui.h"


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setApplicationVersion("1.1");
    QApplication app(argc, argv);
    app.setApplicationName("Video Looper");
    QIcon icon("icon_256px.png");
    app.setWindowIcon(icon);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption showAllInputs({"a", "all-audio-inputs"}, "Show all audio inputs instead of only those ending with \"monitor\".");
    parser.addOption(showAllInputs);
    QCommandLineOption noTrackDataLogging({"n", "no-track-logging"}, "Disable saving calculated song data to database.");
    parser.addOption(noTrackDataLogging);

    parser.process(app);

    MainWindow mainWindow(&parser);

    if (mainWindow.checkInit()) {
        return 1;
        
    } else {
        return app.exec();
    }
}