#include <QApplication>
#include <QCommandLineParser>

#include "mainwindow.h"

bool validateDirectories();

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setApplicationVersion("1.1");
    QApplication app(argc, argv);
    app.setApplicationName("Video Looper");
    QIcon icon("icon_256px.png");
    app.setWindowIcon(icon);

    if(!validateDirectories()) {
        return 1;
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption showAllInputs({"a", "all-audio-inputs"}, "Show all audio inputs instead of only those ending with \"monitor\".");
    parser.addOption(showAllInputs);
    QCommandLineOption noTrackDataLogging({"n", "no-track-logging"}, "Disable saving calculated song data to database.");
    parser.addOption(noTrackDataLogging);

    parser.process(app);

    MainWindow mainWindow(&parser);

    return app.exec();
}

bool validateDirectories()
{
    bool ret = true;
    if (!QDir("./assets/frames").exists()) {
        qInfo("\nERROR: ./assets/frames folder not found.\n");
        ret = false;
    } else if (QDir("./assets/frames").isEmpty()) {
        qInfo("\nERROR: frames folder is empty.\n");
        ret = false;
    } else {
        // check that none of the folders is empty
        QDir directory("./assets/frames");
        QStringList videoLoops = directory.entryList(QDir::NoDotAndDotDot | QDir::Dirs);

        for (auto it = videoLoops.begin(); it != videoLoops.end(); it++) {
            QString path = "./assets/frames/" + *it + "/";
            QDir frameDir = QDir(path);
            QStringList frames = frameDir.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG" << "*.gif" << "*.GIF", QDir::Files);
            if (frames.isEmpty()) {
                ret = false;
                qInfo("\nERROR: Empty frame folder: %s", qPrintable(path));
                qInfo("Please add frames or remove the folder\n");
            } else {
                qDebug("Found %d files in folder %s", frames.size(), qPrintable(path));
            }
        }
    }

    if (!ret) {
        qInfo("Please run the program in a directory which contains the loop frames.\n"
            "The directory structure should be:\n\t"
            "./videoloop-by-tempo (the executable)\n\t"
            "./assets/frames/video1/frame1.jpg\n\t"
            "./assets/frames/video1/frame2.jpg\n\t"
            "./assets/frames/video1/frame....png\n\t"
            "./assets/frames/video2/frame1.png\n\t"
            "./assets/frames/video2/frame....png\n"
            
            "Current working directory is:\n\t%s\n",
            qPrintable(QDir(".").absolutePath())
        );
    }

    return ret;
}