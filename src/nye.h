#pragma once

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "countdownwidget.h"
#include "happynyewidget.h"

class NYE : public QWidget
{
    Q_OBJECT
public:
    NYE(QWidget *parent = nullptr);
    ~NYE();

signals:
    void finished();

public slots:
    void countdown();
    void complete();
    void happy();

private:
    QStackedWidget *m_stackedWidget;
    CountdownWidget *m_countdownWidget;
    HappyNYEWidget *m_nyeWidget;
};
