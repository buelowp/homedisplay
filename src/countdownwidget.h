#pragma once

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

class CountdownWidget : public QWidget
{
    Q_OBJECT
public:
    CountdownWidget(QWidget *parent = nullptr);
    ~CountdownWidget();

    void go();

signals:
    void finished();

public slots:
    void timeout();

private:
    QHBoxLayout *m_layout;
    QTimer *m_timer;
    QLabel *m_countdown;
    int m_remains;
};
