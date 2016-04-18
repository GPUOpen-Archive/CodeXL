//------------------------------ gpTraceDataModel.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/gpTraceDataModel.h>
#include <AMDTGpuProfiling/gpTraceDataParser.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/Session.h>

gpTraceDataModel::gpTraceDataModel(GPUSessionTreeItemData* pSessionItemData) :
    m_pSessionItemData(pSessionItemData), m_pTraceDataContainer(nullptr)
{
}


gpTraceDataModel::~gpTraceDataModel()
{
    delete m_pTraceDataContainer;
}

bool gpTraceDataModel::LoadTraceFile(const osFilePath& traceFilePath, bool& wasParseCanceled, gpTraceDataContainer*& pTraceDataContainter)
{
    bool retVal = false;

    wasParseCanceled = false;

    // Create a parser and parse the file
    gpTraceDataParser* pProfileDataParser = new gpTraceDataParser;

    retVal = pProfileDataParser->Parse(traceFilePath, m_pSessionItemData, wasParseCanceled);

    // Get the container and delete the parser
    m_pTraceDataContainer = pProfileDataParser->SessionDataContainer();
    pTraceDataContainter = m_pTraceDataContainer;
    delete pProfileDataParser;

    return retVal;
}
