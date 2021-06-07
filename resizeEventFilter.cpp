#include "resizeEventFilter.h"

bool ResizeEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        qDebug() << __PRETTY_FUNCTION__ << obj->objectName() << "resized";
        qDebug() << __PRETTY_FUNCTION__ << ": Old size" << resizeEvent->oldSize();
        qDebug() << __PRETTY_FUNCTION__ << ": new size" << resizeEvent->size();
        return true;
    }
    else {
        return QObject::eventFilter(obj, event);
    }
    return true;
}
