//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class tracks opencl object create, retain and release and generates warning messages
//==============================================================================

#include <sstream>
#include <vector>
#include "CLObjRefTracker.h"
#include "../CLCommon/CLFunctionEnumDefs.h"
#include "../Common/Logger.h"
#include "../Common/StringUtils.h"

//#define DEBUG_REF_TRACKER
using namespace GPULogger;

CLObjRefTracker::CLObjRefTracker(CLAPIAnalyzerManager* p) : CLAPIAnalyzer(p)
{
    m_strName = "RefTracker";

    m_bRequireAPIFlattening = true;
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
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueReleaseGLObjects);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMapBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueMapImage);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueSVMFree);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueSVMMemcpy);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueSVMMemFill);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueSVMMap);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clEnqueueSVMUnmap);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateSubBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateImage2D);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateImage3D);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateCommandQueue);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateCommandQueueWithProperties);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateContext);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateContextFromType);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateProgramWithSource);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateProgramWithBinary);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateSampler);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateSamplerWithProperties);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateUserEvent);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateFromGLBuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateFromGLTexture2D);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateFromGLTexture3D);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateFromGLRenderbuffer);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateEventFromGLsyncKHR);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateSubDevicesEXT);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateSubDevices);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateFromD3D10BufferKHR);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreateKernelsInProgram);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainContext);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainCommandQueue);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainMemObject);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainSampler);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainProgram);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainEvent);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainDeviceEXT);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clRetainDevice);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseContext);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseCommandQueue);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseMemObject);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseSampler);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseProgram);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseKernel);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseEvent);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseDeviceEXT);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clReleaseDevice);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clSVMAlloc);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clSVMFree);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clSVMAllocAMD);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clSVMFreeAMD);
    m_dependentAPIs.insert(CL_FUNC_TYPE_clCreatePipe);
}

CLObjRefTracker::~CLObjRefTracker(void)
{
    Clear();
}

void CLObjRefTracker::Clear()
{
    for (APITraceMap::iterator it = m_objRefHistoryMap.begin() ; it != m_objRefHistoryMap.end(); it++)
    {
        if (it->second != NULL)
        {
            delete it->second;
        }
    }

    m_objRefHistoryMap.clear();
    m_msgList.clear();
    m_bEndAnalyze = false;
}

