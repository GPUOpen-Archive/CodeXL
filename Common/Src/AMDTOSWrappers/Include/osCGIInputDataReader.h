//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCGIInputDataReader.h
///
//=====================================================================

//------------------------------ osCGIInputDataReader.h ------------------------------

#ifndef __OSCGIINPUTDATAREADER_H
#define __OSCGIINPUTDATAREADER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osCGIInputDataItem
// General Description:
//   Represents a data item, passed from a Web form into a CGI-bin.
//
// Author:      AMD Developer Tools Team
// Creation Date:        osCGIVariable
// ----------------------------------------------------------------------------------
struct OS_API osCGIInputDataItem
{
    // The item name:
    gtString _name;

    // The item value:
    gtString _value;
};


// ----------------------------------------------------------------------------------
// Class Name:           osCGIInputDataReader
// General Description:
//   Enables reading the data items send from a web form into a CGI-bin.
//   This class should be used inside the CGI-bin.
//
// Author:      AMD Developer Tools Team
// Creation Date:        11/5/2008
// ----------------------------------------------------------------------------------
class OS_API osCGIInputDataReader
{
public:
    osCGIInputDataReader();
    virtual ~osCGIInputDataReader();

    bool readGetInputData();
    bool readPostInputData();

    int amountOfDataItems() const;
    bool getDataItem(int itemIndex, osCGIInputDataItem& itemData) const;
    bool getDataItemValue(const gtString& dataItemName, gtString& dataItemValue) const;

    bool getClientIPAddress(gtString& clientIPAddress) const;
    bool getServerName(gtString& serverName) const;
    bool getServerPort(unsigned int& serverPort) const;
    bool getRemoteHost(gtString& serverName) const;

private:
    bool readRequestString(const gtString& requestString);
    bool decodeFormRequestString(const gtString& encodedString, gtString& decodedString);
    wchar_t hexToChar(wchar_t* pHexValue) const;
    void debugLogRequestString(const gtString& requestString);

private:
    // Contains the data items send from the form into the CGI-bin:
    gtPtrVector<osCGIInputDataItem*> _dataItems;
};


#endif //__OSCGIINPUTDATAREADER_H

