#include "nye.h"

NYE::NYE(QWidget *parent) : QWidget(parent)
{
    m_stackedWidget = new QStackedWidget();
    m_countdownWidget = new CountdownWidget();
    m_nyeWidget = new HappyNYEWidget();

    connect(m_countdownWidget, &CountdownWidget::finished, this, &NYE::happy);
    connect(m_nyeWidget, &HappyNYEWidget::finished, this, &NYE::complete);

    m_stackedWidget->addWidget(m_countdownWidget);
    m_stackedWidget->addWidget(m_nyeWidget);
    m_stackedWidget->setCurrentIndex(WidgetIndex::Primary);
    setCentralWidget(m_stackedWidget);
}

NYE::~NYE()
{
}

void NYE::countdown()
{
    m_stackedWidget->setCurrentIndex(0);
    m_countdownWidget->go();
}

void NYE::happy()
{
    m_stackedWidget->setCurrentIndex(1);
    m_nyeWidget->go();
}

void NYE::complete()
{
    emit finished();
}
