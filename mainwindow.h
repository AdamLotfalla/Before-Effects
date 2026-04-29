#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
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
    AttributePanel* currentObject_ = nullptr;

public:
    AttributePanelWidget(QWidget* parent = nullptr) : QWidget(parent) {
        layout_ = new QVBoxLayout(this);
        layout_->setAlignment(Qt::AlignTop);
    }

public slots:
    void showObject(AttributePanel* obj) {

        if(currentObject_ == obj) {
            return;
        }

        
        if (currentWidget_) {
            layout_->removeWidget(currentWidget_);
            currentWidget_->hide();
            currentWidget_ = nullptr;
        }
        
        currentObject_ = obj;

        if (obj) {
            currentWidget_ = obj->createAttributeWidget(this);
            if (currentWidget_) {
                layout_->addWidget(currentWidget_);
                currentWidget_->show();
            }
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
    viewPort* viewPort_;

    
    QToolButton* selectionTool_;
    QToolButton* nodeTool_;
    QToolButton* bezierPen_;
    
    bool enableSelectionTool_ = false;
    bool enableNodeTool_ = false;
    bool enableBezier_ = false;

    void selectionTool(bool checked);
    void nodeTool(bool checked);
    void bezierTool(bool checked);

    QWidget* templateAttributeWidget_ = nullptr;
    path* tempPath = nullptr;
    
    void preCreateAttributeWidgets();

private slots:
    void startTimer(bool playing);
};
