#include "nye.h"

NYEWidget::NYEWidget(QWidget *parent) : QStackedWidget(parent)
{
    m_countdownWidget = new CountdownWidget();
    m_nyeWidget = new HappyNYEWidget();

    connect(m_countdownWidget, &CountdownWidget::finished, this, &NYEWidget::happy);
    connect(m_nyeWidget, &HappyNYEWidget::finished, this, &NYEWidget::complete);

    addWidget(m_countdownWidget);
    addWidget(m_nyeWidget);
    setCurrentIndex(0);
}

NYEWidget::~NYEWidget()
{
}

void NYEWidget::countdown()
{
    setCurrentIndex(0);
    m_countdownWidget->go();
}

void NYEWidget::happy()
{
    setCurrentIndex(1);
    m_nyeWidget->go();
}

void NYEWidget::complete()
{
    emit finished();
}