std::string CLObjRefTracker::GetEventHandle(CLAPIInfo* pAPIInfo)
{
    switch (pAPIInfo->m_uiAPIID)
    {
        case CL_FUNC_TYPE_clEnqueueReadBuffer:
        case CL_FUNC_TYPE_clEnqueueReadBufferRect:
        case CL_FUNC_TYPE_clEnqueueWriteBuffer:
        case CL_FUNC_TYPE_clEnqueueWriteBufferRect:
        case CL_FUNC_TYPE_clEnqueueCopyBuffer:
        case CL_FUNC_TYPE_clEnqueueCopyBufferRect:
        case CL_FUNC_TYPE_clEnqueueReadImage:
        case CL_FUNC_TYPE_clEnqueueWriteImage:
        case CL_FUNC_TYPE_clEnqueueCopyImage:
        case CL_FUNC_TYPE_clEnqueueCopyImageToBuffer:
        case CL_FUNC_TYPE_clEnqueueCopyBufferToImage:
        case CL_FUNC_TYPE_clEnqueueUnmapMemObject:
        case CL_FUNC_TYPE_clEnqueueNDRangeKernel:
        case CL_FUNC_TYPE_clEnqueueTask:
        case CL_FUNC_TYPE_clEnqueueNativeKernel:
        case CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR:
        case CL_FUNC_TYPE_clEnqueueAcquireGLObjects:
        case CL_FUNC_TYPE_clEnqueueMarker:
        case CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR:
        case CL_FUNC_TYPE_clEnqueueReleaseGLObjects:
        case CL_FUNC_TYPE_clEnqueueSVMFree:
        case CL_FUNC_TYPE_clEnqueueSVMMemcpy:
        case CL_FUNC_TYPE_clEnqueueSVMMemFill:
        case CL_FUNC_TYPE_clEnqueueSVMMap:
        case CL_FUNC_TYPE_clEnqueueSVMUnmap:
        {
            // cl_event object is the last arg
            std::string strHandle;
            std::vector<std::string> strs;
            StringUtils::Split(strs, pAPIInfo->m_ArgList, std::string(";"), true, true);

            if (strs.size() > 1)
            {
                strHandle = strs[strs.size() - 1];

                if (strHandle == "NULL")
                {
                    return strHandle;
                }

                size_t handleStrLen = strHandle.length();

                if (handleStrLen > 2)
                {
                    return strHandle.substr(1, handleStrLen - 2);
                }
            }
            else
            {
                Log(logERROR, "clEqueue: incorrect arg list - %p\n", pAPIInfo->m_ArgList.c_str());
            }

            break;
        }

        case CL_FUNC_TYPE_clEnqueueMapBuffer:
        case CL_FUNC_TYPE_clEnqueueMapImage:
        {
            // cl_event object is the 2nd last arg
            std::string strHandle;
            std::vector<std::string> strs;
            StringUtils::Split(strs, pAPIInfo->m_ArgList, std::string(";"), true, true);

            if (strs.size() > 2)
            {
                strHandle = strs[strs.size() - 2];

                if (strHandle == "NULL")
                {
                    return strHandle;
                }

                size_t handleStrLen = strHandle.length();

                if (handleStrLen > 2)
                {
                    return strHandle.substr(1, handleStrLen - 2);
                }
            }
            else
            {
                Log(logERROR, "clEqueue: incorrect arg list - %p\n", pAPIInfo->m_ArgList.c_str());
            }

            break;
        }

        default:
            break;
    }

    std::string ret;
    ret.clear();
    return ret;
}

void CLObjRefTracker::Analyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
    return;
}

