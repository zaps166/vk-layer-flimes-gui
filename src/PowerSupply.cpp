/*
    MIT License

    Copyright (c) 2020 Błażej Szczygieł

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include "PowerSupply.hpp"

#include <QSocketNotifier>
#include <QDir>

PowerSupply::PowerSupply()
    : m_udev(udev_new())
{
    if (!m_udev)
        return;

    m_monitor = udev_monitor_new_from_netlink(m_udev, "udev");
    if (!m_monitor)
        return;

    if (udev_monitor_enable_receiving(m_monitor) != 0)
        return;

    const int fd = udev_monitor_get_fd(m_monitor);
    if (fd < 0)
        return;

    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this, &PowerSupply::socketActivated);

    m_ok = true;

    checkBattery();
}
PowerSupply::~PowerSupply()
{
    if (m_notifier)
        m_notifier->setEnabled(false);
    if (m_monitor)
        udev_monitor_unref(m_monitor);
    if (m_udev)
        udev_unref(m_udev);
}

QByteArray PowerSupply::readFile(const QString &path) const
{
    QFile f(path);
    if (f.open(QFile::ReadOnly))
        return f.readAll().trimmed().toLower();
    return QByteArray();
}

void PowerSupply::socketActivated()
{
    auto dev = udev_monitor_receive_device(m_monitor);
    if (!dev)
        return;

    const QByteArray subsystem = udev_device_get_subsystem(dev);
    if (subsystem == "power_supply")
        checkBattery();

    udev_device_unref(dev);
}

void PowerSupply::checkBattery()
{
    bool isBattery = true;

    const auto powerSources = QDir("/sys/class/power_supply").entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (powerSources.empty())
    {
        isBattery = false;
    }
    else for (auto &&powerSource : powerSources)
    {
        const auto type = readFile(powerSource.filePath() + "/type");
        const auto online = readFile(powerSource.filePath() + "/online");
        if (type != "battery" && !online.isEmpty() && online != "0")
        {
            isBattery = false;
            break;
        }
    }

    if (m_isBattery != isBattery)
    {
        m_isBattery = isBattery;
        emit powerSourceChanged();
    }
}
