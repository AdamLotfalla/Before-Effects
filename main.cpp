#include <QtSvg/QtSvg>
#include <QApplication>
#include "mainwindow.h"

Q_IMPORT_PLUGIN(QSvgPlugin)
Q_IMPORT_PLUGIN(QSvgIconPlugin)

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    MainWindow w;
    w.setWindowTitle("Before Effects");
    w.showMaximized();

    return a.exec();
}

