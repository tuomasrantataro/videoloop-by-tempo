#include "fileadder.h"

FileAdder::FileAdder(QWidget *parent) : QWidget(parent)
{
    connect(this, &FileAdder::filenamesReady, this, &FileAdder::askFolderName);
}

void FileAdder::openDialog()
{
    m_fileNames = QFileDialog::getOpenFileNames(this, tr("Open Images"), QDir::homePath(), tr("Image Files (*.png *.jpg)"), 0, QFileDialog::DontUseNativeDialog);

    if (!m_fileNames.isEmpty()) {
        emit filenamesReady();
    }
}

void FileAdder::askFolderName()
{
    bool ok;

    QString text = QInputDialog::getText(this, tr("Loop name"),
                                         tr("Loop name:"), QLineEdit::Normal,
                                         QDir::home().dirName(), &ok);

    if (ok && !text.isEmpty()) {
        QDir folder = QDir::currentPath() + "/assets/frames/";

        bool okDir = folder.mkdir(text);

        QString folderName = QDir::currentPath() + "/assets/frames/" + text;

        if (!okDir | !QDir(folderName).isEmpty()) {
            qWarning() << "Folder already exists and is not empty. Won't copy the files to assets/frames.";
            return;
        }
        
        for (auto item : m_fileNames) {
            QStringList pathParts = item.split('/');
            QFile::copy(item, folderName + '/' + pathParts.last());
        }

        emit filesCopied(text);
    }

}