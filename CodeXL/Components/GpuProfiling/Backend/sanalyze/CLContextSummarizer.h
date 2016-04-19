//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides a CL Context Summarizer
//==============================================================================

#ifndef _CL_CONTEXT_SUMMARIZER_H_
#define _CL_CONTEXT_SUMMARIZER_H_

#include <map>
#include <vector>
#include "../CLTraceAgent/CLAPIInfo.h"
#include "../Common/IParserListener.h"
#include "../Common/OSUtils.h"
#include "CLKernelSummarizer.h"

//------------------------------------------------------------------------------------
/// Context Summary table header
//------------------------------------------------------------------------------------
class ContextSummaryItems
{
public:
    unsigned int uiContextID;           ///< Context ID

    unsigned int uiNumMemOp;            ///< Number of memory transfers
    ULONGLONG ullTotalMemDuration;      ///< Total time on data transfer

    KernelSumMap KernelMap;          ///< Per context, per device kernel summary

    //unsigned int uiDeviceID;

    unsigned int uiNumCopy;             ///< Number of EnqueueCopy*
    unsigned int uiNumMap;              ///< Number of EnqueueMap*
    unsigned int uiNumWrite;            ///< Number of EnqueueWrite*
    unsigned int uiNumRead;             ///< Number of EnqueueRead*

    unsigned int uiByteCopy;            ///< size of data transfer from EnqueueCopy*
    unsigned int uiByteMap;             ///< size of data transfer from EnqueueMap*
    unsigned int uiByteWrite;           ///< size of data transfer from EnqueueWrite*
    unsigned int uiByteRead;            ///< size of data transfer from EnqueueRead*

    ULONGLONG ullDurationCopy;          ///< Ttoal duration for EnqueueCopy*
    ULONGLONG ullDurationMap;           ///< Ttoal duration for EnqueueMap*
    ULONGLONG ullDurationWrite;         ///< Ttoal duration for EnqueueWrite*
    ULONGLONG ullDurationRead;          ///< Ttoal duration for EnqueueRead*

    unsigned int uiNumBuffer;           ///< Number of Buffers created on this context
    unsigned int uiNumImage;            ///< Number of Images created on this context

    /// Constructor
    ContextSummaryItems()
    {
        uiNumCopy = uiNumMap = uiNumWrite = uiNumRead = 0;
        uiByteCopy = uiByteMap = uiByteWrite = uiByteRead = 0;
        ullDurationCopy = ullDurationMap = ullDurationWrite = ullDurationRead = 0;
        uiContextID = (unsigned int) - 1;
        uiNumMemOp = 0;
        ullTotalMemDuration = 0;
        uiNumBuffer = uiNumImage = 0;
    }

    /// Copy constructor
    /// \param obj object
    ContextSummaryItems(const ContextSummaryItems& obj)
    {
        uiNumCopy      =  obj.uiNumCopy;
        uiNumMap       =  obj.uiNumMap;
        uiNumWrite     =  obj.uiNumWrite;
        uiNumRead      =  obj.uiNumRead;

        uiByteCopy     =  obj.uiByteCopy;
        uiByteMap      =  obj.uiByteMap;
        uiByteWrite    =  obj.uiByteWrite;
        uiByteRead     =  obj.uiByteRead;

        ullDurationCopy =  obj.ullDurationCopy;
        ullDurationMap  =  obj.ullDurationMap;
        ullDurationWrite =  obj.ullDurationWrite;
        ullDurationRead =  obj.ullDurationRead;

        uiContextID    =  obj.uiContextID;

        uiNumMemOp = obj.uiNumMemOp;

        ullTotalMemDuration = obj.ullTotalMemDuration;

        // Use std::map's assignment operator, shallow copy is enough here
        KernelMap = obj.KernelMap;

        uiNumBuffer = obj.uiNumBuffer;
        uiNumImage = obj.uiNumImage;
    }

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const ContextSummaryItems& operator=(const ContextSummaryItems& obj)
    {
        if (this != &obj)
        {
            uiNumCopy      =  obj.uiNumCopy;
            uiNumMap       =  obj.uiNumMap;
            uiNumWrite     =  obj.uiNumWrite;
            uiNumRead      =  obj.uiNumRead;

            uiByteCopy     =  obj.uiByteCopy;
            uiByteMap      =  obj.uiByteMap;
            uiByteWrite    =  obj.uiByteWrite;
            uiByteRead     =  obj.uiByteRead;

            ullDurationCopy =  obj.ullDurationCopy;
            ullDurationMap  =  obj.ullDurationMap;
            ullDurationWrite =  obj.ullDurationWrite;
            ullDurationRead =  obj.ullDurationRead;

            uiContextID    =  obj.uiContextID;

            uiNumMemOp = obj.uiNumMemOp;

            ullTotalMemDuration = obj.ullTotalMemDuration;

            // Use std::map's assignment operator, shallow copy is enough here
            KernelMap = obj.KernelMap;

            uiNumBuffer = obj.uiNumBuffer;
            uiNumImage = obj.uiNumImage;
        }

        return *this;
    }

