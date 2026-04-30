#include <QtSvg/QtSvg>
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    MainWindow w;
    w.setWindowTitle("Before Effects");
    w.showMaximized();
    w.setWindowIcon(QIcon(":/Program/icons/Program_icon.svg"));

    return a.exec();
}

