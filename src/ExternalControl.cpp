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

#include "ExternalControl.hpp"

#include <QDebug>

#include <filesystem>

#include <unistd.h>
#include <fcntl.h>

using namespace std;

ExternalControl::ExternalControl()
    : m_flimesDir(QString::fromStdString(filesystem::temp_directory_path().concat("/vk-layer-flimes")))
    , m_flimesPlaceholderFile(m_flimesDir.path() + "/PLACEHOLDER")
{
    m_flimesDir.mkpath(".");
    m_flimesPlaceholderFile.open(QFile::WriteOnly);

    if (!m_watcher.addPath(m_flimesDir.path()))
        return;

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &ExternalControl::dirContentsChanged);

    refresh();

    m_ok = true;
}
ExternalControl::~ExternalControl()
{
    m_flimesPlaceholderFile.remove();
    m_flimesDir.rmdir("../" + m_flimesDir.dirName());
}

bool ExternalControl::setFps(const AppDescr &appDescr, double fps)
{
    const int fd = open(appDescr.file.toLocal8Bit().constData(), O_WRONLY | O_NONBLOCK);
    if (fd < 0)
        return false;

    const auto str = QByteArray::number(fps, 'f', 3) + "\n";
    const bool ok = (write(fd, str.constData(), str.size()) == str.size());
    close(fd);

    return ok;
}

void ExternalControl::refresh()
{
    dirContentsChanged(m_flimesDir.path());
}

void ExternalControl::dirContentsChanged(const QString &path)
{
    Q_ASSERT(path == m_flimesDir.path());

    emit applicationsAboutToChange();

    m_applications.clear();

    const auto procList = QDir("/proc").entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    const auto files = m_flimesDir.entryInfoList(QDir::System, QDir::Time);
    for (auto &&file : files)
    {
        const auto filePath = file.filePath();
        const auto filename = file.fileName();

        if (!filesystem::is_fifo(filePath.toStdString()))
            continue;

        const int dashIdx = filename.lastIndexOf("-");
        if (dashIdx < 1)
            continue;

        const auto pidStr = filename.mid(dashIdx + 1);

        bool ok = false;

        AppDescr appDescr;
        appDescr.file = filePath;
        appDescr.name = filename.left(dashIdx);
        appDescr.pid = pidStr.toLongLong(&ok);

        if (!ok)
            continue;

        if (!procList.isEmpty() && !procList.contains(pidStr))
        {
            qDebug() << "Deleting:" << appDescr.file;
            QFile::remove(appDescr.file);
            continue;
        }

        m_applications.push_back(move(appDescr));
    }

    emit applicationsChanged();
}
