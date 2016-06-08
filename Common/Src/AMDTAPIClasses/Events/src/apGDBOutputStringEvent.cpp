//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGDBOutputStringEvent.cpp
///
//==================================================================================

//------------------------------ apGDBOutputStringEvent.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apGDBOutputStringEvent.h>


// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::apGDBOutputStringEvent
// Description: Constructor
// Arguments:  gdbOutputString - The GDB outputted string.
// Author:  AMD Developer Tools Team
// Date:        28/12/2006
// ---------------------------------------------------------------------------
apGDBOutputStringEvent::apGDBOutputStringEvent(const gtString& gdbOutputString)
    : apEvent(OS_NO_THREAD_ID), _gdbOutputString(gdbOutputString)
{
    // Translate the GDB MI string to a human readable string:
    gtString humanReadableStr;
    gdbMIStringToReadableString(gdbOutputString, humanReadableStr);

    // Store the string:
    _gdbOutputString = humanReadableStr;
}

// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apGDBOutputStringEvent::type() const
{
    return OS_TOBJ_ID_GDB_OUTPUT_STRING_EVENT;
}

// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::writeSelfIntoChannel
// Description: Writes this class data into a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apGDBOutputStringEvent::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    // Write the string:
    ipcChannel << _gdbOutputString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::readSelfFromChannel
// Description: Reads this class data from a communication channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        11/8/2009
// ---------------------------------------------------------------------------
bool apGDBOutputStringEvent::readSelfFromChannel(osChannel& ipcChannel)
{
    // Read the string:
    ipcChannel >> _gdbOutputString;

    // Call my parent class's version of this function:
    bool retVal = apEvent::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::type
// Description: Returns my debugged process event type.
// Author:  AMD Developer Tools Team
// Date:        28/12/2006
// ---------------------------------------------------------------------------
apEvent::EventType apGDBOutputStringEvent::eventType() const
{
    return apEvent::AP_GDB_OUTPUT_STRING;
}


// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::clone
// Description: Creates a new copy of self, and returns it.
//              It is the caller responsibility to delete the created copy.
// Author:  AMD Developer Tools Team
// Date:        28/12/2006
// ---------------------------------------------------------------------------
apEvent* apGDBOutputStringEvent::clone() const
{
    apGDBOutputStringEvent* pEventCopy = new apGDBOutputStringEvent(_gdbOutputString);
    return pEventCopy;
}


// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::clone
// Description: Translated GDB MI (machine interface) string into a human
//              readable string.
// Arguments:   gdbMIString - The input GDB MI string.
//              readableString - The output human readable string.
// Author:  AMD Developer Tools Team
// Date:        28/12/2006
// ---------------------------------------------------------------------------
void apGDBOutputStringEvent::gdbMIStringToReadableString(const gtString& gdbMIString,
                                                         gtString& readableString)
{
    // If the string starts with a GDB MI prefix:
    if ((gdbMIString[0] == '~') || (gdbMIString[0] == '@') ||
        (gdbMIString[0] == '&') || (gdbMIString[0] == '*'))
    {
        // Remove the prefix and the \ that follows:
        gdbMIString.getSubString(1, gdbMIString.length(), readableString);
    }
    else
    {
        readableString = gdbMIString;
    }

    // The GDB MI string contains printf like format string.
    // To convert it into human readable string, we need to remove
    // string formatters:

    // Remove new line strings:
    readableString.replace(L"\\n", L" ", true);

    // Remove quote strings:
    readableString.replace(L"\\\"", L"\"", true);

    // Remove new lines:
    readableString.replace(L"\n", L" ", true);

    // Remove the first and the last quotes:
    int firstQuotePos = readableString.find('\"');
    int lastQuotePos = readableString.reverseFind('\"');

    if ((firstQuotePos != -1) || (lastQuotePos != -1))
    {
        readableString[firstQuotePos] = ' ';
        readableString[lastQuotePos] = ' ';
    }
}

// ---------------------------------------------------------------------------
// Name:        apGDBOutputStringEvent::apGDBOutputStringEvent
// Description: Default constructor, only allowed for use by osTransferableObjectCreator
// Author:  AMD Developer Tools Team
// Date:        12/8/2009
// ---------------------------------------------------------------------------
apGDBOutputStringEvent::apGDBOutputStringEvent()
{

}



