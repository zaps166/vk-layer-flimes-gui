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

#include "KeySequence.hpp"

#include <QAbstractNativeEventFilter>
#include <QX11Info>
#include <QObject>

#include <map>

#include <xcb/xcb_keysyms.h>

class X11GlobalHotkey : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

    using X11KeySequence = KeySequence<uint16_t, xcb_keycode_t>;

public:
    X11GlobalHotkey();
    ~X11GlobalHotkey();

    inline bool isOk() const;

    bool registerKeySequence(const QtKeySequence &qKeySeq);

    bool unregisterKeySequence(const QtKeySequence &qKeySeq);
    bool unregisterKeySequences();

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override;

private:
    xcb_connection_t *const m_conn;

    xcb_key_symbols_t *m_keySyms = nullptr;

    bool m_ok = false;

    std::map<QtKeySequence, X11KeySequence> m_qKeySequences;
    std::map<X11KeySequence, QtKeySequence> m_xKeySequences;

signals:
    void activated(const QtKeySequence &qKeySeq);
};

/* Inline implementation */

bool X11GlobalHotkey::isOk() const
{
    return m_ok;
}