void CLObjRefTracker::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    CLAPIInfo* pCLApiInfo = dynamic_cast<CLAPIInfo*>(pAPIInfo);

    if (nullptr != pCLApiInfo)
    {
        switch (pCLApiInfo->m_uiAPIID)
        {
            case CL_FUNC_TYPE_clCreateBuffer:
            case CL_FUNC_TYPE_clCreateSubBuffer:
            case CL_FUNC_TYPE_clCreateImage2D:
            case CL_FUNC_TYPE_clCreateImage3D:
            case CL_FUNC_TYPE_clCreateCommandQueue:
            case CL_FUNC_TYPE_clCreateCommandQueueWithProperties:
            case CL_FUNC_TYPE_clCreateContext:
            case CL_FUNC_TYPE_clCreateContextFromType:
            case CL_FUNC_TYPE_clCreateProgramWithSource:
            case CL_FUNC_TYPE_clCreateProgramWithBinary:
            case CL_FUNC_TYPE_clCreateKernel:
            case CL_FUNC_TYPE_clCreateSampler:
            case CL_FUNC_TYPE_clCreateSamplerWithProperties:
            case CL_FUNC_TYPE_clCreateUserEvent:
            case CL_FUNC_TYPE_clCreateFromGLBuffer:
            case CL_FUNC_TYPE_clCreateFromGLTexture2D:
            case CL_FUNC_TYPE_clCreateFromGLTexture3D:
            case CL_FUNC_TYPE_clCreateFromGLRenderbuffer:
            case CL_FUNC_TYPE_clCreateEventFromGLsyncKHR:
            case CL_FUNC_TYPE_clCreateFromD3D10BufferKHR:
            case CL_FUNC_TYPE_clCreateFromD3D10Texture2DKHR:
            case CL_FUNC_TYPE_clCreateFromD3D10Texture3DKHR:
            case CL_FUNC_TYPE_clCreatePipe:
            case CL_FUNC_TYPE_clSVMAlloc:
            case CL_FUNC_TYPE_clSVMAllocAMD:
            {
                std::string strHandle = pCLApiInfo->m_strRet;

                if (strHandle.empty())
                {
                    Log(logERROR, "Incorrect return value - %s\n", pCLApiInfo->m_strRet.c_str());
                    return;
                }

                AddCLCreate(strHandle, pCLApiInfo);
                break;
            }

            case CL_FUNC_TYPE_clCreateSubDevicesEXT:
            case CL_FUNC_TYPE_clCreateSubDevices:

                if (pCLApiInfo->m_strRet == "CL_SUCCESS")
                {
                    std::vector<std::string> output;
                    StringUtils::Split(output, pCLApiInfo->m_ArgList, std::string(";"));
                    SpAssert(output.size() == NUM_ARG_CL_CREATE_SUB_DEVICES_EXT);

                    if (output.size() == NUM_ARG_CL_CREATE_SUB_DEVICES_EXT)
                    {
                        if (output[3] == "NULL" || output[3].length() <= 2)
                        {
                            return;
                        }

                        std::string strNumDevices = output[2];
                        size_t numDevices;
                        bool ret = StringUtils::Parse(strNumDevices, numDevices);
                        SpAssert(ret);

                        if (!ret || numDevices == 0)
                        {
                            return;
                        }

                        std::string strDeviceHandleList = output[3].substr(1, output[3].length() - 2);
                        std::vector<std::string> devicesHandles;
                        StringUtils::Split(devicesHandles, strDeviceHandleList, std::string(","));
                        // User could pass in bigger number than actual number of devices that are created
                        SpAssert(devicesHandles.size() <= numDevices);

                        if (devicesHandles.size() <= numDevices)
                        {
                            for (size_t i = 0; i < devicesHandles.size(); ++i)
                            {
                                AddCLCreate(devicesHandles[i], pCLApiInfo);
                            }
                        }
                    }
                }

                break;

            case CL_FUNC_TYPE_clCreateKernelsInProgram:

                if (pCLApiInfo->m_strRet == "CL_SUCCESS")
                {
                    std::vector<std::string> output;
                    StringUtils::Split(output, pCLApiInfo->m_ArgList, std::string(";"));
                    SpAssert(output.size() == NUM_ARG_CL_CREATE_KERNELS_IN_PROGRAM);

                    if (output.size() == NUM_ARG_CL_CREATE_KERNELS_IN_PROGRAM)
                    {
                        if (output[2] == "NULL" || output[2].length() <= 2)
                        {
                            return;
                        }

                        std::string strNumKernels = output[1];
                        size_t numKernels;
                        bool ret = StringUtils::Parse(strNumKernels, numKernels);
                        SpAssert(ret);

                        if (!ret || numKernels == 0)
                        {
                            return;
                        }

                        // strip out square brackets
                        std::string strKernelHandleList = output[2].substr(1, output[2].length() - 2);
                        std::vector<std::string> kernelHandles;
                        StringUtils::Split(kernelHandles, strKernelHandleList, std::string(","));
                        // User could pass in bigger number than actual number of kernels that are created
                        SpAssert(kernelHandles.size() <= numKernels);

                        if (kernelHandles.size() <= numKernels)
                        {
                            for (size_t i = 0; i < kernelHandles.size(); ++i)
                            {
                                AddCLCreate(kernelHandles[i], pCLApiInfo);
                            }
                        }
                    }
                }

                break;

            case  CL_FUNC_TYPE_clRetainContext:
            case  CL_FUNC_TYPE_clRetainCommandQueue:
            case  CL_FUNC_TYPE_clRetainMemObject:
            case  CL_FUNC_TYPE_clRetainSampler:
            case  CL_FUNC_TYPE_clRetainProgram:
            case  CL_FUNC_TYPE_clRetainKernel:
            case  CL_FUNC_TYPE_clRetainEvent:
            case  CL_FUNC_TYPE_clRetainDeviceEXT:
            case  CL_FUNC_TYPE_clRetainDevice:
            {
                std::string strHandle = pCLApiInfo->m_ArgList;

                if (strHandle.empty())
                {
                    Log(logERROR, "Incorrect return value - %s\n", pCLApiInfo->m_strRet.c_str());
                    return;
                }

                UpdateCLRefCounter(strHandle, pCLApiInfo, API_OBJECT_ACTION_Retain, 1);
                break;
            }

            case CL_FUNC_TYPE_clReleaseContext:
            case CL_FUNC_TYPE_clReleaseCommandQueue:
            case CL_FUNC_TYPE_clReleaseMemObject:
            case CL_FUNC_TYPE_clReleaseSampler:
            case CL_FUNC_TYPE_clReleaseProgram:
            case CL_FUNC_TYPE_clReleaseKernel:
            case CL_FUNC_TYPE_clReleaseEvent:
            case CL_FUNC_TYPE_clReleaseDeviceEXT:
            case CL_FUNC_TYPE_clReleaseDevice:
            {
                std::string strHandle = pCLApiInfo->m_ArgList;

                if (strHandle.empty())
                {
                    Log(logERROR, "Incorrect return value - %s\n", pCLApiInfo->m_strRet.c_str());
                    return;
                }

                UpdateCLRefCounter(strHandle, pCLApiInfo, API_OBJECT_ACTION_Release, -1);
                break;
            }

            case CL_FUNC_TYPE_clSVMFree:
            case CL_FUNC_TYPE_clSVMFreeAMD:
            {
                // svm pointer is the last arg
                std::string strHandle;
                std::vector<std::string> strs;
                StringUtils::Split(strs, pCLApiInfo->m_ArgList, std::string(";"), true, true);

                if (strs.size() > 1)
                {
                    strHandle = strs[strs.size() - 1];

                    if (strHandle == "NULL")
                    {
                        return;
                    }

                    UpdateCLRefCounter(strHandle, pCLApiInfo, API_OBJECT_ACTION_Release, -1);
                }
                else
                {
                    Log(logERROR, "clSvmFree: incorrect arg list - %p\n", pCLApiInfo->m_ArgList.c_str());
                }

                break;
            }

            case CL_FUNC_TYPE_clEnqueueReadBuffer:
            case CL_FUNC_TYPE_clEnqueueReadBufferRect:
            case CL_FUNC_TYPE_clEnqueueWriteBuffer:
            case CL_FUNC_TYPE_clEnqueueWriteBufferRect:
            case CL_FUNC_TYPE_clEnqueueCopyBuffer:
            case CL_FUNC_TYPE_clEnqueueCopyBufferRect:
            case CL_FUNC_TYPE_clEnqueueReadImage:
            case CL_FUNC_TYPE_clEnqueueWriteImage:
            case CL_FUNC_TYPE_clEnqueueCopyImage:
            case CL_FUNC_TYPE_clEnqueueCopyImageToBuffer:
            case CL_FUNC_TYPE_clEnqueueCopyBufferToImage:
            case CL_FUNC_TYPE_clEnqueueUnmapMemObject:
            case CL_FUNC_TYPE_clEnqueueNDRangeKernel:
            case CL_FUNC_TYPE_clEnqueueTask:
            case CL_FUNC_TYPE_clEnqueueNativeKernel:
            case CL_FUNC_TYPE_clEnqueueAcquireD3D10ObjectsKHR:
            case CL_FUNC_TYPE_clEnqueueAcquireGLObjects:
            case CL_FUNC_TYPE_clEnqueueMarker:
            case CL_FUNC_TYPE_clEnqueueReleaseD3D10ObjectsKHR:
            case CL_FUNC_TYPE_clEnqueueReleaseGLObjects:
            case CL_FUNC_TYPE_clEnqueueMapBuffer:
            case CL_FUNC_TYPE_clEnqueueMapImage:
            case CL_FUNC_TYPE_clEnqueueSVMFree:
            case CL_FUNC_TYPE_clEnqueueSVMMemcpy:
            case CL_FUNC_TYPE_clEnqueueSVMMemFill:
            case CL_FUNC_TYPE_clEnqueueSVMMap:
            case CL_FUNC_TYPE_clEnqueueSVMUnmap:
            {
                std::string eventHandle = GetEventHandle(pCLApiInfo);

                if (!eventHandle.empty() && eventHandle != "NULL")
                {
                    AddCLCreate(eventHandle, pCLApiInfo);
                }
            }
            break;

            default:
                break;
        }
    }
}

