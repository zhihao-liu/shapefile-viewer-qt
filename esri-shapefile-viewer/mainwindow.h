#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "viewform.h"
#include "sidebar.h"
#include "shapemanager.h"

class QLabel;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow, public cl::DataManagement::ShapeDocsObserver
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    virtual void updateDisplay() override;
    virtual QRect const paintingRect() const override;
    virtual void setLabel(QString const& msg) override;

private:
    std::unique_ptr<Ui::MainWindow> ui;

    std::unique_ptr<ViewForm> _viewForm;
    std::unique_ptr<Sidebar> _sidebar;
    std::unique_ptr<QLabel> _msgLabel;

private slots:
    void openDataset();
    void closeAll();
    void removeLayer();
    void layerUp();
    void layerDown();
};

#endif // MAINWINDOW_H
