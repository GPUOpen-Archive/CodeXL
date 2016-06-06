//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMDIViewCreateEvent.h
///
//==================================================================================

//------------------------------ apMDIViewCreateEvent.h ------------------------------

#ifndef __APMDIVIEWCREATEEVENT_H
#define __APMDIVIEWCREATEEVENT_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Class Name:           apMDIViewCreateEvent : public apEvent
// General Description:  This class is used for MDI view open process. The event is
//                       registered whenever somewhere in the application there is a
//                       need for display of MDI view object
//                       This event should not be used in direct, but should be inherited
//                       and used for specific objects display
// Author:  AMD Developer Tools Team
// Creation Date:        23/8/2011
// ----------------------------------------------------------------------------------
class AP_API apMDIViewCreateEvent : public apEvent
{
public:
    apMDIViewCreateEvent(const gtString& createdMDIType, const osFilePath& filePath, const gtString& viewTitle, int viewIndex, int lineNumber, int programCounterIndex = -1);

    /// Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    /// Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    const gtString& CreatedMDIType() const { return m_createdMDIType; };

    void SetSecondFilePath(const osFilePath& filePath) { m_filePath2 = filePath; }
    void SetItemData(void* pData) { m_pItemData = pData; }
    void* ItemData() { return m_pItemData; }

    /// Self functions:
    const osFilePath& filePath() const { return m_filePath1; };
    const osFilePath& filePath2() const { return m_filePath2; };
    int viewIndex() const {return m_viewIndex;}
    gtString viewTitle() const {return m_viewTitle;}
    int lineNumber() const {return m_lineNumber;}
    int programCounterIndex() const {return m_programCounterIndex;}
protected:

    /// Do not allow the use of the default constructor:
    apMDIViewCreateEvent();

protected:

    /// The displayed file path:
    osFilePath m_filePath1;

    /// The second file path:
    osFilePath m_filePath2;

    /// The requested view index (within the view creator):
    int m_viewIndex;

    /// The view title:
    gtString m_viewTitle;

    /// The displayed line number:
    int m_lineNumber;

    /// A unique string, which is supplied by the user, and supposed to help locate the views
    /// creator related to this MDI create event:
    gtString m_createdMDIType;

    /// Program counter - for source files:
    int m_programCounterIndex;

    /// Void pointer - can be used to transfer data:
    void* m_pItemData;
};

#endif //__APMDIVIEWCREATEEVENT_H

