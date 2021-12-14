#include "gui.h"
#include <QApplication>
#include <QCommandLineParser>


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setApplicationVersion("1.0");
    QApplication app(argc, argv);
    app.setApplicationName("Video Looper");
    QIcon icon("icon_256px.png");
    app.setWindowIcon(icon);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption showAllInputs({"a", "all-audio-inputs"}, "Show all audio inputs instead of only those ending with \"monitor\".");
    parser.addOption(showAllInputs);

    parser.process(app);

    MainWindow mainWindow(&parser);

    if (mainWindow.checkInit()) {
        return 1;
        
    } else {
        return app.exec();
    }
}