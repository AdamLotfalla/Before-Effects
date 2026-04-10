#pragma once

#include <QMainWindow>
#include <QPushButton.h>
#include <QLabel.h>
#include <QBoxLayout>
#include <QWidget>
#include <QPalette>
#include <QSplitter>
#include <QTimer>
#include <QApplication>
#include "viewPort.h"
#include "timeline.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class AttributePanelWidget : public QWidget {
    Q_OBJECT
    QVBoxLayout* layout_;
    QWidget* currentWidget_ = nullptr;

public:
    AttributePanelWidget(QWidget* parent = nullptr) : QWidget(parent) {
        layout_ = new QVBoxLayout(this);
        layout_->setAlignment(Qt::AlignTop);
    }

public slots:
    void showObject(AttributePanel* obj) {
        if (currentWidget_) {
            layout_->removeWidget(currentWidget_);
            delete currentWidget_;
            currentWidget_ = nullptr;
        }
        if (obj) {
            currentWidget_ = obj->createAttributeWidget(this);
            layout_->addWidget(currentWidget_);
        }
    }
};

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
