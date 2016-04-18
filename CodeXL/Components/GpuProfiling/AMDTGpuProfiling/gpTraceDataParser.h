//------------------------------ GPUProfileSessionDataParser.h ------------------------------

#ifndef _GPUPROFILESESSIONDATAPARSER_H_
#define _GPUPROFILESESSIONDATAPARSER_H_

// Qt:
#include <QObject>

// Backend:
#include "IParserListener.h"
#include "IParserProgressMonitor.h"
#include "CLAPIInfo.h"
#include "HSAAPIInfo.h"
#include "DX12APIInfo.h"
#include "VulkanAPIInfo.h"
#include "StackTraceAtpFile.h"
#include "HSAAtpFile.h"
#include "PerfMarkerAtpFile.h"

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

class gpTraceDataContainer;
class GPUSessionTreeItemData;
// ----------------------------------------------------------------------------------
// Class Name:          gpProfileSessionDataParser
// General Description: Is handling the parsing of a GPU profile session
//                      The parser creates and manages the data container and data access
//                      interface for the profiled session
// ----------------------------------------------------------------------------------
class gpTraceDataParser : public IParserListener<CLAPIInfo>, public IParserListener<HSAAPIInfo>,
    public IParserListener<SymbolFileEntry>, IParserListener<PerfMarkerEntry>,
    IParserListener<DX12APIInfo>, IParserListener<VKAPIInfo>, IParserProgressMonitor
{
public:

    /// Constructor
    gpTraceDataParser();

    /// Destructor
    ~gpTraceDataParser();

    /// Loads the session data from the file to the data container
    /// \param traceFilePath the trace file path
    /// \param pSessionItemData an item data representing the session
    /// \param wasParseCanceled[output] true iff the user clicked cancel while parsing
    /// \param pSessionDataContainer the session data container in which the data should be stored
    virtual bool Parse(const osFilePath& traceFilePath, GPUSessionTreeItemData* pSessionItemData, bool& wasParseCanceled);

    /// IParserListener overrides
    /// Sets the session API calls count
    virtual void SetAPINum(osThreadId threadId, unsigned int apiNum);

    /// IParserListener implementation for CL API trace/timeline data in .atp file.
    /// This method is called once for each CL API in the .atp file
    /// \param pAPIInfo the current CLAPIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for DX12 API trace/timeline data in .atp file.
    /// This method is called once for each DX12 API in the .atp file
    /// \param pAPIInfo the current DX12APIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(DX12APIInfo* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for Vulkan API trace/timeline data in .atp file.
    /// This method is called once for each Vulkan API in the .atp file
    /// \param pAPIInfo the current VKAPIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(VKAPIInfo* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for HSA api trace/timeline data in .atp file.
    /// This method is called once for each HSA api in the .atp file
    /// \param pAPIInfo the current CLAPIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for symbol data in .atp file
    /// This method is called once for each entry in the symbol section of the .atp file
    /// \param pSymFileEntry the current SymbolFileEntry item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(SymbolFileEntry* pSymFileEntry, bool& stopParsing);

    /// IParserListener implementation for perf marker data in .atp file
    /// This method is called once for each entry in the perf marker section of the .atp file
    /// \param pPerfMarkerEntry the current PerfMarkerEntry item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(PerfMarkerEntry* pPerfMarkerEntry, bool& stopParsing);

    /// IParserProgressMonitor implementation for reporting progress when loading trace data
    /// \param strProgressMessage the progress message to display for this progress event
    /// \param uiCurItem the index of the current item being parsed
    /// \param uiTotalItems the total number of items to be parsed
    void OnParserProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems);

    /// Load the kernel occupancy data, and store the occupancy info object into the data container
    bool LoadOccupancyFile();

    /// Return the session data container
    gpTraceDataContainer* SessionDataContainer() const { return m_pSessionDataContainer; };

protected:

    /// Session file path
    osFilePath m_traceFilePath;

    /// Tree item data representing the parsed session
    GPUSessionTreeItemData* m_pSessionItemData;

    /// The session trace data container
    gpTraceDataContainer* m_pSessionDataContainer;

    /// True iff the occupancy file is already loaded
    bool m_isOccupancyFileLoaded;

    /// True iff we're in the process of loading the occupancy file
    bool m_isExecutingOccupancyFileLoad;

};

#endif