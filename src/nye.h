#pragma once

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

#include "countdownwidget.h"
#include "happynyewidget.h"

class NYEWidget : public QStackedWidget
{
    Q_OBJECT
public:
    NYEWidget(QWidget *parent = nullptr);
    ~NYEWidget();

signals:
    void finished();

public slots:
    void countdown();
    void complete();
    void happy();

private:
    CountdownWidget *m_countdownWidget;
    HappyNYEWidget *m_nyeWidget;
};
