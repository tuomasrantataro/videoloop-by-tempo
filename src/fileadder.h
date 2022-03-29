#ifndef FILEADDER_H
#define FILEADDER_H

#include <QtCore>
#include <QFileDialog>
#include <QInputDialog>


class FileAdder : public QWidget
{
    Q_OBJECT
public:
    FileAdder(QWidget *parent = nullptr);

public slots:
    void openDialog();

signals:
    void filenamesReady();
    void filesCopied(QString);

private:
    QStringList m_fileNames;

private slots:
    void askFolderName();
};

#endif