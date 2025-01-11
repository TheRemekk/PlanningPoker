#include "clickablelabel.h"
#include <QMouseEvent>
#include <QPen>

ClickableLabel::ClickableLabel(QWidget* parent)
    : QLabel(parent), selected(false) {}

void ClickableLabel::setSelected(bool isSelected) {
    selected = isSelected;
    update();
}

void ClickableLabel::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);

    if (selected) {
        QPainter painter(this);
        QPen pen(Qt::blue, 6);
        painter.setPen(pen);


        painter.drawRect(rect().adjusted(3, 3, -3, -3));
    }
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }

    QLabel::mousePressEvent(event);
}
