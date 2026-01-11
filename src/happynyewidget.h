#pragma once

#include <QtCore/QtCore>
#include <QtWidgets/QtWidgets>

class HappyNYEWidget : public QWidget
{
    Q_OBJECT
public:
    HappyNYEWidget(QWidget *parent = nullptr);
    ~HappyNYEWidget();

    void go();

public slots:
    void timeout();

signals:
    void finished();

private:
    QVBoxLayout *m_layout;
    QLabel *m_happy;
    QLabel *m_nye;
};
