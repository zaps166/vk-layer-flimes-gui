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

#include "X11ActiveWindow.hpp"

#include <QDebug>

X11ActiveWindow::X11ActiveWindow()
    : m_conn(QX11Info::connection())
{
    if (!m_conn)
        return;

    const uint32_t mask = XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_change_window_attributes(m_conn, QX11Info::appRootWindow(), XCB_CW_EVENT_MASK, &mask);

    const QByteArray netActiveWindowAtom = "_NET_ACTIVE_WINDOW";
    if (auto activeWindowReply = XCB_CALL(xcb_intern_atom, m_conn, true, netActiveWindowAtom.length(), netActiveWindowAtom.constData()))
        _NET_ACTIVE_WINDOW = activeWindowReply->atom;

    const QByteArray newWmPidAtom = "_NET_WM_PID";
    if (auto pidReply = XCB_CALL(xcb_intern_atom, m_conn, true, newWmPidAtom.length(), newWmPidAtom.constData()))
        _NET_WM_PID = pidReply->atom;

    m_timer.setInterval(10);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout,
            this, &X11ActiveWindow::activeWindowChanged);

    m_ok = true;
}
X11ActiveWindow::~X11ActiveWindow()
{
}

void X11ActiveWindow::activeWindowChanged()
{
    auto emitActiveWindowPidChanged = [this](pid_t pid) {
        if (m_activeWindowPid != pid)
        {
            m_activeWindowPid = pid;
            emit activeWindowPidChanged(m_activeWindowPid);
        }
    };

    auto activeWindowReply = XCB_CALL(xcb_get_property, m_conn, false, m_pevWindow, _NET_ACTIVE_WINDOW, XCB_GET_PROPERTY_TYPE_ANY, 0, ~0);
    if (!activeWindowReply || activeWindowReply->type == 0)
    {
        emitActiveWindowPidChanged(0);
        return;
    }

    xcb_window_t activeWindow = reinterpret_cast<uint32_t *>(xcb_get_property_value(activeWindowReply.get()))[0];
    if (activeWindow == 0)
    {
        emitActiveWindowPidChanged(0);
        return;
    }

    auto pidReply = XCB_CALL(xcb_get_property, m_conn, false, activeWindow, _NET_WM_PID, XCB_GET_PROPERTY_TYPE_ANY, 0, ~0);
    if (!pidReply || pidReply->type == 0)
    {
        emitActiveWindowPidChanged(0);
        return;
    }

    pid_t activeWindowPid = reinterpret_cast<uint32_t *>(xcb_get_property_value(pidReply.get()))[0];
    emitActiveWindowPidChanged(activeWindowPid);
}

bool X11ActiveWindow::nativeEventFilter(const QByteArray &eventType, void *message, NativeEventFilterResult *result)
{
    Q_UNUSED(result)

    if (eventType != "xcb_generic_event_t")
        return false;

    auto gev = static_cast<xcb_generic_event_t *>(message);
    if ((gev->response_type & ~0x80) != XCB_PROPERTY_NOTIFY)
        return false;

    auto pev = static_cast<xcb_property_notify_event_t *>(message);
    if (pev->atom != _NET_ACTIVE_WINDOW)
        return false;

    m_pevWindow = pev->window;
    if (m_pevWindow != 0)
        m_timer.start();
    else
        m_timer.stop();

    return false;
}
