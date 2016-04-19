//------------------------------ gpObjectDataModel.cpp ------------------------------

// TODO Note: The file is created to match Object data model to trace data model.  Most functionality is not required, need more cleanup

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtASCIIStringTokenizer.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuProfiling/gpObjectDataContainer.h>
#include <AMDTGpuProfiling/gpObjectDataModel.h>
#include <AMDTGpuProfiling/gpObjectDataParser.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/Session.h>

#ifdef GP_OBJECT_VIEW_ENABLE

gpObjectDataModel::gpObjectDataModel(GPUSessionTreeItemData* pSessionItemData) :
    m_pSessionItemData(pSessionItemData), m_pObjectDataContainer(nullptr)
{
}

gpObjectDataModel::~gpObjectDataModel()
{
    delete m_pObjectDataContainer;
}

bool gpObjectDataModel::LoadObjectFile(const osFilePath& ObjectFilePath, bool& wasParseCanceled, gpObjectDataContainer*& pObjectDataContainter)
{
    bool retVal = false;

    wasParseCanceled = false;

    GT_UNREFERENCED_PARAMETER(ObjectFilePath);
    GT_UNREFERENCED_PARAMETER(wasParseCanceled);
    GT_UNREFERENCED_PARAMETER(pObjectDataContainter);

    // Create a parser and parse the file
    gpObjectDataParser* pProfileDataParser = new gpObjectDataParser;

    retVal = pProfileDataParser->Parse(ObjectFilePath, m_pSessionItemData, wasParseCanceled);

    // Get the container and delete the parser
    m_pObjectDataContainer = pProfileDataParser->SessionDataContainer();
    pObjectDataContainter = m_pObjectDataContainer;
    delete pProfileDataParser;

    return retVal;
}

#endif
