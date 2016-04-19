//------------------------------ gdResumeDebuggingAfterMemoryLeakCheckEvent.h ------------------------------

#ifndef __GDRESUMEDEBUGGINGAFTERMEMORYLEAKCHECKEVENT_H
#define __GDRESUMEDEBUGGINGAFTERMEMORYLEAKCHECKEVENT_H

// Infra:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// Local:
#include <CodeXLAppCode/gdCodeXLAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdResumeDebuggingAfterMemoryLeakCheckEvent : public apEvent
// General Description: An event created by the memory viewer when it needs to resume
//                      debugging after checking for memory leaks. This event allows
//                      us to only issue the gaResumeDebuggedProcess function call after
//                      all other events in the queue were processed
// Author:              Uri Shomroni
// Creation Date:       18/8/2009
// ----------------------------------------------------------------------------------
class GD_API gdResumeDebuggingAfterMemoryLeakCheckEvent : public apEvent
{
public:
    gdResumeDebuggingAfterMemoryLeakCheckEvent();
    ~gdResumeDebuggingAfterMemoryLeakCheckEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;
};

#endif //__GDRESUMEDEBUGGINGAFTERMEMORYLEAKCHECKEVENT_H

