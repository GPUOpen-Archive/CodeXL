//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osProductVersion.cpp
///
//=====================================================================

//------------------------------ osProductVersion.cpp ------------------------------

// Local:
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>

// ---------------------------------------------------------------------------
// Name:        osProductVersion::osProductVersion
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        29/6/2004
// ---------------------------------------------------------------------------
osProductVersion::osProductVersion()
    : _majorVersion(0), _minorVersion(0), _patchNumber(0), _revisionNumber(0)
{
}


// ---------------------------------------------------------------------------
// Name:        osProductVersion::initToZeroVersion
// Description: Initialize this class to contains the zero product version.
// Author:      AMD Developer Tools Team
// Date:        19/5/2008
// ---------------------------------------------------------------------------
void osProductVersion::initToZeroVersion()
{
    _majorVersion = 0;
    _minorVersion = 0;
    _patchNumber = 0;
    _revisionNumber = 0;
}


// ---------------------------------------------------------------------------
// Name:        osProductVersion::fromString
// Description: Takes a string in the format x.y.z.w and turns it into the variable
// Arguments: versionAsString Should be in the format x.y.z.w where w-z are ints
// Return Val: bool  - Success / failure (fails if string is not of format x.y.z.w
// Author:      AMD Developer Tools Team
// Date:        2/4/2008
// ---------------------------------------------------------------------------
bool osProductVersion::fromString(const gtString& versionAsString)
{
    struct TempStructType
    {
        int _majorVersion;
        int _minorVersion;
        int _patchNumber;
        int _revisionNumber;
    };
    TempStructType temp;

    if (versionAsString.count('.') != 3) // string not of format x.y.z.w is invalid!
    {
        temp._majorVersion = 0;
        temp._minorVersion = 0;
        temp._patchNumber = 0;
        temp._revisionNumber = 0;
        return false;
    }
    else // cuts the string into four parts separated by the . character
    {
        int lastSeperator = 0;
        int currentSeperator = versionAsString.find('.', lastSeperator);
        int currentPart = 0;
        gtString currentPartAsString;
        versionAsString.getSubString(lastSeperator, currentSeperator - 1, currentPartAsString);

        if (!currentPartAsString.toIntNumber(currentPart))
        {
            currentPart = 0;
        }

        temp._majorVersion = currentPart;
        lastSeperator = currentSeperator + 1;
        currentSeperator = versionAsString.find('.', lastSeperator);
        versionAsString.getSubString(lastSeperator, currentSeperator - 1, currentPartAsString);

        if (!currentPartAsString.toIntNumber(currentPart))
        {
            currentPart = 0;
        }

        temp._minorVersion = currentPart;
        lastSeperator = currentSeperator + 1;
        currentSeperator = versionAsString.find('.', lastSeperator);
        versionAsString.getSubString(lastSeperator, currentSeperator - 1, currentPartAsString);

        if (!currentPartAsString.toIntNumber(currentPart))
        {
            currentPart = 0;
        }

        temp._patchNumber = currentPart;
        lastSeperator = currentSeperator + 1;
        currentSeperator = versionAsString.find('.', lastSeperator);
        versionAsString.getSubString(lastSeperator, currentSeperator - 1, currentPartAsString);

        if (!currentPartAsString.toIntNumber(currentPart))
        {
            currentPart = 0;
        }

        temp._revisionNumber = currentPart;
    }

    // Output the product version:
    this->_majorVersion = temp._majorVersion;
    this->_minorVersion = temp._minorVersion;
    this->_patchNumber = temp._patchNumber;
    this->_revisionNumber = temp._revisionNumber;

    return true;
}



// ---------------------------------------------------------------------------
// Name:        osProductVersion::fromString
// Description: Takes a string in the format x.y.z.w and turns it into the variable - ASCII version
// Arguments:   gtASCIIString const& version
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osProductVersion::fromString(const gtASCIIString& version)
{
    // Convert the string to a unicode string:
    gtString versionUnicode;
    versionUnicode.fromASCIIString(version.asCharArray());
    // Call the unicode version:
    return fromString(versionUnicode);
}
// ---------------------------------------------------------------------------
// Name:        osProductVersion::toString
// Description: Turns the version into a string of format x.y.z.w
//              If fullNumber = false, only displays the major and minor version, and the patch number
//              if it's not 0.
// Author:      AMD Developer Tools Team
// Date:        2/4/2008
// ---------------------------------------------------------------------------
gtString osProductVersion::toString(bool fullNumber) const
{
    gtString retString;

    if (fullNumber)
    {
        retString.appendFormattedString(L"%d.%d.%d.%d", _majorVersion, _minorVersion, _patchNumber, _revisionNumber);
    }
    else // !fullNumber
    {
        if (_patchNumber > 0)
        {
            retString.appendFormattedString(L"%d.%d.%d", _majorVersion, _minorVersion, _patchNumber);
        }
        else // _patchNumber <= 0
        {
            retString.appendFormattedString(L"%d.%d", _majorVersion, _minorVersion);
        }
    }

    return retString;
}

// ---------------------------------------------------------------------------
// Name:        osProductVersion::operator<
// Description: compares two versions.
// Return Val: bool  - true if first is older.
// Author:      AMD Developer Tools Team
// Date:        2/4/2008
// ---------------------------------------------------------------------------
bool osProductVersion::operator<(const osProductVersion& otherVersion) const
{
    bool retVal = false;

    if (_majorVersion < otherVersion._majorVersion)
    {
        retVal = true;
    }
    else if (_majorVersion > otherVersion._majorVersion)
    {
        retVal = false;
    }
    else if (_minorVersion < otherVersion._minorVersion)
    {
        retVal = true;
    }
    else if (_minorVersion > otherVersion._minorVersion)
    {
        retVal = false;
    }
    else if (_patchNumber < otherVersion._patchNumber)
    {
        retVal = true;
    }
    else if (_patchNumber > otherVersion._patchNumber)
    {
        retVal = false;
    }
    else if (_revisionNumber < otherVersion._revisionNumber)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osProductVersion::operator>
// Description: compares two versions.
// Return Val: bool  - true if first is newer.
// Author:      AMD Developer Tools Team
// Date:        2/4/2008
// ---------------------------------------------------------------------------
bool osProductVersion::operator>(const osProductVersion& otherVersion) const
{
    return otherVersion < *this;
}


// ---------------------------------------------------------------------------
// Name:        osProductVersion::operator==
// Description: compares two product versions.
// Author:      AMD Developer Tools Team
// Date:        3/6/2008
// ---------------------------------------------------------------------------
bool osProductVersion::operator==(const osProductVersion& otherVersion) const
{
    bool retVal = false;

    if (_majorVersion == otherVersion._majorVersion)
    {
        if (_minorVersion == otherVersion._minorVersion)
        {
            if (_patchNumber == otherVersion._patchNumber)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

