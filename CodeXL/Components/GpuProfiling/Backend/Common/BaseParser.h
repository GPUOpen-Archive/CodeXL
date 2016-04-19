//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Base parser class
//==============================================================================

#ifndef _BASE_PARSER_H_
#define _BASE_PARSER_H_

#include <vector>
#include <fstream>
#include <string>
#include "IParserListener.h"
#include "Logger.h"

using namespace GPULogger;


#define READLINE(line) bError = !ReadLine(in, line); \
    if (bError) \
    { \
        m_bWarning = true; \
        m_strWarningMsg = StringUtils::FormatString("AtpFileParser: Failed to input read stream @ line %d", m_nLine); \
        SpBreak("Parsing error"); \
        return false; \
    }

//------------------------------------------------------------------------------------
/// Base class for all Parsers (.atp, .csv)
//------------------------------------------------------------------------------------
template <class T>
class BaseParser
{
public:
    /// Constructor
    BaseParser(void)
    {
        m_bWarning = false;
        m_strWarningMsg.clear();
        m_nLine = 0;
        m_bValidLastPos = false;
    }

    /// Virtual destructor
    virtual ~BaseParser(void)
    {}

    /// Get parse warning
    /// \param[out] bWarning warning flag
    /// \param[out] strMsg warning msg
    void GetParseWarning(bool& bWarning, std::string& strMsg)
    {
        bWarning = m_bWarning;
        strMsg = m_strWarningMsg;
    }

    /// Add Listener
    /// Parser generate T object and pass to Listener to do further processing
    /// \param pListener Listener object
    void AddListener(IParserListener<T>* pListener)
    {
        if (pListener != NULL)
        {
            m_listenerList.push_back(pListener);
        }
    }
    //------------------------------------------------------------------------------------
    /// Helper class to hold and update error message string passed to some BaseParser
    /// The class is just a helper , in order to keep the latest error message of BaseParser class
    //------------------------------------------------------------------------------------
    class ErrorMessageUpdater
    {
    public:
        ErrorMessageUpdater() = delete;
        ErrorMessageUpdater(const ErrorMessageUpdater&) = delete;
        /// Constructor
        /// Use  RAII concept in order to update given reference to error message
        /// \param strMessageToUpdate - string message that shall be updated on object end of lifetime
        /// \param pBaseParser - use this base parser error message to update
        ErrorMessageUpdater(std::string& strMessageToUpdate, BaseParser* pBaseParser) : m_strMessageToUpdate(strMessageToUpdate), m_pBaseParser(pBaseParser) {}

        virtual ~ErrorMessageUpdater()
        {
            if (m_pBaseParser != nullptr)
            {
                bool bWarning = false;
                m_pBaseParser->GetParseWarning(bWarning, m_strMessageToUpdate);
            }

        }
    private:
        std::string& m_strMessageToUpdate;
        BaseParser*  m_pBaseParser;
    };

protected:
    /// Read line
    /// \param in Input stream
    /// \param[out] line Output line
    /// \return True if no error
    virtual bool ReadLine(std::istream& in, std::string& line)
    {
        m_bValidLastPos = true;
        m_lastPos = in.tellg();
        getline(in, line);

        if (in.fail())
        {
            if (in.eof())
            {
                return true;
            }
            else
            {
                Log(logERROR, "Error bit set in istream.\n");
                return false;
            }
        }

        m_nLine++;
        return true;
    }

    /// Rewind to previous position, it can only keep track of one step back
    /// \param in Input stream
    /// \return True if succeeded
    bool RewindToPreviousPos(std::istream& in)
    {
        if (m_bValidLastPos)
        {
            in.seekg(m_lastPos);
            m_bValidLastPos = false;
            return true;
        }
        else
        {
            return false;
        }
    }

    std::vector<IParserListener<T>*> m_listenerList;   ///< Listener list
    bool m_bWarning;                                   ///< set to true if warning happens in parsing, if summary pages are created if (Parse() || m_bWarning)
    std::string m_strWarningMsg;                       ///< warning message
    size_t m_nLine;                                    ///< Line number
    std::streampos m_lastPos;                          ///< last stream position
    bool m_bValidLastPos;                              ///< A flag indicating whether or not last position is valid

private:
    /// Disable copy constructor
    BaseParser(const BaseParser& rhs);

    /// Disable assignment operator
    BaseParser& operator = (const BaseParser& rhs);
};

//------------------------------------------------------------------------------------
/// File parser
//------------------------------------------------------------------------------------
template <class T = void*>
class BaseFileParser : public BaseParser<T>
{
public:
    /// Constructor
    BaseFileParser() : BaseParser<T>()
    {
        m_bFileOpen = false;
    }

    /// Destructor
    virtual ~BaseFileParser() {}

    /// Load File
    /// \param szFileName File name
    /// \return true if succeed
    bool LoadFile(const char* szFileName)
    {
        fin.open(szFileName);

        if (fin.is_open())
        {
            m_bFileOpen = true;
            m_strFileName = szFileName;
            return true;
        }
        else
        {
            return false;
        }
    }

    /// Close file
    void Close()
    {
        if (fin.is_open())
        {
            fin.close();
        }
    }

    /// Parse
    virtual bool Parse() = 0;

protected:
    /// Helper function to read line from file stream
    /// \param[out] line Output line
    /// \return true if succeeded
    bool ReadLine(std::string& line)
    {
        return BaseParser<T>::ReadLine(fin, line);
    }

    std::ifstream fin;                                 ///< ifstream
    std::string m_strFileName;                         ///< File name
    bool m_bFileOpen;                                  ///< Flag indicating whether files is opened or not
};

#endif

