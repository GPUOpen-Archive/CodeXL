//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class detects excessive synchronization APIs
//==============================================================================

#include <sstream>
#include "../Common/Logger.h"
#include "../Common/StringUtils.h"
#include "CLSyncAnalyzer.h"
#include "CLObjRefTracker.h"

using namespace std;
using namespace GPULogger;

#define CHECK_NULL_RET(obj) \
    SpAssert(obj != NULL);   \
    if (obj == NULL)         \
        return;


CLSyncAnalyzer::CLSyncAnalyzer(CLAPIAnalyzerManager* p)
    : CLAPIAnalyzer(p)
{
    m_strName = "SyncAnalyzer";

    m_bRequireAPIFlattening = true;
    m_dependentAPIs.insert(CL_FUNC_TYPE_clFinish);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clWaitForEvents);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clGetEventInfo);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateCommandQueue);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateCommandQueueWithProperties);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseCommandQueue);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainCommandQueue);

    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueReadBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueReadBufferRect);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueWriteBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueWriteBufferRect);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueCopyBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueCopyBufferRect);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueReadImage);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueWriteImage);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueCopyImage);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueCopyImageToBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueCopyBufferToImage);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueUnmapMemObject);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueNDRangeKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueTask);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueNativeKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueAcquireGLObjects);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMarker);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMarkerWithWaitList);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueReleaseGLObjects);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMapBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMapImage);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueBarrier);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueBarrierWithWaitList);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueFillBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueFillImage);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMigrateMemObjects);
}

CLSyncAnalyzer::~CLSyncAnalyzer(void)
{
    for (CommandQueueMap::iterator it = m_cmdQueueMap.begin(); it != m_cmdQueueMap.end(); ++it)
    {
        if (it->second)
        {
            delete it->second;
        }
    }

    m_cmdQueueMap.clear();
}

bool IsEnqueueCmd(CLAPIInfo* pAPIInfo)
{
    return (pAPIInfo->m_Type == CL_ENQUEUE_MEM ||
            pAPIInfo->m_Type == CL_ENQUEUE_KERNEL ||
            pAPIInfo->m_Type == CL_ENQUEUE_BASE_API);
}

void CLSyncAnalyzer::Analyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
}

