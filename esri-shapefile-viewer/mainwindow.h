#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "viewform.h"
#include "sidebar.h"
#include "mapwindow.h"
#include "shapemanager.h"
#include "map.h"

class QLabel;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow, public cl::DataManagement::ShapeViewObserver
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    virtual void updateDisplay() override;
    virtual void setLabel(QString const& msg) override;

private:
    std::unique_ptr<Ui::MainWindow> ui;

    std::unique_ptr<ViewForm> _viewForm;
    std::unique_ptr<Sidebar> _sidebar;
    std::unique_ptr<QLabel> _msgLabel;
    std::unique_ptr<MapWindow> _mapWindow;

    void createMap(cl::Map::MapStyle mapStyle);

private slots:
    void openDataset();
    void closeAll();
    void removeLayer();
    void layerUp();
    void layerDown();

    void createMapFullElements();
    void createMapNoGridLine();
};

#endif // MAINWINDOW_H
