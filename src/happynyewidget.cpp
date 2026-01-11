#include "happynyewidget.h"

HappyNYEWidget::HappyNYEWidget(QWidget *parent) : QWidget(parent)
{
    QFont l("Roboto-Regular", 100);

    m_layout = new QVBoxLayout();
    m_happy = new QLabel("Happy");
    m_happy->setScaledContents(true);
    m_happy->setAlignment(Qt::AlignCenter);
    m_happy->setFont(l);
    m_nye = new QLabel("New Years!");
    m_nye->setScaledContents(true);
    m_nye->setAlignment(Qt::AlignCenter);
    m_nye->setFont(l);

    m_layout->addWidget(m_happy);
    m_layout->addWidget(m_nye);
    setLayout(m_layout);
}

HappyNYEWidget::~HappyNYEWidget()
{
}

void HappyNYEWidget::go()
{
    QTimer::singleShot(60000, this, &HappyNYEWidget::timeout);
}

void HappyNYEWidget::timeout()
{
    emit finished();
}
