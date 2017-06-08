#ifndef VIEWFORM_H
#define VIEWFORM_H

#include <QWidget>

namespace Ui { class ViewForm; }

class ViewForm : public QWidget
{
    Q_OBJECT

public:
    explicit ViewForm(QWidget* parent = nullptr);
    ~ViewForm();

private:
    virtual void paintEvent(QPaintEvent*) override;
    virtual void wheelEvent(QWheelEvent*) override;
    virtual void mouseDoubleClickEvent(QMouseEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void mouseReleaseEvent(QMouseEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;

    std::unique_ptr<Ui::ViewForm> ui;
    bool _mouseDragging = false;
};

#endif // VIEWFORM_H
