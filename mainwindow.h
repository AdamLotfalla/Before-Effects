#pragma once

#include <QMainWindow>
#include <QPushButton.h>
#include <QLabel.h>
#include <QObject>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int frameRate_ = 24;
    QTimer* timer_;

private slots:
    void startTimer(bool playing);
};
