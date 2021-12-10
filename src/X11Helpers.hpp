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

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#   include <QtGui/private/qtx11extras_p.h>
#else
#   include <QX11Info>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    using NativeEventFilterResult = qintptr;
#else
    using NativeEventFilterResult = long;
#endif

#include <cstdlib>
#include <memory>

template<typename Ptr>
static inline auto managePtr(Ptr *reply)
{
    return std::unique_ptr<Ptr, void(*)(void *)>(reply, free);
}

#define XCB_CALL(xcbFunc, conn, ...) \
    managePtr(xcbFunc##_reply(conn, xcbFunc(conn, ##__VA_ARGS__), nullptr))

#define XCB_CALL_VOID_CHECKED(xcbFunc, conn, ...) \
    managePtr(xcb_request_check(conn, xcbFunc##_checked(conn, ##__VA_ARGS__)))