    /// Plus operator
    /// \param obj object
    /// \return ref to itself
    const ContextSummaryItems& operator+=(const ContextSummaryItems& obj)
    {
        uiNumCopy      +=  obj.uiNumCopy;
        uiNumMap       +=  obj.uiNumMap;
        uiNumWrite     +=  obj.uiNumWrite;
        uiNumRead      +=  obj.uiNumRead;

        uiByteCopy     +=  obj.uiByteCopy;
        uiByteMap      +=  obj.uiByteMap;
        uiByteWrite    +=  obj.uiByteWrite;
        uiByteRead     +=  obj.uiByteRead;

        ullDurationCopy +=  obj.ullDurationCopy;
        ullDurationMap  +=  obj.ullDurationMap;
        ullDurationWrite +=  obj.ullDurationWrite;
        ullDurationRead +=  obj.ullDurationRead;

        uiNumMemOp += obj.uiNumMemOp;
        ullTotalMemDuration += obj.ullTotalMemDuration;

        // Search device type, if existed, update it, if not, copy and add to kernel map.
        for (KernelSumMap::const_iterator it = obj.KernelMap.begin(); it != obj.KernelMap.end(); it++)
        {
            KernelSumMap::iterator thisIt = KernelMap.find(it->first);

            if (thisIt != KernelMap.end())
            {
                thisIt->second.ullTotalTime += it->second.ullTotalTime;
                thisIt->second.uiNumCalls += it->second.uiNumCalls;
            }
            else
            {
                KernelSummaryItems kitem;
                kitem = it->second;
                KernelMap.insert(std::pair< std::string, KernelSummaryItems>(it->first, kitem));
            }
        }

        uiNumBuffer += obj.uiNumBuffer;
        uiNumImage += obj.uiNumImage;

        return *this;
    }
};

//------------------------------------------------------------------------------------
/// Temporary CLObject counter per context
/// When we handle clCreate* APIs, we don't know what context ID but context handle the object was created on.
/// m_tmpCLObjCounter maintains a map from context handle to number of cl objects that were created on the context.
/// Context handle could be reused, therefore, we flush tmp counter everytime we handle an enqueueCmd
//------------------------------------------------------------------------------------
struct CLObjectCounter
{
    unsigned int uiImageCount;    /// Image count
    unsigned int uiBufferCount;   /// Buffer count

    /// Constructor
    CLObjectCounter()
    {
        uiBufferCount = uiImageCount = 0;
    }
};

typedef std::map<unsigned int, ContextSummaryItems> ContextSumMap;


//------------------------------------------------------------------------------------
/// OpenCL Context Summarizer
//------------------------------------------------------------------------------------
class CLContextSummarizer
    : public IParserListener<CLAPIInfo>
{
public:
    /// Constructor
    CLContextSummarizer(void);

    /// Destructor
    ~CLContextSummarizer(void);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing);

    /// Generate HTML table from statistic data and write to std::ostream
    /// \param sout output stream
    void GenerateHTMLTable(std::ostream& sout);

    /// Generate simple HTML page
    /// \param szFileName file name
    /// \return true if the page was generated, false otherwise
    bool GenerateHTMLPage(const char* szFileName);

    /// When we handle clCreate* APIs, we don't know what context ID but context handle the object was created on.
    /// m_tmpCLObjCounter maintains a map from context handle to number of cl objects that were created on the context.
    /// Context handle could be reused, therefore, we flush tmp counter everytime we handle an enqueueCmd
    void FlushTmpCounters(std::string& strCntx, ContextSummaryItems* pItems);

protected:
    ContextSumMap m_ContextSumMap;                                 ///< Context summary map (ContextID to ContextSummaryItems )
    std::map< std::string, CLObjectCounter > m_tmpCLObjCounter;    ///< Temp cl object counter: map from context handle string to CLObjectCounter
    std::vector< std::string > m_vecDevices;                       ///< Global Devices (as oppose to devices per context) created, this is used generate table header
private:
    /// Copy constructor
    /// \param obj object
    CLContextSummarizer(const CLContextSummarizer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CLContextSummarizer& operator = (const CLContextSummarizer& obj);
};

#endif //_CL_CONTEXT_SUMMARIZER_H_
