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

#include <qnamespace.h>

template<typename T1, typename T2>
struct KeySequence : public std::pair<T1, T2>
{
    KeySequence() = default;
    KeySequence(T1 mod, T2 key)
        : std::pair<T1, T2>(mod, key)
    {}

    const T1 &mod() const
    {
        return std::pair<T1, T2>::first;
    }
    T1 &mod()
    {
        return std::pair<T1, T2>::first;
    }

    const T2 &key() const
    {
        return std::pair<T1, T2>::second;
    }
    T2 &key()
    {
        return std::pair<T1, T2>::second;
    }
};
using QtKeySequence = KeySequence<Qt::KeyboardModifiers, Qt::Key>;