void CLObjRefTracker::UpdateCLRefCounter(std::string strHandle, CLAPIInfo* pAPIInfo, APIObjectAction action, int toBeAdd)
{
    APIObjectHistory his;
    his.m_action = action;
    his.m_pAPIInfoObj = pAPIInfo;
    APITraceMap::iterator it = m_objRefHistoryMap.find(strHandle);

    if (it != m_objRefHistoryMap.end())
    {
        APIObjHistoryList* list = it->second;
        int iRef = list->back().m_iCurrentRef;
        his.m_iCurrentRef = iRef + toBeAdd;

        if (his.m_iCurrentRef < 0)
        {
            Log(logERROR, "clRelease: ocl object ref < 0\n", strHandle.c_str());
        }

        list->push_back(his);
    }
    else
    {
        Log(logERROR, "clRelease/clRetain: ocl object handle invalid - %p\n", strHandle.c_str());
    }
}

void CLObjRefTracker::AddCLCreate(std::string strHandle, CLAPIInfo* pAPIInfo)
{
    APIObjectHistory his;
    his.m_action = API_OBJECT_ACTION_Create;
    his.m_pAPIInfoObj = pAPIInfo;
    his.m_iCurrentRef = 1;
    APITraceMap::iterator it = m_objRefHistoryMap.find(strHandle);

    if (it != m_objRefHistoryMap.end())
    {
        APIObjHistoryList* list = it->second;

        if (list->back().m_iCurrentRef != 0)
        {
            Log(logERROR, "clCreate: ocl object handle conflict - %p\n", strHandle.c_str());
        }
        else
        {
            // pointer reuse
            // just append to it
            list->push_back(his);
        }
    }
    else
    {
        APIObjHistoryList* list = new APIObjHistoryList();
        list->push_back(his);
        m_objRefHistoryMap.insert(std::pair<std::string, APIObjHistoryList*>(strHandle, list));
    }
}