void CLSyncAnalyzer::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(pAPIInfo);

    if (nullptr != pCLApiInfo)
    {
        switch (pCLApiInfo->m_uiAPIID)
        {
            case CL_FUNC_TYPE_clFinish:
            case CL_FUNC_TYPE_clWaitForEvents:
                apiBuffer.push_back(pCLApiInfo);
                break;

            case CL_FUNC_TYPE_clGetEventInfo:

                if (pCLApiInfo->m_strComment.find("consecutive calls") != std::string::npos)
                {
                    // busy waiting
                    apiBuffer.push_back(pCLApiInfo);
                }

                break;

            // Queue
            case CL_FUNC_TYPE_clCreateCommandQueue:
            case CL_FUNC_TYPE_clCreateCommandQueueWithProperties:
            {
                CommandQueueMap::iterator it = m_cmdQueueMap.find(pCLApiInfo->m_strRet);

                if (it != m_cmdQueueMap.end())
                {
                    // pointer reuse? shouldn't happen
                    // delete the old queue.
                    CLCommandQueue* pQueue = it->second;
                    m_cmdQueueMap.erase(it);

                    if (pQueue)
                    {
                        //GenerateMessage(pQueue->m_queue);
                        delete pQueue;
                    }
                }

                m_cmdQueueMap[pCLApiInfo->m_strRet] = new CLCommandQueue(pCLApiInfo->m_strRet);
            }
            break;

            case CL_FUNC_TYPE_clReleaseCommandQueue:
            {
                CommandQueueMap::iterator it = m_cmdQueueMap.find(pCLApiInfo->m_ArgList);

                if (it != m_cmdQueueMap.end())
                {
                    if (it->second->Release())
                    {
                        // command queue released
                        CLCommandQueue* pQueue = it->second;
                        m_cmdQueueMap.erase(it);

                        if (pQueue)
                        {
                            //GenerateMessage(pQueue->m_queue);
                            delete pQueue;
                        }

                    }
                }
                else
                {
                    // Common for partial atp file
                    Log(logERROR, "Unknown command queue handle.\n");
                }
            }
            break;

            case CL_FUNC_TYPE_clRetainCommandQueue:
            {
                CommandQueueMap::iterator it = m_cmdQueueMap.find(pCLApiInfo->m_ArgList);

                if (it != m_cmdQueueMap.end())
                {
                    it->second->Retain();
                }
                else
                {
                    // Common for partial atp file
                    Log(logERROR, "Unknown command queue handle.\n");
                }
            }
            break;

            // Mem read, Map/Unmap

            default:

                if (IsEnqueueCmd(pCLApiInfo))
                {
                    CLEnqueueAPI* pEnqAPI = dynamic_cast<CLEnqueueAPI*>(pCLApiInfo);
                    SpAssertRet(nullptr != pEnqAPI);

                    CommandQueueMap::iterator it = m_cmdQueueMap.find(pEnqAPI->m_strCmdQHandle);
                    CLDependencyNode* pPreNode = NULL;
                    CLDependencyNode* pNewNode = new(nothrow)CLDependencyNode(pEnqAPI);
                    CHECK_NULL_RET(pNewNode)

                    if (it != m_cmdQueueMap.end())
                    {
                        if (it->second->m_queue.size() > 0)
                        {
                            pPreNode = it->second->m_queue.back();
                        }

                        it->second->m_queue.push_back(pNewNode);
                    }
                    else
                    {
                        Log(logERROR, "Unknown command queue handle.\n");
                    }

                    if (pPreNode != NULL)
                    {
                        // Add implicit dependency, look at previous enqueue command
                        if (pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueReadBuffer &&
                            pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueReadImage &&
                            pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueReadBufferRect &&
                            pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueMapBuffer &&
                            pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueMapImage &&
                            pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueUnmapMemObject &&
                            pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueReleaseGLObjects &&        //GL/DX objects need to be released before they can be used by OpenGL/DX.
                            pPreNode->m_pObj->m_uiAPIID != CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR)
                        {
                            SP_TODO("If EnqueueRead* is blocking, this is an implicit dependency. But performance impact not huge.")
                            SP_TODO("Add clEnqueueReleaseD3D11ObjectsKHR when it became available");

                            // A sync may be required between CPU read and GPU command,
                            // else, add dependency to previous node in the command queue
                            CLDependencyEdge* pImplicitEdge = new(nothrow)CLDependencyEdge(pPreNode->m_pObj, ET_Implicit);
                            CHECK_NULL_RET(pImplicitEdge)
                            pNewNode->m_dependencyList.push_back(pImplicitEdge);
                        }
                    }

                    // else, this is the first command in the queue, no implicit dependency.

                    for (list<CLAPIInfo*>::iterator iter = apiBuffer.begin(); iter != apiBuffer.end(); ++iter)
                    {
                        // Add explicit dependency from apiBuffer
                        CLDependencyEdge* pExplicitEdge = new(nothrow)CLDependencyEdge(*iter, ET_Explicit);
                        CHECK_NULL_RET(pExplicitEdge)

                        switch ((*iter)->m_uiAPIID)
                        {
                            case CL_FUNC_TYPE_clFinish:
                            case CL_FUNC_TYPE_clGetEventInfo:
                            {
                                for (EdgeList::iterator edgeIter = pNewNode->m_dependencyList.begin(); edgeIter != pNewNode->m_dependencyList.end(); ++edgeIter)
                                {
                                    CLDependencyEdge* pEdge = *edgeIter;

                                    if (pPreNode != NULL)
                                    {
                                        // As long as an existing dependency on previous command exist, it's redundant dependency
                                        if (pEdge->m_pObj == pPreNode->m_pObj)
                                        {
                                            // generate message
                                            GenerateMessage(pExplicitEdge->m_pObj);
                                            break;
                                        }
                                    }
                                }
                            }
                            break;

                            case CL_FUNC_TYPE_clWaitForEvents:
                            {
                                vector<string> output;
                                StringUtils::Split(output, (*iter)->m_ArgList, string(";"));
                                SpAssert(output.size() == NUM_ARG_CL_WAIT_FOR_EVENTS);

                                if (output.size() == NUM_ARG_CL_WAIT_FOR_EVENTS)
                                {
                                    if (output[1] == "NULL" || output[1].length() <= 2)
                                    {
                                        continue;
                                    }

                                    string strNumEvents = output[0];
                                    size_t numEvents = 0;
                                    bool ret = StringUtils::Parse(strNumEvents, numEvents);
                                    SpAssert(ret);

                                    if (!ret || numEvents == 0)
                                    {
                                        continue;
                                    }

                                    // strip out square brackets
                                    string strEventHandleList = output[1].substr(1, output[1].length() - 2);
                                    vector<string> eventHandles;
                                    StringUtils::Split(eventHandles, strEventHandleList, string(","));

                                    SpAssert(eventHandles.size() == numEvents);

                                    if (eventHandles.size() == numEvents)
                                    {
                                        // Check each event on the wait list, whether any of the event belongs to existing dependent enqueue command
                                        for (size_t i = 0; i < eventHandles.size(); ++i)
                                        {
                                            for (EdgeList::iterator edgeIter = pNewNode->m_dependencyList.begin(); edgeIter != pNewNode->m_dependencyList.end(); ++edgeIter)
                                            {
                                                CLDependencyEdge* pEdge = *edgeIter;

                                                if (IsEnqueueCmd(pEdge->m_pObj))
                                                {
                                                    string strEvent = CLObjRefTracker::GetEventHandle(pEdge->m_pObj);

                                                    if (!strEvent.empty() && strEvent != "NULL")
                                                    {
                                                        if (strEvent == eventHandles[i])
                                                        {
                                                            GenerateMessage(pExplicitEdge->m_pObj);
                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        Log(logWARNING, "clWaitForEvents: Inconsistent number of events.\n");
                                    }
                                }
                            }
                            break;
                        }

                        pNewNode->m_dependencyList.push_back(pExplicitEdge);
                    }

                    apiBuffer.clear();
                }

                break;
        }
    }
}

void CLSyncAnalyzer::EndAnalyze()
{
}

void CLSyncAnalyzer::GenerateMessage(CLAPIInfo* pAPIInfo)
{
    stringstream ss;
    ss << "Redundant synchronization detected. ";
    ss << "Synchronization API = " <<  pAPIInfo->m_strName << endl;
    APIAnalyzerMessage msg;
    msg.type = MSGTYPE_BestPractices;
    msg.uiSeqID = pAPIInfo->m_uiSeqID;
    msg.uiDisplaySeqID = pAPIInfo->m_uiDisplaySeqID;
    msg.bHasDisplayableSeqId = pAPIInfo->m_bHasDisplayableSeqId;
    msg.uiTID = pAPIInfo->m_tid;
    msg.strMsg = ss.str();
    m_msgList.push_back(msg);
}
