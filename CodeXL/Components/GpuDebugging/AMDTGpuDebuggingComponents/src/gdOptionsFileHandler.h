//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdOptionsFileHandler.h
///
//==================================================================================

//------------------------------ gdOptionsFileHandler.h ------------------------------

#ifndef __GDOPTIONSFILEHANDLER_H
#define __GDOPTIONSFILEHANDLER_H

// Forward decelerations:
class osPortAddress;

// Infra:
#include <AMDTOSWrappers/Include/osSettingsFileHandler.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdOptionsFileHandler : public osSettingsFileHandler
// General Description:
//    Responsible for loading and storing the options file and its data.
//
// Author:      Yaki Tebeka
// Date:        11/11/2007
// ----------------------------------------------------------------------------------
class gdOptionsFileHandler : public osSettingsFileHandler
{
public:
    gdOptionsFileHandler(const osFilePath& optionsFilePath);
    virtual ~gdOptionsFileHandler();

    bool updateXMLDocumentFromApplicationValues();
    bool updateApplicationValuesFromXMLDocument();

protected:
    virtual bool initXMLDocToSettingsFileDefaultValues(TiXmlDocument& XMLDocument);

private:
    bool writeAdditionalSourceCodeDirectoriesStringToXml(TiXmlNode* pXmlNode, gtString dataFieldValue);
    bool readAdditionalDirectoriesFromXml(const TiXmlNode* pXmlNode, gtString& string);

    // Do not allow the use of my default constructor:
    gdOptionsFileHandler();
};

#endif //__GDOPTIONSFILEHANDLER_H

