//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This worker thread is responsible for gathering request data sent back
/// from the server plugin and sending it back to the client
//==============================================================================

#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include "PluginResponseThread.h"
#include "../Common/SharedGlobal.h"
#include "../Common/SharedMemoryManager.h"
#include "../Common/NamedSemaphore.h"
#include "../Common/NamedEvent.h"
#include "../Common/ICommunication.h"
#include "../Common/OSWrappers.h"
#include "ProcessTracker.h"
#include "RequestsInFlightDatabase.h"

//--------------------------------------------------------------
/// Creates and then waits for the PLUGINS_TO_GPS_SEMAPHORE to be
/// signaled, then reads the responses in PLUGINS_TO_GPS shared
/// memory and sends them to the requester.
/// \param pData should be NULL (it is ignored though)
//--------------------------------------------------------------
void PluginResponseThread::WaitForPluginResponses(void* pData)
{
    PS_UNREFERENCED_PARAMETER(pData);

    //create a semaphore which allows putting at most MAX_SEM_COUNT response into SM;
    NamedSemaphore semaphore;
    semaphore.Create("PLUGINS_TO_GPS_SEMAPHORE");

    bool bEvent;

    NamedEvent shutdownEvent;
    bool opened = shutdownEvent.Open("GPS_SHUTDOWN_SERVER");

    while (opened && false == shutdownEvent.IsSignaled())
    {
        // wait for incoming signals from the plugin indicating that there is new data in shared memory
        bEvent = semaphore.Wait();

        if (bEvent == false)
        {
            Log(logERROR, "Failed to wait on an event (Error %d). Closing response thread.\n", osGetLastSystemError());
            smClose("PLUGINS_TO_GPS");
            break;
        }

        // check to see if the shutdownEvent has been signaled. When closing down, the PLUGINS_TO_GPS_SEMAPHORE is signaled to
        // unblock the semaphore.Wait() above, in which case there is no data in shared memory and the thread needs to exit now
        if (shutdownEvent.IsSignaled())
        {
            break;
        }

        // the PLUGINS_TO_GPS_EVENT was signaled
        // retrieve and send the response
        if (smLockGet("PLUGINS_TO_GPS"))
        {
            // read data from shared memory
            CommunicationID requestID = 0;
            char pcMimeType[PS_MAX_PATH];

            // the response was signaled, read out one response
            if (bEvent == true)
            {
                requestID = 0;
                memset(pcMimeType, 0, PS_MAX_PATH);
                char* pResponse = NULL;
                unsigned long uResponseSize = 0;

                LARGE_INTEGER nPreSharedMemoryGetTime;
                OSWrappers::QueryPerformanceCounter(&nPreSharedMemoryGetTime);

                if (smGet("PLUGINS_TO_GPS", &requestID, sizeof(CommunicationID)) == sizeof(CommunicationID))
                {
                    // successfully got the requestID

                    // Get the socket associated with this requestID and remove the socket since we're now donw with it.
                    NetSocket* client_socket = ProcessTracker::Instance()->GetSocketFromHandle(requestID);
                    ProcessTracker::Instance()->RemoveSocketFromMap(requestID);

#ifdef CODEXL_GRAPHICS
#ifdef USE_GRAPHICS_SERVER_STATUS_RETURN_CODES
                    // We can remove this message from the DB as we no longer need to monitor it anymore
                    RequestsInFlightDatabase::Instance()->Remove(client_socket);
#endif
#endif
                    // now try to get the mime type
                    if (smGet("PLUGINS_TO_GPS", &pcMimeType, PS_MAX_PATH) > 0)
                    {
                        // successfully got the mime type

                        // get response size
                        while (uResponseSize == 0)
                        {
                            uResponseSize = smGet("PLUGINS_TO_GPS", NULL, 0);
                        }

                        try
                        {
                            pResponse = new char[uResponseSize];
                            memset(pResponse, 0, uResponseSize * sizeof(char));
                        }
                        catch (std::bad_alloc)
                        {
                            Log(logERROR, "Failed to allocate memory for response of size: %lu\n", uResponseSize);
                            pResponse = NULL;
                        }

                        if (pResponse != NULL)
                        {
                            // Read from shared memory
                            if (smGet("PLUGINS_TO_GPS", pResponse, uResponseSize) == 0)
                            {
                                Log(logERROR, "Failed to get response from sharedMemory.\n");
                                smReset("PLUGINS_TO_GPS");
                            }

#ifdef DEBUG_COMMS_PERFORMANCE
                            // Record the time now
                            CommandTiming* pTiming = CommandTimingManager::Instance()->HandleResponse((NetSocket*)requestID);

                            if (pTiming != NULL)
                            {
                                LARGE_INTEGER nPerformanceCount;
                                QueryPerformanceCounter(&nPerformanceCount);
                                pTiming->SetWebServerRoundTripEnd(nPerformanceCount);    // Must set this one before SetPreSharedMemoryGet
                                pTiming->SetPreSharedMemoryGet(nPreSharedMemoryGetTime);
                                pTiming->SetResponseSize(uResponseSize);
                            }

#endif

                            // Send the data back to the client
                            SendMimeResponse(requestID, pcMimeType, pResponse, uResponseSize, client_socket);

#ifdef DEBUG_COMMS_PERFORMANCE
                            CommandTimingManager::Instance()->IncrementServerLoadingCount(-1);
#endif

                            SAFE_DELETE_ARRAY(pResponse);
                        }
                        else
                        {
                            smReset("PLUGINS_TO_GPS");
                            SendMimeResponse(requestID, "plain/text", "Error: Failed to get response from shared memory\n", 49 * sizeof(char), client_socket);
                        }

                    }
                    else
                    {
                        smReset("PLUGINS_TO_GPS");
                        SendMimeResponse(requestID, "plain/text", "Error: Could not read mime type\n", 32 * sizeof(char), client_socket);
                    }
                }
                else
                {
                    // in this case, we don't know socket the communication was on, so we can't send any errors or even close the socket
                    // just have to let the client time out.
                    smReset("PLUGINS_TO_GPS");
                    Log(logERROR, "Failed to get requestID from sharedMemory.\n");
                }

            } // end while sm has data or no responses have been read

            smUnlockGet("PLUGINS_TO_GPS");

        }
        else
        {
            Log(logERROR, "LockGet Failed\n");
        }
    }
    semaphore.Close();
    shutdownEvent.Close();
}
