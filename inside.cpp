// SPDX-FileCopyrightText: 2022 <copyright holder> <email>
// SPDX-License-Identifier: MIT

#include "inside.h"

Inside::Inside(int frequency, QObject *parent) : QObject(parent), m_frequency(frequency), m_isSHT(false), m_isTCS(false), m_available(true)
{
    if (available(4, 0x44)) {
        m_isSHT = instantiateSHT();
    }
}

Inside::~Inside()
{
    m_timer->stop();
}

bool Inside::instantiateSHT()
{
    qDebug() << __PRETTY_FUNCTION__ << ": Instantiating an SHT31D";
    m_sht31d = new SHT31D(4);
    m_timer = new QTimer(this);
    m_timer->setInterval(m_frequency);
    connect(m_timer, &QTimer::timeout, this, &Inside::timeout);
    m_available = true;
    return true;
}

bool Inside::instantiateTCS()
{
    return false;
}

void Inside::timeout()
{
    if (m_available && m_isSHT) {
        if (m_sht31d->isOpen()) {
            if (m_sht31d->values(m_temperature, m_humidity) == SHT31D::sht31dreturn::SHT31_OK) {
                emit (temperature(m_temperature));
                emit (humidity(m_humidity));
            }
        }
    }
}

bool Inside::available(uint8_t device, uint8_t address)
{
    int file;
    int res;
    std::string d = "/dev/i2c-";
    d += std::to_string(device);

    std::cout << __PRETTY_FUNCTION__ << ": Probing " << d << " to find device at address " << std::hex << static_cast<int>(address) << std::endl;
    if ((file = open(d.c_str(), O_RDWR)) < 0) {
        std::cerr << __PRETTY_FUNCTION__ << ": Unable to open " << d << ": " << strerror(errno) << std::endl;
        return false;
    }
    if (ioctl(file, I2C_SLAVE, address) < 0) {
        if (errno == EBUSY) {
            std::cerr << __PRETTY_FUNCTION__ << ": Device is responding busy" << std::endl;
            return false;
        }
        else {
            std::cerr << __PRETTY_FUNCTION__ << ": Could not set I2C address to " << address << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
    if ((res = i2c_smbus_write_quick(file, (uint8_t)I2C_SMBUS_WRITE)) < 0) {
        std::cerr << __PRETTY_FUNCTION__ << ": Unable to query device at address " << address << ": " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

