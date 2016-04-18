#ifndef _DX_ATP_FILE_H_
#define _DX_ATP_FILE_H_

#include <string>
#include <map>
#include <set>
#include "../Common/FileUtils.h"
#include "APIInfoManagerBase.h"
#include "DX12APIInfo.h"
#include "../sprofile/AtpFile.h"

typedef std::map<osThreadId, std::vector<DX12APIInfo*> > DX12APIInfoMap;

//------------------------------------------------------------------------------------
/// DX API trace result
//------------------------------------------------------------------------------------
class DX12AtpFilePart : public IAtpFilePart, public IAtpFilePartParser, public BaseParser<DX12APIInfo>
{
    enum TraceType
    {
        API,
        GPU
    };
public:
    /// Constructor
    /// \param config Config object
    DX12AtpFilePart(const Config& config, bool shouldReleaseMemory = true);

    /// Destructor
    ~DX12AtpFilePart();

    /// Write header section
    /// If a AptFilePart wants to output to header section, implement this method
    /// \param sout Output stream
    void WriteHeaderSection(SP_fileStream& sout);

    /// Write content section
    /// \param sout Output stream
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    /// \return true if any contents were written, false otherwise
    bool WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID);

    /// Save atp file part into a separate file (Called only in compatibility mode)
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    void SaveToFile(const std::string& strTmpFilePath, const std::string& strPID);

    // For IAtpFilePartParser

    /// Parse input stream
    /// \param in Input stream
    /// \return True if succeeded
    bool Parse(std::istream& in, std::string& outErrorMsg) override;

    /// Parse header
    /// \param strKey Key name
    /// \param strVal Value
    /// \return True if succeeded
    bool ParseHeader(const std::string& strKey, const std::string& strVal);

protected:

    /// Parse a line describing an API function call
    // \param apiStr the string from the trace file, describing the API call
    /// \apiInfo[out] will contain the details of the call
    /// \return true for success parsing
    bool ParseCPUAPICallString(const std::string& apiStr, DX12APIInfo& apiInfo);

    /// Parse a line describing a GPU function call
    // \param apiStr the string from the trace file, describing the API call
    /// \apiInfo[out] will contain the details of the call
    /// \return true for success parsing
    bool ParseGPUAPICallString(const std::string& apiStr, DX12GPUTraceInfo& apiInfo);

    /// Parse a section header line.
    /// \param line the line describing the section header
    /// \return true if the line is indeed a section header, false if not
    bool ParseSectionHeaderLine(const std::string& line);
private:

    /// API Map key = threadID
    DX12APIInfoMap m_DXAPIInfoMap;

    /// Which part of the trace are we reading now?
    TraceType m_currentParsedTraceType;

    /// Will hold the current parsed thread ID:
    osThreadId m_currentParsedThreadID;

    /// Will hold the current parsed thread API count:
    int m_currentParsedThreadAPICount;

    /// Currently not used (assuming that this is always "DX12")
    std::string m_apiStr;

};


#endif //_DX_ATP_FILE_H_
