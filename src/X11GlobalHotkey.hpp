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

#include "KeySequence.hpp"
#include "X11Helpers.hpp"

#include <QAbstractNativeEventFilter>
#include <QObject>

#include <deque>

class X11GlobalHotkey : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

    using KeySequenceList = std::deque<KeySequence>;

public:
    X11GlobalHotkey();
    ~X11GlobalHotkey();

    inline bool isOk() const;

    bool registerKeySequence(const KeySequence &keySeq);

    bool unregisterKeySequence(const KeySequence &keySeq);
    bool unregisterKeySequences();

private:
    KeySequenceList::iterator findKeySeq(const KeySequence &keySeq);

    bool unregisterKeySequenceInternal(const KeySequence &keySeq);

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, NativeEventFilterResult *result) override;

private:
    xcb_connection_t *const m_conn;

    bool m_ok = false;

    KeySequenceList m_keySequences;

signals:
    void activated(const KeySequence &keySeq);
};

/* Inline implementation */

bool X11GlobalHotkey::isOk() const
{
    return m_ok;
}
