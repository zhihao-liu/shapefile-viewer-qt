#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include "viewform.h"
#include "sidebar.h"

class QLabel;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateWidgets();
    void setStatusLabel(QString msg);
    QRect const viewRect() const;

private:
    std::unique_ptr<Ui::MainWindow> ui;

    std::unique_ptr<ViewForm> _viewForm;
    std::unique_ptr<Sidebar> _sidebar;
    std::unique_ptr<QLabel> _msgLabel;

private slots:
    void openFile();
};

#endif // MAINWINDOW_H
