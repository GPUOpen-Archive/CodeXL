//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GPUProjectHandler.h $
/// \version $Revision: #8 $
/// \brief  This file contains GPU profile handler class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/GPUProjectHandler.h#8 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _GPU_PROJECT_HANDLER_H_
#define _GPU_PROJECT_HANDLER_H_
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtXml>
#include <QtWidgets>

// infra:
#include <AMDTBaseTools/Include/gtString.h>


/// XML data handler for project data
class GPUProjectHandler : public QXmlDefaultHandler
{
public :

    /// Get project settings in XML format in a string.
    /// \param projectAsXMLString will contain project setting info in XML format
    /// \param projectPage project setting page name
    /// \return True on success, else false
    virtual bool getProjectSettingsXML(gtString& projectAsXMLString, gtString& projectPage);

    /// Set project settings info available in XML format in the string.
    /// \param projectAsXMLString will contain project setting info in XML format
    /// \return True on success, else false
    virtual bool setProjectSettingsXML(const gtString& projectAsXMLString);

    /// Create string in XML format for settings of the page
    /// \param projectAsXMLString the XML string
    /// \param type the type
    virtual void writeSession(gtString& projectAsXMLString, const gtString& type) = 0;

    /// Will update Tree widget nodes from XML
    /// \param node Will contains XML data
    /// \param tree Will update tree items as per XML data
    /// \param shouldSearchOrigText when true, the tree node item is searched for the exact text in node. Otherwise, the string searched is the original, with "_" replaced by " ":
    virtual void UpdateTreeWidgetFromXML(QDomNode node, QTreeWidget* tree, bool shouldSearchOrigText);

    /// Will update Tree widget nodes from XML
    /// \param node Will contains counters enabled / disabled data
    /// \param tree Will update tree items as per XML data
    /// \param shouldSearchOrigText when true, the tree node item is searched for the exact text in node. Otherwise, the string searched is the original, with "_" replaced by " ":
    virtual void UpdateTreeWidgetFromCountersList(QTreeWidget* pCountersTree, const QMap<QString, bool>& checkedCounterList, bool shouldSearchOrigText);

    /// Update the counters checked list from the XML:
    /// \param node Will contains XML data
    /// \param map of counter name to enabled / disabled flag
    /// \param shouldSearchOrigText when true, the tree node item is searched for the exact text in node. Otherwise, the string searched is the original, with "_" replaced by " ":
    virtual void UpdateCountersCheckedListFromXML(QDomNode node, QMap<QString, bool>& checkedCounterList, bool shouldSearchOrigText);

    /// Will append TreeWidget in XML format
    /// \param projectAsXMLString out string which will have final value.
    /// \param treeName tree tag name
    /// \param list will have tree list.
    virtual void AppendTree(gtString& projectAsXMLString, const QString& treeName, QMap<QString, bool>& list);

    /// Will append TreeWidget in XML format
    /// \param projectAsXMLString out string which will have final value.
    /// \param treeName tree tag name
    /// \param list will have tree list
    virtual void AppendTree(gtString& projectAsXMLString, const QString& treeName, const QStringList& list);

protected:

    /// Write value in XML format
    /// \param projectAsXMLString Will have XML format string
    /// \param key XML key
    /// \param value Value of the XML key
    virtual void writeValue(gtString& projectAsXMLString, const gtString& key, const gtString& value);

    /// Write bool value in XML format
    /// \param projectAsXMLString Will have XML format string
    /// \param key XML key
    /// \param value Boolean value of the XML key
    virtual void writeBool(gtString& projectAsXMLString, const gtString& key, const bool value);
};


#endif //_GPU_PROJECT_HANDLER_H_
