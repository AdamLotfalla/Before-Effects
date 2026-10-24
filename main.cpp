#include <QtSvg/QtSvg>
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window,          QColor("#2E2E2E"));
    darkPalette.setColor(QPalette::WindowText,      QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::Base,            QColor("#1E1E1E"));
    darkPalette.setColor(QPalette::AlternateBase,   QColor("#2E2E2E"));
    darkPalette.setColor(QPalette::ToolTipBase,     QColor("#2E2E2E"));
    darkPalette.setColor(QPalette::ToolTipText,     QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::Text,            QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::Button,          QColor("#2E2E2E"));
    darkPalette.setColor(QPalette::ButtonText,      QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::BrightText,      QColor("#FF5555"));
    darkPalette.setColor(QPalette::Highlight,       QColor("#2A7FFF"));
    darkPalette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    darkPalette.setColor(QPalette::Disabled, QPalette::Text,       QColor("#666666"));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#666666"));

    a.setPalette(darkPalette);
    
    MainWindow w;
    w.setWindowTitle("Before Effects");
    w.showMaximized();
    w.setWindowIcon(QIcon(":/Program/icons/Program_icon.svg"));

    return a.exec();
}

