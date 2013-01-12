#include <QDebug>

#include "HUD2RenderThread.h"

HUD2RenderThread::HUD2RenderThread(HUD2Painter &hudpainter, QObject *parent) :
    QThread(parent),
    hudpainter(hudpainter)
{
    abort = false;

    renderMutex.lock();
    image = new QImage(640, 480, QImage::Format_ARGB32_Premultiplied);
    render = new QPainter(image);
    render->setRenderHint(QPainter::Antialiasing);
    renderMutex.unlock();
}

HUD2RenderThread::~HUD2RenderThread(){
    syncMutex.lock();
    abort = true;
    condition.wakeOne();
    syncMutex.unlock();

    wait();
    qDebug() << "RenderThread: destroyed";
}

void HUD2RenderThread::run(void){
    qDebug() << "RenderThread: run";

    while (!abort){
        condition.wait(&syncMutex);
        renderMutex.lock();
        render->fillRect(image->rect(), Qt::gray);
        hudpainter.paint(render);
        emit  renderedImage(*image);
        renderMutex.unlock();
    }

    qDebug() << "RenderThread: quit";
    quit();
}

void HUD2RenderThread::paint(void){

    if (!isRunning())
        start(LowPriority);
    else{
        syncMutex.lock();
        condition.wakeOne();
        syncMutex.unlock();
    }
}

void HUD2RenderThread::updateGeometry(const QSize &size){
    renderMutex.lock();
    delete render;
    delete image;
    image = new QImage(size, QImage::Format_ARGB32_Premultiplied);
    render = new QPainter(image);
    render->setRenderHint(QPainter::Antialiasing);
    hudpainter.updateGeometry(size);
    renderMutex.unlock();
}
