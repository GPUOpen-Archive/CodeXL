//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the locale settings.
//==============================================================================

#ifndef _LOCALE_SETTING_H_
#define _LOCALE_SETTING_H_

#include <locale>

/// \addtogroup Common
// @{

/// This class manages the locale settings
class LocaleSetting
{
public:
    /// retrieve the list separator in the current locale
    /// \return a list separator
    static char GetListSeparator();
};

#endif //_LOCALE_SETTING_H_

// @}
