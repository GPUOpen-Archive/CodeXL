//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afUtils.h
///
//==================================================================================

#ifndef __AFUTILS
#define __AFUTILS

// QtGui
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class TiXmlNode;

// ----------------------------------------------------------------------------------
// Class Name:           afUtils
// General Description:
//  Utility class for static utility functions.
//
// Author:               Doron Ofek
// Creation Date:        Oct-21, 2012
// ----------------------------------------------------------------------------------
class AF_API afUtils
{

public:


    void static addFieldToXML(gtString& xmlString, const gtString& fieldName, const gtString& fieldValue);
    void static addFieldToXML(gtString& xmlString, const gtString& fieldName, int fieldValue);
    void static addFieldToXML(gtString& xmlString, const gtString& fieldName, bool fieldValue);
    void static getFieldFromXML(TiXmlNode& debugNode, const gtString& fieldName, gtString& fieldValue);
    void static getFieldFromXML(TiXmlNode& debugNode, const gtString& fieldName, int& fieldValue);
    void static getFieldFromXML(TiXmlNode& debugNode, const gtString& fieldName, bool& fieldValue);

};

#endif  // __AFUTILS
