//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class generates an API summary page.
//==============================================================================

#ifndef _API_SUMMARIZER_H_
#define _API_SUMMARIZER_H_

#include <map>
#include <string>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "../CLTraceAgent/CLAPIInfo.h"
#include "../HSAFdnTrace/HSAAPIInfo.h"
#include "../Common/IParserListener.h"
#include "../Common/OSUtils.h"

//------------------------------------------------------------------------------------
/// API Summary Items struct
//------------------------------------------------------------------------------------
struct APISummaryItems
{
    std::string strName;         ///< API name
    unsigned int uiNumCalls;     ///< Number of Calls
    unsigned int uiNumErrors;    ///< Number of Errors
    ULONGLONG ullTotalTime;      ///< Total time
    ULONGLONG ullMax;            ///< Max time
    ULONGLONG ullMin;            ///< Min time
    ULONGLONG ullAve;            ///< Ave time
    unsigned int uiMaxCallIndex; ///< Call index of instance with max time
    unsigned int uiMinCallIndex; ///< Call index of instance with min time
    osThreadId maxTid;             ///< Thread id of instance with max time
    osThreadId minTid;             ///< Thread id of instance with min time

    /// Constructor
    APISummaryItems()
    {
        strName.clear();
        uiNumCalls = uiNumErrors = 0;
        ullTotalTime = 0;
        ullMax = 0;
        ullMin = (ULONGLONG) - 1; // init to maximum ulonglong
        ullAve = 0;
        uiMaxCallIndex = 0;
        uiMinCallIndex = 0;
        maxTid = 0;
        minTid = 0;
    }

    /// Copy constructor
    /// \param obj object
    APISummaryItems(const APISummaryItems& obj)
    {
        strName = obj.strName;
        uiNumCalls = obj.uiNumCalls;
        uiNumErrors = obj.uiNumErrors;
        ullTotalTime = obj.ullTotalTime;
        ullMax = obj.ullMax;
        ullMin = obj.ullMin;
        ullAve = obj.ullAve;
        uiMaxCallIndex = obj.uiMaxCallIndex;
        uiMinCallIndex = obj.uiMinCallIndex;
        maxTid = obj.maxTid;
        minTid = obj.minTid;
    }

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const APISummaryItems& operator = (const APISummaryItems& obj)
    {
        if (this != &obj)
        {
            strName = obj.strName;
            uiNumCalls = obj.uiNumCalls;
            uiNumErrors = obj.uiNumErrors;
            ullTotalTime = obj.ullTotalTime;
            ullMax = obj.ullMax;
            ullMin = obj.ullMin;
            ullAve = obj.ullAve;
            uiMaxCallIndex = obj.uiMaxCallIndex;
            uiMinCallIndex = obj.uiMinCallIndex;
            maxTid = obj.maxTid;
            minTid = obj.minTid;
        }

        return *this;
    }
};

typedef std::map< std::string, APISummaryItems > APICountMap;
typedef std::pair< std::string, APISummaryItems > APICountMapPair;

//------------------------------------------------------------------------------------
/// OpenCL API Summarizer
//------------------------------------------------------------------------------------
template<class T>
class APISummarizer :
    public IParserListener<T>
{
public:
    /// Constructor
    APISummarizer(void);

    /// Destructor
    ~APISummarizer(void);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(T* pAPIInfo, bool& stopParsing);

    /// Debug function
    void Debug();

    /// Generate HTML table from statistic data and write to std::ostream
    /// \param sout output stream
    void GenerateHTMLTable(std::ostream& sout);

    /// Generate simple HTML page
    /// \param szFileName file name
    /// \return true if the page was generated, false otherwise
    bool GenerateHTMLPage(const char* szFileName);

protected:
    APICountMap m_APICountMap;       /// API map key = API Name
private:
    /// Copy constructor
    /// \param obj object
    APISummarizer(const APISummarizer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const APISummarizer& operator = (const APISummarizer& obj);

    ULONGLONG m_ullTotalTime;  ///< Cumulative total time of all API calls (used to calculate % time of each API call)
};

typedef class APISummarizer<CLAPIInfo> CLAPISummarizer;

typedef class APISummarizer<HSAAPIInfo> HSAAPISummarizer;

#endif //_API_SUMMARIZER_H_
