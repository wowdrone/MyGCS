#include <QColor>

#include "HUD2Painter.h"


HUD2Painter::HUD2Painter(HUD2Data &huddata, QWidget *parent) :
    QWidget(parent),
    horizon(huddata, this),
    altimeter(huddata, this),
    huddata(huddata)
{
    defaultColor = QColor(70, 255, 70);
    warningColor = Qt::yellow;
    criticalColor = Qt::red;
    infoColor = QColor(20, 200, 20);
    fuelColor = criticalColor;
}

void HUD2Painter::paint(QPainter *painter)
{
    painter->save();
    // all painting will perform relative to center of screen
    painter->translate(painter->window().center());
    horizon.paint(painter);
    altimeter.paint(painter);
    painter->restore();

    emit paintComplete();
}

void HUD2Painter::updateGeometry(const QSize &size){
    horizon.updateGeometry(size);
    altimeter.updateGeometry(size);
}
