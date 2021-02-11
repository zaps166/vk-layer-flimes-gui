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

#include <algorithm>

using namespace std;

X11GlobalHotkey::X11GlobalHotkey()
    : m_conn(QX11Info::connection())
{
    if (!m_conn)
        return;

    m_ok = true;
}
X11GlobalHotkey::~X11GlobalHotkey()
{
    unregisterKeySequences();
}

bool X11GlobalHotkey::registerKeySequence(const KeySequence &keySeq)
{
    if (!keySeq.key || !keySeq.mod)
        return false;

    if (auto it = findKeySeq(keySeq); it != m_keySequences.end())
        return false;

    auto errReply = XCB_CALL_VOID_CHECKED(
        xcb_grab_key,
        QX11Info::connection(),
        1,
        QX11Info::appRootWindow(),
        keySeq.mod,
        keySeq.key,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC
    );
    if (errReply)
        return false;

    m_keySequences.push_back(keySeq);
    return true;
}

bool X11GlobalHotkey::unregisterKeySequence(const KeySequence &keySeq)
{
    if (!keySeq.mod || !keySeq.key)
        return false;

    auto it = findKeySeq(keySeq);
    if (it == m_keySequences.end())
        return false;

    if (!unregisterKeySequenceInternal(*it))
        return false;

    m_keySequences.erase(it);
    return true;
}
bool X11GlobalHotkey::unregisterKeySequences()
{
    bool ok = false;

    for (auto &&keySeq : m_keySequences)
        ok |= unregisterKeySequenceInternal(keySeq);
    m_keySequences.clear();

    return ok;
}

X11GlobalHotkey::KeySequenceList::iterator X11GlobalHotkey::findKeySeq(const KeySequence &keySeq)
{
    return find_if(m_keySequences.begin(), m_keySequences.end(), [&](const KeySequence &it) {
        return (it.mod == keySeq.mod && it.key == keySeq.key);
    });
}

bool X11GlobalHotkey::unregisterKeySequenceInternal(const KeySequence &keySeq)
{
    auto errReply = XCB_CALL_VOID_CHECKED(
        xcb_ungrab_key,
        QX11Info::connection(),
        keySeq.key,
        QX11Info::appRootWindow(),
        keySeq.mod
    );
    if (errReply)
        return false;
    return true;
}

bool X11GlobalHotkey::nativeEventFilter(const QByteArray &eventType, void *message, long *)
{
    if (eventType != "xcb_generic_event_t")
        return false;

    auto gev = static_cast<xcb_generic_event_t *>(message);
    if ((gev->response_type & ~0x80) != XCB_KEY_PRESS)
        return false;

    auto kev = reinterpret_cast<xcb_key_press_event_t *>(message);

    KeySequence keySeq;
    keySeq.mod = kev->state & (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_1 | XCB_MOD_MASK_4);
    keySeq.key = kev->detail;

    auto it = findKeySeq(keySeq);
    if (it == m_keySequences.end())
        return false;

    emit activated(*it);
    return true;
}
