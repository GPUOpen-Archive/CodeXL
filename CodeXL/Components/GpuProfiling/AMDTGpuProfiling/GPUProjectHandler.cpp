//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GPUProjectHandler.cpp $
/// \version $Revision: #12 $
/// \brief  This file contains GPU profile handler class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GPUProjectHandler.cpp#12 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#include "GPUProjectHandler.h"
#include <AMDTGpuProfiling/Util.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>



void GPUProjectHandler::writeValue(gtString& projectAsXMLString, const gtString& key, const gtString& value)
{
    projectAsXMLString.appendFormattedString(L"<%ls>%ls</%ls>", key.asCharArray(), value.asCharArray(), key.asCharArray());
}

void GPUProjectHandler::writeBool(gtString& projectAsXMLString, const gtString& key, const bool value)
{
    gtString val;
    val = value ? L"T" : L"F";
    writeValue(projectAsXMLString, key, val);
}

bool GPUProjectHandler::getProjectSettingsXML(gtString& projectAsXMLString, gtString& projectPage)
{
    gtString numVal;
    projectAsXMLString.appendFormattedString(L"<%ls>", projectPage.asCharArray());
    writeSession(projectAsXMLString, L"Current");
    projectAsXMLString.appendFormattedString(L"</%ls>", projectPage.asCharArray());

    return true;
}

bool GPUProjectHandler::setProjectSettingsXML(const gtString& projectAsXMLString)
{
    QXmlInputSource source;
    source.setData(acGTStringToQString(projectAsXMLString));
    QXmlSimpleReader reader ;

    // Connect this object's handler interface to the XML reader
    reader.setContentHandler(this) ;
    reader.setErrorHandler(this);

    return reader.parse(source);
}


void GPUProjectHandler::AppendTree(gtString& projectAsXMLString, const QString& treeName, QMap<QString, bool>& list)
{
    QString tagName = "<" + treeName + ">";
    projectAsXMLString.append(acQStringToGTString(tagName));
    QString strKey;

    if (list.count() > 0)
    {
        QMap<QString, bool>::const_iterator iterator = list.constBegin();

        for (; iterator != list.constEnd(); iterator++)
        {
            strKey = iterator.key();
            strKey = strKey.replace(' ', "_");
            writeBool(projectAsXMLString, acQStringToGTString(strKey), iterator.value());
        }
    }

    tagName = "</" + treeName + ">";
    projectAsXMLString.append(acQStringToGTString(tagName));
}


void GPUProjectHandler::AppendTree(gtString& projectAsXMLString, const QString& treeName, const QStringList& list)
{
    QString tagName = "<" + treeName + ">";
    projectAsXMLString.append(acQStringToGTString(tagName));
    QString strKey;

    if (list.count() > 0)
    {
        QStringList::const_iterator iterator = list.constBegin();

        for (; iterator != list.constEnd(); iterator++)
        {
            strKey = *iterator;
            writeBool(projectAsXMLString, acQStringToGTString(strKey), true);
        }
    }

    tagName = "</" + treeName + ">";
    projectAsXMLString.append(acQStringToGTString(tagName));
}


void GPUProjectHandler::UpdateTreeWidgetFromXML(QDomNode node, QTreeWidget* pTreeWidget, bool shouldSearchOrigText)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pTreeWidget != nullptr)
    {
        QString value;
        bool val = false;
        QDomNode RootNode = node;
        QDomNode ChildNode = RootNode.firstChild();

        bool setInitialState = false;

        while (!ChildNode.isNull())
        {
            value = ChildNode.firstChild().nodeValue();

            if (!setInitialState)
            {
                // check to see if the first item's value is "F".  If so, then this is a CodeXL 1.0
                // project file where only unchecked items were written to the project file.
                Util::SetCheckState(pTreeWidget, value == "F");
                setInitialState = true;
            }

            // Get the value from the XML ("T" / "F"):
            val = (value == "T") ? true : false;

            // Get the tree item expected text:
            QString itemText = ChildNode.nodeName();

            if (!shouldSearchOrigText)
            {
                itemText = itemText.replace('_', " ");
            }

            Util::SetTreeWidgetItemChecked(pTreeWidget, itemText, val, true);
            ChildNode = ChildNode.nextSibling();
        }
    }
}



void GPUProjectHandler::UpdateTreeWidgetFromCountersList(QTreeWidget* pCountersTree, const QMap<QString, bool>& checkedCounterList, bool shouldSearchOrigText)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pCountersTree != nullptr)
    {
        QTreeWidgetItemIterator iter(pCountersTree);

        // First uncheck all:
        while ((*iter) != nullptr)
        {
            QTreeWidgetItem* pItem = (*iter);
            QString counterName = pItem->text(0);

            if (!shouldSearchOrigText)
            {
                counterName = counterName.replace('_', " ");
            }

            if (checkedCounterList.contains(counterName))
            {
                Qt::CheckState state = (checkedCounterList[counterName] == true) ? Qt::Checked : Qt::Unchecked;
                pItem->setCheckState(0, state);
            }

            ++iter;
        }
    }
}

void GPUProjectHandler::UpdateCountersCheckedListFromXML(QDomNode node, QMap<QString, bool>& checkedCounterList, bool shouldSearchOrigText)
{

    QString value;
    bool val = false;
    QDomNode rootNode = node;
    QDomNode childNode = rootNode.firstChild();

    auto iter = checkedCounterList.begin();
    auto iterEnd = checkedCounterList.end();

    for (; iter != iterEnd; iter++)
    {
        checkedCounterList[(iter.key())] = false;
    }

    while (!childNode.isNull())
    {
        value = childNode.firstChild().nodeValue();

        // Get the value from the XML ("T" / "F"):
        val = (value == "T") ? true : false;

        // Get the tree item expected text:
        QString itemText = childNode.nodeName();

        if (!shouldSearchOrigText)
        {
            itemText = itemText.replace('_', " ");
        }

        checkedCounterList[itemText] = val;
        childNode = childNode.nextSibling();
    }
}

