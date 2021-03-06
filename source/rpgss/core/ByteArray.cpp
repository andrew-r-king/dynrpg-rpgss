/*
    The MIT License (MIT)

    Copyright (c) 2014 Anatoli Steinmark

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include <cmath>
#include <cstring>

#include "ByteArray.hpp"


namespace rpgss {
    namespace core {

        //-----------------------------------------------------------------
        ByteArray::Ptr
        ByteArray::New(int size)
        {
            ByteArray::Ptr o = new ByteArray();
            if (size > 0) {
                o->resize(size);
            }
            return o;
        }

        //-----------------------------------------------------------------
        ByteArray::Ptr
        ByteArray::New(int size, u8 value)
        {
            ByteArray::Ptr o = new ByteArray();
            if (size > 0) {
                o->resize(size, value);
            }
            return o;
        }

        //-----------------------------------------------------------------
        ByteArray::Ptr
        ByteArray::New(const u8* buffer, int bufferSize)
        {
            ByteArray::Ptr o = new ByteArray();
            if (bufferSize > 0) {
                o->reset(buffer, bufferSize);
            }
            return o;
        }

        //-----------------------------------------------------------------
        ByteArray::Ptr
        ByteArray::New(const ByteArray* that)
        {
            ByteArray::Ptr o = new ByteArray();
            if (that->_size > 0) {
                o->reset(that->_buffer, that->_size);
            }
            return o;
        }

        //-----------------------------------------------------------------
        ByteArray::ByteArray()
            : _buffer(0)
            , _size(0)
        {
        }

        //-----------------------------------------------------------------
        ByteArray::~ByteArray()
        {
            clear();
        }

        //-----------------------------------------------------------------
        std::string
        ByteArray::toString() const
        {
            if (_size > 0) {
                return std::string((const char*)_buffer, _size);
            } else {
                return std::string();
            }
        }

        //-----------------------------------------------------------------
        void
        ByteArray::resize(int size)
        {
            if (size <= 0) {
                clear();
                return;
            }
            if (size != _size) {
                u8* new_buffer = new u8[size];
                if (_size > 0) {
                    int can_copy = std::min(_size, size);
                    std::memcpy(new_buffer, _buffer, can_copy);
                }
                delete[] _buffer;
                _buffer = new_buffer;
                _size = size;
            }
        }

        //-----------------------------------------------------------------
        void
        ByteArray::resize(int size, u8 fillValue)
        {
            int old_size = _size;
            resize(size);
            int new_size = _size;
            if (old_size < new_size) {
                std::memset(_buffer + old_size, fillValue, new_size - old_size);
            }
        }

        //-----------------------------------------------------------------
        void
        ByteArray::reset(const u8* buffer, int bufferSize)
        {
            if (bufferSize <= 0) {
                clear();
                return;
            }
            if (_size == bufferSize) {
                std::memcpy(_buffer, buffer, bufferSize);
                return;
            }
            clear();
            _buffer = new u8[bufferSize];
            _size = bufferSize;
            std::memcpy(_buffer, buffer, bufferSize);
        }

        //-----------------------------------------------------------------
        ByteArray::Ptr
        ByteArray::concat(const ByteArray* that) const
        {
            ByteArray::Ptr temp = New(_size + that->_size);
            if (_size > 0) {
                std::memcpy(temp->_buffer, _buffer, _size);
            }
            if (that->_size > 0) {
                std::memcpy(temp->_buffer + _size, that->_buffer, that->_size);
            }
            return temp;
        }

        //-----------------------------------------------------------------
        void
        ByteArray::append(const ByteArray* that)
        {
            if (that->_size > 0) {
                ByteArray::Ptr temp = New(_size + that->_size);
                if (_size > 0) {
                    std::memcpy(temp->_buffer, _buffer, _size);
                }
                std::memcpy(temp->_buffer + _size, that->_buffer, that->_size);
                swap(temp);
            }
        }

        //-----------------------------------------------------------------
        void
        ByteArray::clear()
        {
            if (_buffer) {
                delete[] _buffer;
                _buffer = 0;
                _size   = 0;
            }
        }

        //-----------------------------------------------------------------
        void
        ByteArray::memset(u8 value)
        {
            if (_size > 0) {
                std::memset(_buffer, value, _size);
            }
        }

        //-----------------------------------------------------------------
        void
        ByteArray::swap(ByteArray* that)
        {
            std::swap(_buffer, that->_buffer);
            std::swap(_size, that->_size);
        }

    } // namespace core
} // namespace rpgss
