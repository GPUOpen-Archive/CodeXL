//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines APIAnalyzer and APIAnalyzerManager base classes
//==============================================================================

#ifndef _API_ANALYZER_H_
#define _API_ANALYZER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include "../Common/StringUtils.h"
#include "../Common/Logger.h"
#include "../Common/HTMLTable.h"
#include "../Common/Defs.h"
#include "../Common/IParserListener.h"
#include "../Common/APIInfo.h"
#include "AnalyzerHTMLUtils.h"

//------------------------------------------------------------------------------------
/// APIAnalyzer Message Type
//------------------------------------------------------------------------------------
enum APIAnalyzerMessageType
{
    MSGTYPE_Error,        ///< Error message
    MSGTYPE_Warning,      ///< Warning message
    MSGTYPE_BestPractices ///< Best practices message
};

//------------------------------------------------------------------------------------
/// APIAnalyzer Message
//------------------------------------------------------------------------------------
struct APIAnalyzerMessage
{
    unsigned int uiSeqID;        ///< Sequence ID
    unsigned int uiDisplaySeqID; ///< Displayable sequence ID
    bool bHasDisplayableSeqId;   ///< Flag indicating whether or not this API has a displayable sequence id
    unsigned int uiTID;          ///< Thread ID
    std::string strMsg;          ///< Message
    APIAnalyzerMessageType type; ///< Type
};

//------------------------------------------------------------------------------------
/// API Analyzer base class
/// Expectations: T is an API-specific enum of function types
//------------------------------------------------------------------------------------
template<typename T>
class APIAnalyzer
{
public:
    /// Constructor
    APIAnalyzer() :
        m_bEnabled(true),
        m_bEndAnalyze(false),
        m_strName(nullptr),
        m_bRequireAPIFlattening(false)
    {}

    /// Virtual Analyze function
    /// \param pAPIInfo APIInfo object
    virtual void Analyze(APIInfo* pAPIInfo) = 0;

    /// Virtual Generate APIAnalyzerMessage
    virtual void EndAnalyze() = 0;

    /// Callback function for flattened APIs
    /// \param pAPIInfo APIInfo object
    virtual void FlattenedAPIAnalyze(APIInfo* pAPIInfo) = 0;

    /// Virtual destructor
    virtual ~APIAnalyzer() {}

    /// Enable/Disable analyzers according to config file
    /// \param op AnalyzeOps object
    virtual void SetEnable(const AnalyzeOps& op)
    {
        auto it = op.analyzerMap.find(m_strName);

        if (it != op.analyzerMap.end())
        {
            m_bEnabled = it->second;
        }
    }

    /// Get messages
    /// \param[out] output array
    void GetMessages(std::vector<APIAnalyzerMessage>& output)
    {
        output.assign(m_msgList.begin(), m_msgList.end());
    }

    /// Clear messages
    virtual void Clear()
    {
        m_msgList.clear();
    }

    /// Gets a flag indicating whether or not this analyzer is enabled
    /// \return true if this analyzer is enabled, false otherwise
    bool IsEnabled() const { return m_bEnabled; }

    /// Gets a flag indicating whether or not this analyzer requires flattening
    /// \return true if this analyzer requiers flattening, false otherwise
    bool DoesRequireFlattening() const { return m_bRequireAPIFlattening; }

    /// Gets the name of the analyzer
    /// \return the name of the analyzer
    const char* GetName() const { return m_strName; }

    /// typedef for a set of APIs
    typedef std::set<T> DependentAPIs;

    /// Gets the set of APIs which this analyser requires
    /// \return a set of APIs which this analyzer requires
    DependentAPIs GetDependentAPIs() const { return m_dependentAPIs; }

    /// typedef for a list of messages
    typedef std::vector<APIAnalyzerMessage> APIAnalyzerMessageList;

