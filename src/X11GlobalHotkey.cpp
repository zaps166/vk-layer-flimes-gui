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

#include "X11GlobalHotkey.hpp"
#include "X11Helpers.hpp"

#include <QDebug>

constexpr auto g_XK_F1 = 0xffbe;

X11GlobalHotkey::X11GlobalHotkey()
    : m_conn(QX11Info::connection())
{
    if (!m_conn)
        return;

    m_keySyms = xcb_key_symbols_alloc(m_conn);

    m_ok = true;
}
X11GlobalHotkey::~X11GlobalHotkey()
{
    unregisterKeySequences();
    xcb_key_symbols_free(m_keySyms);
}

bool X11GlobalHotkey::registerKeySequence(const QtKeySequence &qKeySeq)
{
    X11KeySequence x11KeySeq;

    if (m_qKeySequences.count(qKeySeq) > 0)
        return false;

    xcb_keysym_t keySymbol = 0;
    if ((qKeySeq.key() >= Qt::Key_A && qKeySeq.key() <= Qt::Key_Z) || (qKeySeq.key() >= Qt::Key_0 && qKeySeq.key() <= Qt::Key_9))
        keySymbol = qKeySeq.key();
    else if (qKeySeq.key() >= Qt::Key_F1 && qKeySeq.key() <= Qt::Key_F35)
        keySymbol = g_XK_F1 + (qKeySeq.key() - Qt::Key_F1);
    else
        return false;

    if (qKeySeq.mod() & Qt::ShiftModifier)
        x11KeySeq.mod() |= XCB_MOD_MASK_SHIFT;
    if (qKeySeq.mod() & Qt::ControlModifier)
        x11KeySeq.mod() |= XCB_MOD_MASK_CONTROL;
    if (qKeySeq.mod() & Qt::AltModifier)
        x11KeySeq.mod() |= XCB_MOD_MASK_1;
    if (qKeySeq.mod() & Qt::MetaModifier)
        x11KeySeq.mod() |= XCB_MOD_MASK_4;

    auto keycodes = xcb_key_symbols_get_keycode(m_keySyms, keySymbol);
    if (!keycodes)
        return false;

    x11KeySeq.key() = keycodes[0];
    free(keycodes);

    if (x11KeySeq.key() == XCB_NO_SYMBOL)
        return false;

    auto errReply = XCB_CALL_VOID_CHECKED(
        xcb_grab_key,
        QX11Info::connection(),
        1,
        QX11Info::appRootWindow(),
        x11KeySeq.mod(),
        x11KeySeq.key(),
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC
    );
    if (errReply)
        return false;

    m_qKeySequences[qKeySeq] = x11KeySeq;
    m_xKeySequences[x11KeySeq] = qKeySeq;
    return true;
}

bool X11GlobalHotkey::unregisterKeySequence(const QtKeySequence &qKeySeq)
{
    auto it = m_qKeySequences.find(qKeySeq);
    if (it == m_qKeySequences.end())
        return false;

    auto errReply = XCB_CALL_VOID_CHECKED(
        xcb_ungrab_key,
        QX11Info::connection(),
        it->first.key(),
        QX11Info::appRootWindow(),
        it->first.mod()
    );
    if (errReply)
        return false;

    m_xKeySequences.erase(it->second);
    m_qKeySequences.erase(it);
    return true;
}
bool X11GlobalHotkey::unregisterKeySequences()
{
    bool ok = false;

    auto qHotkeys = m_qKeySequences;
    for (auto &&qKeySeq : qHotkeys)
        ok |= unregisterKeySequence(qKeySeq.first);

    return ok;
}

bool X11GlobalHotkey::nativeEventFilter(const QByteArray &eventType, void *message, long *)
{
    if (eventType != "xcb_generic_event_t")
        return false;

    auto gev = static_cast<xcb_generic_event_t *>(message);
    if ((gev->response_type & ~0x80) != XCB_KEY_PRESS)
        return false;

    auto kev = reinterpret_cast<xcb_key_press_event_t *>(message);

    X11KeySequence x11KeySeq;
    x11KeySeq.mod() = kev->state & (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_1 | XCB_MOD_MASK_4);
    x11KeySeq.key() = kev->detail;

    auto it = m_xKeySequences.find(x11KeySeq);
    if (it == m_xKeySequences.end())
        return false;

    emit activated(it->second);
    return true;
}
