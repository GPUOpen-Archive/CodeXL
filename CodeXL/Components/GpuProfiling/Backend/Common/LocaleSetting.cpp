//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the locale settings.
//==============================================================================

#ifdef _WIN32
    #include <windows.h>
#endif //_WIN32
#include "LocaleSetting.h"
#include "Defs.h"

#ifdef _WIN32

char LocaleSetting::GetListSeparator()
{
    // set the current locale to be based on the current OS settings
    // user can override this settings by using a LANG environment variable (see the setlocale's manual)
    setlocale(LC_ALL, "");

    char cList = ',';  // the default setting

    // get the list separator from the registry key
    HKEY hKey;

    SP_TODO("revisit use of RegOpenKeyExA for Unicode support")

    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Control Panel\\International", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        char sList[256];
        DWORD dwSize = sizeof(sList);

        SP_TODO("revisit use of RegQueryValueExA for Unicode support")

        if (RegQueryValueExA(hKey, "sList", NULL, NULL, (BYTE*) sList, &dwSize) == ERROR_SUCCESS)
        {
            cList = sList[0];
        }

        RegCloseKey(hKey);
    }

    return cList;
}

#else

#include <langinfo.h>

// Use default separator for linux
char LocaleSetting::GetListSeparator()
{
    // use comma for the list separator, unless comma is also used for the decimal character, then use a semi-colon.
    // there doesn't appear to be a "list separator" character on Linux, like there is on Windows, so we need to try
    // our best to use a character than doesn't conflict with another locale-specific character (like the comma decimal
    // character in some cases)
    char retVal = ',';

    char* current_locale;
    char* saved_locale;

    // get current locale
    current_locale = setlocale(LC_ALL, NULL);

    // save a copy so it doesn't get overwritten by next call to setlocale
    saved_locale = strdup(current_locale);

    if (NULL == saved_locale)
    {
        // strdup failed
        return retVal;
    }

    // set the current locale to be based on the current OS settings
    // user can override this settings by using a LANG environment variable (see the setlocale's manual)
    setlocale(LC_ALL, "");

    std::string decimalChar = nl_langinfo(RADIXCHAR);

    // restore original locale
    setlocale(LC_ALL, saved_locale);
    free(saved_locale);

    if (decimalChar == ",")
    {
        retVal = ';';
    }

    return retVal;
}

#endif //_WIN32
