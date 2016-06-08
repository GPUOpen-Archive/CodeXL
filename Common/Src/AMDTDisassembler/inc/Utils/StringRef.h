//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StringRef.h
/// \brief A wrapper for a C string.
///
//==================================================================================

#ifndef _STRINGREF_H_
#define _STRINGREF_H_

#include "typedefs.h"
#include <cstring>

class DASM_API StringRef
{
protected:
    const char* m_data;
    size_t m_length;

public:
    static const size_t npos = ~size_t(0);

    StringRef() : m_data(NULL), m_length(0) {}
    StringRef(const char* str) : m_data(str), m_length(strlen(str)) {}
    StringRef(const char* buf, size_t len) : m_data(buf), m_length(len) {}
    StringRef(const StringRef& str) : m_data(str.m_data), m_length(str.m_length) {}

    const char* data() const { return m_data; }
    bool empty() const { return 0 == m_length; }
    size_t size() const { return m_length; }

    char operator[](size_t idx) const { return m_data[idx]; }

    StringRef& operator=(const char* str) { m_data = str; m_length = strlen(str); return *this; }
    StringRef& operator=(const StringRef& str) { m_data = str.m_data; m_length = str.m_length; return *this; }
};

class DASM_API MutableStringRef : public StringRef
{
private:
    size_t m_capacity;

    explicit MutableStringRef(const MutableStringRef&);

public:
    MutableStringRef() : m_capacity(0) {}
    MutableStringRef(char* data, size_t cap) : StringRef(data, 0), m_capacity(cap) {}

    void reset(char* data, size_t cap) { m_data = data; m_length = 0; m_capacity = cap; }

    char* data() const { return const_cast<char*>(m_data); }

    char& operator[](size_t idx) const { return data()[idx]; }

    MutableStringRef& append(const StringRef& str)
    {
        // append str
        return append(str.data(), str.size());
    }

    MutableStringRef& append(const StringRef& str, size_t count)
    {
        // append str [0, count)
        size_t num = str.size();

        if (num < count)
        {
            count = num;    // trim to size
        }

        if (0 < count && m_capacity >= (num = m_length + count))
        {
            memcpy(data() + m_length, str.data(), count);
            data()[m_length = num] = '\0';
        }

        return *this;
    }

    MutableStringRef& append(const char* buf, size_t count)
    {
        // append [buf, buf + count)
        size_t num;

        if (0 < count && m_capacity >= (num = m_length + count))
        {
            memcpy(data() + m_length, buf, count);
            data()[m_length = num] = '\0';
        }

        return *this;
    }

    MutableStringRef& append(const char* str)
    {
        // append [str, <null>)
        return append(str, strlen(str));
    }

    MutableStringRef& append(char c)
    {
        // append c
        if (m_capacity > m_length)
        {
            data()[m_length] = c;
            data()[++m_length] = '\0';
        }

        return *this;
    }

    MutableStringRef& operator+=(const StringRef& str) { return append(str); }
    MutableStringRef& operator+=(const char* str) { return append(str); }
    MutableStringRef& operator+=(char c) { return append(c); }


    MutableStringRef& assign(const StringRef& str)
    {
        // assign str
        return assign(str.data(), str.size());
    }

    MutableStringRef& assign(const StringRef& str, size_t count)
    {
        // assign str [0, count)
        size_t num = str.size();

        if (count < num)
        {
            num = count;    // trim to size
        }

        if (m_capacity < num)
        {
            num = m_capacity;
        }

        if (0 < num)
        {
            memcpy(data(), str.data(), num);
            data()[m_length = num] = '\0';
        }

        return *this;
    }

    MutableStringRef& assign(const char* buf, size_t count)
    {
        // assign [_Ptr, _Ptr + _Count)
        if (m_capacity < count)
        {
            count = m_capacity;
        }

        if (0 < count)
        {
            memcpy(data(), buf, count);
            data()[m_length = count] = '\0';
        }

        return *this;
    }

    MutableStringRef& assign(const char* str)
    {
        // assign [str, <null>)
        return assign(str, strlen(str));
    }

