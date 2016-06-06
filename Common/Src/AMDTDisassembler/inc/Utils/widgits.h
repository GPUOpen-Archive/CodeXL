//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file widgits.h
///
//==================================================================================

#ifndef _WIDGETS_H_
#define _WIDGETS_H_

#ifndef LINUX
#if _MSC_VER < 1300
#ifndef WIN64
inline ostream& operator << (ostream& stream, AMD_UINT64& val)
{
    switch (stream.flags() & ios::basefield)
    {
        case ios::hex:
        {
            AMD_UINT32* pHalfs = (AMD_UINT32*)(&val);
            stream.fill('0');

            if (stream.flags() & ios::showbase)
            {
                stream << "0x";
            }

            stream << setw(8) << pHalfs[1];
            stream << setw(8) << pHalfs[0];
        }
        break;

        case ios::dec:
        {
            char str[32];
            char* pos = &str[31];
            AMD_UINT64 value = val;
            *pos = '\0';

            do
            {
                *(--pos) = '0' + (value % 10);
                value /= 10;
            }
            while (value > 0);

            stream << pos;
        }
        break;

        default:
            stream << "octal not supported";
            break;
    }

    return stream;
}
#endif
#endif
#endif

#endif // _WIDGETS_H_
