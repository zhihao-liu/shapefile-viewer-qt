#ifndef VIEWFORM_H
#define VIEWFORM_H

#include <QWidget>

namespace Ui
{
class ViewForm;
}

class ViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit ViewForm(QWidget* parent = nullptr);
    ~ViewForm();

    void paintEvent(QPaintEvent*);
    void wheelEvent(QWheelEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

private:
    std::unique_ptr<Ui::ViewForm> ui;
    bool _mouseDragging = false;
};

#endif // VIEWFORM_H