    /// Gets the list of messages for this analyzer
    /// \return the list of messages for this analyzer
    APIAnalyzerMessageList GetMessageList() const { return m_msgList; }

protected:
    bool                   m_bEnabled;              ///< Flag indicating whether or not Analyzer is enabled
    bool                   m_bEndAnalyze;           ///< Flag indicating whether or not EndAnalyze() is called
    const char*            m_strName;               ///< Name of the analyzer
    APIAnalyzerMessageList m_msgList;               ///< APIAnalyzerMessage array
    DependentAPIs          m_dependentAPIs;         ///< Dependent APIs
    bool                   m_bRequireAPIFlattening; ///< Does this analyzer work on flattened APIs or not, if true, all dependent APIs will be flattened

private:
    /// Copy constructor
    /// \param obj object
    APIAnalyzer(const APIAnalyzer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const APIAnalyzer& operator = (const APIAnalyzer& obj);
};

//------------------------------------------------------------------------------------
/// API Analyzer manager
/// Expectation: Analyzer is an APIAnalyzer specialization
/// Expectation: T is an API-specific enum of function types (same as used in APIAnalyzer)
//------------------------------------------------------------------------------------
template <class Analyzer, typename T>
class APIAnalyzerManager
{
public:
    /// Constructor
    APIAnalyzerManager(void) : m_uiMaxNumMsg(200)
    {}

    /// Virtual destructor
    virtual ~APIAnalyzerManager(void) {}

    /// Generate HTML table from statistic data and write to std::ostream
    /// \param sout output stream
    void GenerateHTMLTable(std::ostream& sout)
    {
        HTMLTable table;
        table.AddColumn("Index")
        .AddColumn("Call Index")
        .AddColumn("Thread ID")
        .AddColumn("Type")
        .AddColumn("Message");

        unsigned int idx = 0;
        std::stringstream ssMsg;

        for (auto it = m_analyzers.begin(); it != m_analyzers.end(); ++it)
        {
            //(*it)->EndAnalyze();
            auto msgList = (*it)->GetMessageList();
            unsigned int count = 0;
            bool bMaxReached = false;

            for (auto msgIt = msgList.begin(); msgIt != msgList.end() && !bMaxReached; msgIt++)
            {
                HTMLTableRow row(&table);
                row.AddItem(0, StringUtils::ToString(idx));
                row.AddItem(1, msgIt->bHasDisplayableSeqId ? StringUtils::ToString(msgIt->uiDisplaySeqID) : "N/A");
                row.AddItem(2, StringUtils::ToString(msgIt->uiTID));

                switch (msgIt->type)
                {
                    case MSGTYPE_Error:
                        row.AddItem(3, "Error");
                        break;

                    case MSGTYPE_Warning:
                        row.AddItem(3, "Warning");
                        break;

                    case MSGTYPE_BestPractices:
                        row.AddItem(3, "Best Practices");
                        break;
                }

                std::string keyValues;
                keyValues = GenerateHTMLKeyValue(gs_THREAD_ID_TAG, msgIt->uiTID);
                keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_SEQUENCE_ID_TAG, msgIt->uiSeqID));
                keyValues = AppendHTMLKeyValue(keyValues, GenerateHTMLKeyValue(gs_VIEW_TAG, gs_VIEW_TRACE_TAG));
                std::string hRef = GenerateHref(keyValues, msgIt->strMsg);

                row.AddItem(4, hRef);
                idx++;
                count++;
                table.AddRow(row);

                if (count >= m_uiMaxNumMsg)
                {
                    bMaxReached = true;
                    SP_TODO("Handle SimpleCLAPIRuleManager differently since it contains more than one rules.");
                    ssMsg << "<p>Warning: Maximum number of messages for " << (*it)->GetName() << " exceeded. " << m_uiMaxNumMsg << " out of " << msgList.size() << " are displayed.</p>";
                }
            }
        }

