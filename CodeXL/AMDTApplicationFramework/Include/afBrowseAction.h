//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afBrowseAction.h
///
//==================================================================================

#ifndef __AFBROWSEACTION_H
#define __AFBROWSEACTION_H

// Qt:
#include <QAction>


// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          afBrowseAction
// General Description: This class inherits QPushButton. Use this class when a you want to implement
//                      a button that implements browsing, and you want to store the last browsed location
// ----------------------------------------------------------------------------------
class AF_API afBrowseAction : public QAction
{
    Q_OBJECT

public:

    /// Constructor:
    afBrowseAction(const QString& actionID);
    afBrowseAction(QAction* pAction);

    /// Destructor:
    virtual ~afBrowseAction();

    /// Accessors:
    QString LastBrowsedFolder() const { return m_lastBrowsedFolder; };

    /// Set the last browsed folder:
    /// \param lastBrowsedFolder the last browsed folder
    void SetLastBrowsedFolder(const QString& lastBrowsedFolder);

private:

    /// Initializes:
    void Initilaize();

    /// Contain the last browsed location:
    QString m_lastBrowsedFolder;


};

#endif //__AFBROWSEACTION_H

