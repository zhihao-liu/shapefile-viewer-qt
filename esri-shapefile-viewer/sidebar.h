#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <QDockWidget>
#include <memory>

class QListWidget;
class QListWidgetItem;

namespace Ui { class Sidebar; }

class Sidebar : public QDockWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget* parent = nullptr);
    ~Sidebar();

    void updateList();
    QList<QListWidgetItem*> listSelection() const;
    QListWidgetItem const* listFirst() const;
    QListWidgetItem const* listLast() const;

private:
    std::unique_ptr<Ui::Sidebar> ui;

private slots:
    void doubleClickItem(QListWidgetItem*);
};

#endif // SIDEBAR_H
