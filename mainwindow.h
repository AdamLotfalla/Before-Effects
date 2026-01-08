#pragma once

#include <QMainWindow>
#include <QPushButton.h>
#include <QLabel.h>
#include <QBoxLayout>
#include <QWidget>
#include <QPalette>
#include <QSplitter>
#include <QTimer>
#include "viewPort.h"
#include "timeline.h"

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
    viewPort* ViewPort_;

    
    QToolButton* selectionTool_;
    QToolButton* nodeTool_;
    QToolButton* bezierPen_;
    
    bool enableSelectionTool_ = false;
    bool enableNodeTool_ = false;
    bool enableBezier_ = false;

    void selectionTool(bool checked);
    void nodeTool(bool checked);
    void bezierTool(bool checked);

private slots:
    void startTimer(bool playing);
};
