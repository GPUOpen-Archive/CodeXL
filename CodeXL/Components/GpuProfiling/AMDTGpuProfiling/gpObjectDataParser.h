//------------------------------ gpObjectDataParser.h ------------------------------

#ifndef _GPUPROFILESESSIONDATAPARSER_H_
#define _GPUPROFILESESSIONDATAPARSER_H_

// Qt:
#include <QObject>

// RCP Backend:
#include <IParserListener.h>
#include <IParserProgressMonitor.h>
#include <IAtpDataHandler.h>

#include "DX12Trace/DX12APIInfo.h"

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

class gpObjectDataContainer;
class GPUSessionTreeItemData;
// ----------------------------------------------------------------------------------
// Class Name:          gpProfileSessionDataParser
// General Description: Is handling the parsing of a GPU profile session
//                      The parser creates and manages the data container and data access
//                      interface for the profiled session
// ----------------------------------------------------------------------------------
class gpObjectDataParser : IParserListener<DX12APIInfo>, IParserListener<VKAPIInfo>, IParserProgressMonitor
{
public:

    /// Constructor
    gpObjectDataParser();

    /// Destructor
    ~gpObjectDataParser();

    /// Loads the session data from the file to the data container
    /// \param objectFilePath the object file path
    /// \param pSessionItemData an item data representing the session
    /// \param wasParseCanceled[output] true iff the user clicked cancel while parsing
    /// \param pSessionDataContainer the session data container in which the data should be stored
    virtual bool Parse(const osFilePath& objectFilePath, GPUSessionTreeItemData* pSessionItemData, bool& wasParseCanceled);

    /// IParserListener overrides
    /// Sets the session API calls count
    virtual void SetAPINum(osThreadId threadId, unsigned int apiNum);

    /// IParserListener implementation for CL API object/timeline data in .atp file.
    /// This method is called once for each CL API in the .atp file
    /// \param pAPIInfo the current CLAPIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(ICLAPIInfoDataHandler* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for DX12 API object/timeline data in .atp file.
    /// This method is called once for each DX12 API in the .atp file
    /// \param pAPIInfo the current DX12APIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(DX12APIInfo* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for Vulkan API object/timeline data in .atp file.
    /// This method is called once for each Vulkan API in the .atp file
    /// \param pAPIInfo the current VKAPIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(VKAPIInfo* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for HSA api object/timeline data in .atp file.
    /// This method is called once for each HSA api in the .atp file
    /// \param pAPIInfo the current CLAPIInfo item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(IHSAAPIInfoDataHandler* pAPIInfo, bool& stopParsing);

    /// IParserListener implementation for symbol data in .atp file
    /// This method is called once for each entry in the symbol section of the .atp file
    /// \param pSymFileEntry the current SymbolFileEntry item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(ISymbolFileEntryInfoDataHandler* pSymFileEntry, bool& stopParsing);

    /// IParserListener implementation for perf marker data in .atp file
    /// This method is called once for each entry in the perf marker section of the .atp file
    /// \param pPerfMarkerEntry the current PerfMarkerEntry item from the parser
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(IPerfMarkerInfoDataHandler* pPerfMarkerEntry, bool& stopParsing);

    /// IParserProgressMonitor implementation for reporting progress when loading object data
    /// \param strProgressMessage the progress message to display for this progress event
    /// \param uiCurItem the index of the current item being parsed
    /// \param uiTotalItems the total number of items to be parsed
    void OnParserProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems);

    /// Load the kernel occupancy data, and store the occupancy info object into the data container
    bool LoadOccupancyFile();

    /// Return the session data container
    gpObjectDataContainer* SessionDataContainer() const { return m_pSessionDataContainer; };

protected:

    /// Session file path
    osFilePath m_objectFilePath;

    /// Tree item data representing the parsed session
    GPUSessionTreeItemData* m_pSessionItemData;

    /// The session object data container
    gpObjectDataContainer* m_pSessionDataContainer;

    /// True iff the occupancy file is already loaded
    bool m_isOccupancyFileLoaded;

    /// True iff we're in the process of loading the occupancy file
    bool m_isExecutingOccupancyFileLoad;

};

#endif
