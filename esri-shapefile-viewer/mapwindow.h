#ifndef MAPWINDOW_H
#define MAPWINDOW_H

#include <QWidget>
#include "map.h"

namespace Ui { class MapWindow; }

class MapWindow : public QWidget, public cl::Map::MapObserver
{
    Q_OBJECT

public:
    explicit MapWindow(QWidget* parent = nullptr);
    ~MapWindow();

public:
    virtual void paintEvent(QPaintEvent*) override;

    virtual void updateDisplay() override;

    void setMap(std::shared_ptr<cl::Map::Map> const& map);

private:
    std::unique_ptr<Ui::MapWindow> ui;

    std::shared_ptr<cl::Map::Map> _map;
};

#endif // MAPWINDOW_H
