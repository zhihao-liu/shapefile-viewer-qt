#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTime>
#include <QLabel>
#include "shapemanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qsrand(QTime::currentTime().second());

    // initialize the view widget of shapes
    _viewForm.reset(new ViewForm(this));
    setCentralWidget(_viewForm.get());

    // initialize the sidebar widget
    _sidebar.reset(new Sidebar(this));
    addDockWidget(Qt::LeftDockWidgetArea, _sidebar.get(), Qt::Vertical);

    // initialize the status label
    _msgLabel.reset(new QLabel());
    statusBar()->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
    statusBar()->addWidget(_msgLabel.get());

    // initialize the static data object
    cl::ShapeManager::setData(*this);

    // connect the open file signal
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(openFile()));
    // if the slot function name is wrong,
    // without any error prompts the connection will not work

    // show the main window
    showMaximized();
}

MainWindow::~MainWindow() {}

void MainWindow::openFile()
{
    QFileDialog dialog(this, tr("Open ESRI Shape File:"), "", tr("*.shp"));
    if (!dialog.exec())
        return;

    QStringList fileNames = dialog.selectedFiles();
    QString qsPath = fileNames.at(0);
    std::string sPath = qsPath.toStdString();

    cl::ShapeManager::data().addShape(sPath);
}

void MainWindow::updateWidgets()
{
    _viewForm->update();
    _sidebar->updateListView();
    update();
}

QRect const MainWindow::viewRect() const
{
    return _viewForm->rect();
}

void MainWindow::setStatusLabel(QString msg)
{
    if (_msgLabel != nullptr)
        _msgLabel->setText(msg);
}
