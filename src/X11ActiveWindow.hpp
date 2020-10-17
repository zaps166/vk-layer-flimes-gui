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

#pragma once

#include <QAbstractNativeEventFilter>
#include <QX11Info>
#include <QObject>
#include <QTimer>

class X11ActiveWindow : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    X11ActiveWindow();
    ~X11ActiveWindow();

    inline bool isOk() const;

private:
    void activeWindowChanged();

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;

signals:
    void activeWindowPidChanged(pid_t pid);

private:
    xcb_connection_t *const m_conn;

    xcb_atom_t _NET_ACTIVE_WINDOW = 0;
    xcb_atom_t _NET_WM_PID = 0;

    bool m_ok = false;

    xcb_window_t m_pevWindow = 0;
    QTimer m_timer;

    pid_t m_activeWindowPid = 0;
};

/* Inline implementation */

bool X11ActiveWindow::isOk() const
{
    return m_ok;
}
