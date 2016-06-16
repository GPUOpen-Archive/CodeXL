//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afUtils.cpp
///
//==================================================================================

// QtGui
#include <QtWidgets>

// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/src/afUtils.h>

// ---------------------------------------------------------------------------
// Name:        afUtils::addFieldToXML
// Description: Adds a field to the XML string
// Author:      Uri Shomroni
// Date:        22/5/2012
// ---------------------------------------------------------------------------
void afUtils::addFieldToXML(gtString& xmlString, const gtString& fieldName, const gtString& fieldValue)
{
    gtString fieldFinalValue = fieldValue;

    if (fieldFinalValue.isEmpty())
    {
        fieldFinalValue = OS_STR_EmptyXMLStringUnicode;
    }

    xmlString.appendFormattedString(L"<%ls>%ls</%ls>", fieldName.asCharArray(), fieldValue.asCharArray(), fieldName.asCharArray());
}
void afUtils::addFieldToXML(gtString& xmlString, const gtString& fieldName, int fieldValue)
{
    xmlString.appendFormattedString(L"<%ls>%d</%ls>", fieldName.asCharArray(), fieldValue, fieldName.asCharArray());
}
void afUtils::addFieldToXML(gtString& xmlString, const gtString& fieldName, bool fieldValue)
{
    xmlString.appendFormattedString(L"<%ls>%ls</%ls>", fieldName.asCharArray(), fieldValue ? OS_STR_TrueXMLValueUnicode : OS_STR_FalseXMLValueUnicode, fieldName.asCharArray());
}

// ---------------------------------------------------------------------------
// Name:        afUtils::getFieldFromXML
// Description: Gets a field's value from the main debug node
// Author:      Uri Shomroni
// Date:        23/5/2012
// ---------------------------------------------------------------------------
void afUtils::getFieldFromXML(TiXmlNode& debugNode, const gtString& fieldName, gtString& fieldValue)
{
    fieldValue.makeEmpty();
    TiXmlNode* pFieldNode = debugNode.FirstChild(fieldName.asASCIICharArray());

    if (nullptr != pFieldNode)
    {
        TiXmlNode* pFieldValueNode = pFieldNode->FirstChild();

        if (nullptr != pFieldValueNode)
        {
            TiXmlText* pFieldValueText = pFieldValueNode->ToText();

            if (nullptr != pFieldValueText)
            {
                QString utf8Value = QString::fromUtf8(pFieldValueText->Value());
                fieldValue = acQStringToGTString(utf8Value);
            }
        }
    }
}
void afUtils::getFieldFromXML(TiXmlNode& debugNode, const gtString& fieldName, int& fieldValue)
{
    gtString valueAsStr;
    getFieldFromXML(debugNode, fieldName, valueAsStr);

    if (!valueAsStr.isEmpty())
    {
        valueAsStr.toIntNumber(fieldValue);
    }
}
void afUtils::getFieldFromXML(TiXmlNode& debugNode, const gtString& fieldName, bool& fieldValue)
{
    gtString valueAsStr;
    getFieldFromXML(debugNode, fieldName, valueAsStr);

    if (!valueAsStr.isEmpty())
    {
        static const gtString trueXMLString = OS_STR_TrueXMLValueUnicode;
        fieldValue = (trueXMLString == valueAsStr);
    }
}

