//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLinuxDebuggedApplicationOutputReaderThread.h
///
//==================================================================================

//------------------------------ pdLinuxDebuggedApplicationOutputReaderThread.h ------------------------------

#ifndef __PDLINUXDEBUGGEDAPPLICATIONOUTPUTREADERTHREAD_H
#define __PDLINUXDEBUGGEDAPPLICATIONOUTPUTREADERTHREAD_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osThread.h>

// Predeclarations:
class osPipeSocketServer;

class pdLinuxDebuggedApplicationOutputReaderThread : public osThread
{
public:
    pdLinuxDebuggedApplicationOutputReaderThread();
    ~pdLinuxDebuggedApplicationOutputReaderThread();

    // Overrides osThread
    virtual int entryPoint();
    virtual void beforeTermination();

    const gtString& getPipeFilePath() const {return _pipeFilePath;};

private:
    // The path to the file where we write the output strings:
    gtString _pipeFilePath;

    // The current output:
    gtASCIIString _accumulatedOutput;

    // pipe to the file to be read:
    FILE* m_pPipe;
};

#endif //__PDLINUXDEBUGGEDAPPLICATIONOUTPUTREADERTHREAD_H
