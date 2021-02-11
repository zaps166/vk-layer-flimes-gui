/*
    MIT License

    Copyright (c) 2020-2021 Błażej Szczygieł

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

#pragma once

#include <QFileSystemWatcher>
#include <QFile>
#include <QDir>

class ExternalControl : public QObject
{
    Q_OBJECT

public:
    struct AppDescr
    {
        QString file;
        QString name;
        qint64 pid = 0;
    };

public:
    ExternalControl();
    ~ExternalControl();

    inline bool isOk() const;

    void cleanup();

    inline const std::vector<AppDescr> &applications() const;

    bool setFps(const AppDescr &app, double fps);

    void refresh();

private:
    void dirContentsChanged(const QString &path);

signals:
    void applicationsAboutToChange();
    void applicationsChanged();

private:
    const QDir m_flimesDir;
    QFile m_flimesPlaceholderFile;

    QFileSystemWatcher m_watcher;

    bool m_ok = false;
    bool m_cleanupDone = false;

    std::vector<AppDescr> m_applications;
};

/* Inline implementation */

bool ExternalControl::isOk() const
{
    return m_ok;
}

const std::vector<ExternalControl::AppDescr> &ExternalControl::applications() const
{
    return m_applications;
}