std::string CLObjRefTracker::APIObjHistoryListToString(APIObjHistoryList* list)
{
    std::stringstream ss;

    for (APIObjHistoryList::iterator it = list->begin(); it != list->end(); it++)
    {
        CLAPIInfo* pInfo = dynamic_cast<CLAPIInfo*>(it->m_pAPIInfoObj);

        if (nullptr != pInfo)
        {
            ss << "[ ThreadID: " << pInfo->m_tid << ", SeqID: " << pInfo->m_uiSeqID << " ]" << pInfo->m_strName << " - Ref = " << it->m_iCurrentRef << std::endl;
        }
    }

    return ss.str();
}

void CLObjRefTracker::EndAnalyze()
{
    if (m_bEndAnalyze)
    {
        return;
    }

    for (APITraceMap::iterator it = m_objRefHistoryMap.begin() ; it != m_objRefHistoryMap.end(); it++)
    {
#ifdef DEBUG_REF_TRACKER
        m_msgList.push_back(APIObjHistoryListToString(it->second));
#endif

        if (it->second->back().m_iCurrentRef != 0)
        {
            CLAPIInfo* pInfo = dynamic_cast<CLAPIInfo*>(it->second->front().m_pAPIInfoObj);

            if (nullptr != pInfo)
            {
                std::stringstream ss;
                ss << "Memory leak detected [Ref = " << it->second->back().m_iCurrentRef << ", Handle = " << it->first << "]: ";
                ss << "Object created by " << pInfo->m_strName << std::endl;
                APIAnalyzerMessage msg;
                msg.type = MSGTYPE_Warning;
                msg.uiSeqID = pInfo->m_uiSeqID;
                msg.uiDisplaySeqID = pInfo->m_uiDisplaySeqID;
                msg.bHasDisplayableSeqId = pInfo->m_bHasDisplayableSeqId;
                msg.uiTID = pInfo->m_tid;
                msg.strMsg = ss.str();
                m_msgList.push_back(msg);
            }
        }
    }

    m_bEndAnalyze = true;
}