    MutableStringRef& assign(char c)
    {
        if (0 < m_capacity)
        {
            data()[0] = c;
            data()[m_length = 1] = '\0';
        }

        return *this;
    }

    MutableStringRef& operator=(const char* str) { return assign(str); }
    MutableStringRef& operator=(const StringRef& str) { return assign(str); }

    void clear() { data()[m_length = 0] = '\0'; }

    size_t capacity() const { return m_capacity; }



private:
    static char ConvertHex(size_t v)
    {
        static const char convert[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                          'a', 'b', 'c', 'd', 'e', 'f'
                                        };
        return convert[v];
    }

    void appendSign(AMD_INT64& val, bool prependPlus)
    {
        if (val < 0)
        {
            append('-');
            val = -val;
        }
        else if (prependPlus)
        {
            append('+');
        }
    }

    void appendSign(AMD_UINT64& val, bool prependPlus)
    {
        (void)(val); // unused

        if (prependPlus)
        {
            append('+');
        }
    }

public:
    static void FormatHex16(char* str, AMD_UINT64 val)
    {
        for (int i = 0; i < 16; ++i)
        {
            *str++ = ConvertHex((val >> (4 * (16 - 1 - i))) & 0xf);
        }

        *str = '\0';
    }

    static void FormatHex8(char* str, AMD_UINT32 val)
    {
        for (int i = 0; i < 8; ++i)
        {
            *str++ = ConvertHex((val >> (4 * (8 - 1 - i))) & 0xf);
        }

        *str = '\0';
    }

    static void FormatHex4(char* str, AMD_UINT16 val)
    {
        for (int i = 0; i < 4; ++i)
        {
            *str++ = ConvertHex((val >> (4 * (4 - 1 - i))) & 0xf);
        }

        *str = '\0';
    }

    static void FormatHex2(char* str, AMD_UINT8 val)
    {
        for (int i = 0; i < 2; ++i)
        {
            *str++ = ConvertHex((val >> (4 * (2 - 1 - i))) & 0xf);
        }

        *str = '\0';
    }

    static void FormatHex(char* str, AMD_UINT64 val, size_t width)
    {
        switch (width)
        {
            case 16:
                FormatHex16(str, val);
                return;

            case 8:
                FormatHex8(str, (AMD_UINT32)val);
                return;

            case 4:
                FormatHex4(str, (AMD_UINT16)val);
                return;

            case 2:
                FormatHex2(str, (AMD_UINT8)val);
                return;
        }

        size_t i = 0;

        for (size_t e = 16 - width; i < e; ++i)
            if (val & (0xf << (4 * (16 - 1 - i))))
            {
                break;
            }

        for (; i < 8; ++i)
        {
            *str++ = ConvertHex((val >> (4 * (16 - 1 - i))) & 0xf);
        }

        *str = '\0';
    }

    void appendUInt(AMD_UINT64 val, size_t width, const StringRef& hexPostfix, bool prependPlus = false)
    {
        appendSign(val, prependPlus);

        if (m_capacity >= (m_length + width))
        {
            FormatHex(data() + m_length, val, width);
            data()[m_length += width] = '\0';
            append(hexPostfix);
        }
    }

    void appendSInt(AMD_INT64 val, size_t width, const StringRef& hexPostfix, bool prependPlus = true)
    {
        appendSign(val, prependPlus);
        appendUInt((AMD_UINT64)val, width, hexPostfix, false);
    }

    void appendUInt(const StringRef& hexPrefix, AMD_UINT64 val, size_t width, bool prependPlus = false)
    {
        appendSign(val, prependPlus);

        if (m_capacity >= (m_length + width))
        {
            append(hexPrefix);
            FormatHex(data() + m_length, val, width);
            data()[m_length += width] = '\0';
        }
    }

    void appendSInt(const StringRef& hexPrefix, AMD_INT64 val, size_t width, bool prependPlus = true)
    {
        appendSign(val, prependPlus);
        appendUInt(hexPrefix, (AMD_UINT64)val, width, false);
    }
};

#endif // _STRINGREF_H_