        table.WriteToStream(sout);
        sout << ssMsg.str();
    }

    /// Add OpenCL API Analyzer
    /// \param pAnalyzer Analyzer pointer
    void AddAnalyzer(Analyzer* pAnalyzer)
    {
        m_analyzers.push_back(pAnalyzer);
    }

    /// Enable/Disable analyzers according to config file
    /// \param op AnalyzeOps object
    /// \return true if any rules are enable
    bool SetEnable(const AnalyzeOps& op)
    {
        if (op.analyzerMap.empty())
        {
            // all are enabled
            if (DoEnableAnalyzer())
            {
                for (auto analyzerIt = m_analyzers.begin(); analyzerIt != m_analyzers.end(); analyzerIt++)
                {
                    AnalyzerInstance* pAnalyzer = (*analyzerIt);

                    if (pAnalyzer->DoesRequireFlattening())
                    {
                        auto dependentAPIs = pAnalyzer->GetDependentAPIs();

                        for (auto it = dependentAPIs.begin(); it != dependentAPIs.end(); ++it)
                        {
                            m_apisToFlatten.insert(*it);
                        }
                    }
                }
            }

            return true;
        }

        bool enabled = false;

        for (auto analyzerIt = m_analyzers.begin(); analyzerIt != m_analyzers.end(); analyzerIt++)
        {
            AnalyzerInstance* pAnalyzer = (*analyzerIt);
            pAnalyzer->SetEnable(op);

            if (pAnalyzer->IsEnabled())
            {
                enabled = true;

                if (pAnalyzer->DoesRequireFlattening())
                {
                    auto dependentAPIs = pAnalyzer->GetDependentAPIs();

                    for (auto it = dependentAPIs.begin(); it != dependentAPIs.end(); ++it)
                    {
                        m_apisToFlatten.insert(*it);
                    }
                }
            }
        }

        return enabled;
    }

    /// Generate simple HTML page
    /// \param szFileName file name
    void GenerateHTMLPage(const char* szFileName)
    {
        if (EndAnalyze() == 0)
        {
            return;
        }

        std::ofstream fout(szFileName);
        fout <<
             "<!-- saved from url=(0014)about:internet -->\n"      // add this line so that java script is enabled automatically
             "<html>\n"
             "<head>\n"
             "<title>Warning(s)/Error(s) Page</title>\n"
             "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=ISO-8859-1\">\n";

        HTMLTable::WriteTableStyle(fout);
        HTMLTable::WriteSortableTableScript(fout);

        fout << "</head>\n";
        fout << "<body>\n";

        GenerateHTMLTable(fout);

        fout << "\n";

        fout <<
             "</body>"
             "</html>";

        fout.close();
    }

protected:
    // virtual function to enable analyzers
    // \return true if sucessful, false otherwise
    virtual bool DoEnableAnalyzer()
    {
        return true;
    }

    // virtual function to and analysis
    // \return true if sucessful, false otherwise
    virtual bool DoEndAnalyze(APIInfo* pAPIInfo)
    {
        SP_UNREFERENCED_PARAMETER(pAPIInfo);
        return true;
    }

private:
    /// Before generating HTML pages, this function is called to
    /// enable the analyzers that required APIs to be flattened to
    /// do analysis and generate messages.
    /// If return value is equal to 0, we don't generate html page
    /// \return number of error/warning messages generated by analyzers
    size_t EndAnalyze()
    {
        size_t ret = 0;

        // Iterate flattened APIs, call each analyzer which requires API flattening
        if (m_apisToFlatten.size() > 0)
        {
            for (auto apiIt = m_flattenedAPIs.begin(); apiIt != m_flattenedAPIs.end(); ++apiIt)
            {
                if (DoEndAnalyze(*apiIt))
                {
                    for (auto it = m_analyzers.begin(); it != m_analyzers.end(); it++)
                    {
                        if ((*it)->DoesRequireFlattening() && (*it)->IsEnabled())
                        {
                            (*it)->FlattenedAPIAnalyze(*apiIt);
                        }
                    }
                }
            }
        }

        // Finalize analysis
        for (auto it = m_analyzers.begin(); it != m_analyzers.end(); it++)
        {
            (*it)->EndAnalyze();
            ret += (*it)->GetMessageList().size();
        }

        return ret;
    }

    /// Copy constructor
    /// \param obj object
    APIAnalyzerManager(const APIAnalyzerManager& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    APIAnalyzerManager& operator= (const APIAnalyzerManager& obj);

protected:
    /// template spceialization typedef for the Analyzers supported by the manager
    typedef APIAnalyzer<T> AnalyzerInstance;

    /// typedef for a list of analyzers
    typedef std::vector<AnalyzerInstance*> AnalyzerList;

    /// typedef for the flattened list of APIs, sorted by time
    typedef std::multiset<APIInfo*, MemberCmp<APIInfo, ULONGLONG, &APIInfo::m_ullEnd> > SortedAPISet;

    AnalyzerList m_analyzers;     ///< APIAnalyzer array
    unsigned int m_uiMaxNumMsg;   ///< Maximum number of messages
    std::set<T>  m_apisToFlatten; ///< APIs to flatten, this is constructed from all enabled analyzers
    SortedAPISet m_flattenedAPIs; ///< Sorted API list by time, for multi-threaded app, we flatten APIs since some of the analysis is time-sensitive
};

#endif // _API_ANALYZER_H_
