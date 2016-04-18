//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLinuxDebuggedApplicationOutputReaderThread.cpp
///
//==================================================================================

//------------------------------ pdLinuxDebuggedApplicationOutputReaderThread.cpp ------------------------------

// POSIX
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Local:
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>
#include <src/pdLinuxDebuggedApplicationOutputReaderThread.h>
#include <src/pdStringConstants.h>

#define PD_MAX_FILE_SIZE 65534

// ---------------------------------------------------------------------------
// Name:        pdLinuxDebuggedApplicationOutputReaderThread::pdLinuxDebuggedApplicationOutputReaderThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/3/2009
// ---------------------------------------------------------------------------
pdLinuxDebuggedApplicationOutputReaderThread::pdLinuxDebuggedApplicationOutputReaderThread()
    : osThread(L"pdLinuxDebuggedApplicationOutputReaderThread", true), m_pPipe(NULL)
{
    gtString fileName = PD_STR_outputReaderThreadOutputFilePrefix;
    osTime currentTime;
    currentTime.setFromCurrentTime();
    gtString dateAsString;
    currentTime.dateAsString(dateAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
    gtString timeAsString;
    currentTime.timeAsString(timeAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
    fileName.append(dateAsString).append('-').append(timeAsString);

    osFilePath pipeFilePath(osFilePath::OS_TEMP_DIRECTORY);
    pipeFilePath.setFileName(fileName);
    pipeFilePath.setFileExtension(L"out");

    _pipeFilePath = pipeFilePath.asString();
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxDebuggedApplicationOutputReaderThread::~pdLinuxDebuggedApplicationOutputReaderThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/3/2009
// ---------------------------------------------------------------------------
pdLinuxDebuggedApplicationOutputReaderThread::~pdLinuxDebuggedApplicationOutputReaderThread()
{
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxDebuggedApplicationOutputReaderThread::entryPoint
// Description: Thread entry point, opens the file for writing.
// Author:      Uri Shomroni
// Date:        10/3/2009
// ---------------------------------------------------------------------------
int pdLinuxDebuggedApplicationOutputReaderThread::entryPoint()
{
    gtString command(L"");
    command.append(_pipeFilePath);
    // open the pipe only when the file was created:
    osFilePath filePath(_pipeFilePath);

    while (!filePath.exists());

    m_pPipe = fopen(command.asASCIICharArray(), "r");

    if (m_pPipe != NULL)
    {
        int c;
        int bufferCount = 0;

        // read the pipe
        for (;;)
        {
            c = fgetc(m_pPipe);

            if (0 != c && -1 != c)
            {
                _accumulatedOutput.append((char) c);
                bufferCount++;
            }
            else
            {
                // wait a bit until the file gets some characters into it.
                sleep(1);
            }

            // check if a line of data was read and send it to the output view
            // or buffer is very large (PD_MAX_FILE_SIZE characters)
            if (('\n' == c) || ('\r' == c) || (bufferCount > PD_MAX_FILE_SIZE))
            {
                // if buffer is large and there is no end of line add it to the end of the buffer:
                if ((bufferCount > PD_MAX_FILE_SIZE) && ('\n' != c) && ('\r' != c))
                {
                    _accumulatedOutput.append('\n');
                }

                // end the line and send it to the output view:
                gtString finalString;
                finalString.fromASCIIString(_accumulatedOutput.asCharArray());
                apOutputDebugStringEvent debugStringEve(OS_NO_THREAD_ID, finalString, apOutputDebugStringEvent::AP_GENERAL_OUTPUT_VIEW);
                apEventsHandler::instance().registerPendingDebugEvent(debugStringEve);
                _accumulatedOutput = "";
                bufferCount = 0;
            }
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Name:        pdLinuxDebuggedApplicationOutputReaderThread::beforeTermination
// Description: Called before the thread is terminated
// Author:      Uri Shomroni
// Date:        10/3/2009
// ---------------------------------------------------------------------------
void pdLinuxDebuggedApplicationOutputReaderThread::beforeTermination()
{
    if (NULL != m_pPipe)
    {
        fclose(m_pPipe);
    }
}
