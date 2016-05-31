//------------------------------ kaProjectDataManager.cpp ------------------------------

// Qt
// needed again although it is included in the header since someone in the path is doing a pop
#include <qtIgnoreCompilerWarnings.h>

// TinyXml:
#include <tinyxml.h>

// std find
#include <algorithm>
#include <ctype.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalSettingsPage.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/Include/kaAppWrapper.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>

// Static member initializations:
kaProjectDataManager* kaProjectDataManager::m_psMySingleInstance = NULL;

QList<QRegExp> kaProjectDataManager::m_sKeywordsList;

#define KA_DEFAULT_GLOBAL_SIZE 64
#define KA_DEFAULT_LOCAL_SIZE 4

//defines for kernels code source parsing
#define KERNEL_FUNCTION_ATTRIBUTE             "__attribute__"
#define KERNEL_FUNCTION_KERNEL_ATTRIBUTE      "__kernel"
#define KERNEL_FUNCTION_MANGLED_NAME_ATTRIBUTE "mangled_name"


// ---------------------------------------------------------------------------
// Name:        kaProjectDataManagerAnaylzeData::kaProjectDataManagerAnaylzeData
// Description: constructor
// Return Val:
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
kaProjectDataManagerAnaylzeData::kaProjectDataManagerAnaylzeData()
{
    m_globalWorkSize[0] = KA_DEFAULT_GLOBAL_SIZE;
    m_globalWorkSize[1] = KA_DEFAULT_GLOBAL_SIZE;
    m_globalWorkSize[2] = KA_DEFAULT_GLOBAL_SIZE;
    m_localWorkSize[0] = KA_DEFAULT_LOCAL_SIZE;
    m_localWorkSize[1] = KA_DEFAULT_LOCAL_SIZE;
    m_localWorkSize[2] = KA_DEFAULT_LOCAL_SIZE;
}

// ---------------------------------------------------------------------------
void kaProjectDataManagerAnaylzeData::CopyDataFromExecutionData(const kaKernelExecutionDataStruct& executionData)
{
    for (int i = 0; i < 3; i++)
    {
        m_globalWorkSize[i] = executionData.m_globalWorkSize[i];
        m_localWorkSize[i] = executionData.m_localWorkSize[i];
    }

    m_loopIterations = executionData.m_loopIterations;

}



// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::kaProjectDataManager
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
kaProjectDataManager::kaProjectDataManager() : m_shaderD3dBuildOptionsMask(0),
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_buildArchitecture(kaBuildArch32_bit)
#else
    m_buildArchitecture(kaBuildArch64_bit)
#endif

{
    m_sourceFileManager.reset(new kaFileManager());

    QString tmpStr = KA_STR_HLSLSemanticKeyWords;
    tmpStr += "," + QString(KA_STR_ProgrammingKeyWords);

    tmpStr.remove('\t');
    tmpStr.remove(' ');
    QStringList wordsList = tmpStr.split(',');

    foreach (QString word, wordsList)
    {
        QRegExp rx(word, Qt::CaseInsensitive);
        rx.setPatternSyntax(QRegExp::Wildcard);
        m_sKeywordsList.append(rx);
    }

    m_glShaderTypesList = QString(KA_STR_toolbarTypeData).split(AF_STR_SpaceA);
    m_glShaderDetectorsVector.resize(GLSHADERTYPES_TOTAL);
    m_glShaderDetectorsVector[FRAGMENT] = QString(KA_STR_FragmentShaderIndicators).split(AF_STR_SpaceA);
    m_glShaderDetectorsVector[VERTEX] = QString(KA_STR_VertexShaderIndicators).split(AF_STR_SpaceA);
    m_glShaderDetectorsVector[COMPUTE] = QString(KA_STR_ComputeShaderIndicators).split(AF_STR_SpaceA);
    m_glShaderDetectorsVector[GEOMETRY] = QString(KA_STR_GeometryShaderIndicators).split(AF_STR_SpaceA);
    m_glShaderDetectorsVector[TESSCONT] = QString(KA_STR_TessContShaderIndicators).split(AF_STR_SpaceA);
    m_glShaderDetectorsVector[TESSEVAL] = QString(KA_STR_TessEvalShaderIndicators).split(AF_STR_SpaceA);

    m_glShaderExtensions = QString(KA_STR_OpenGLShaderExtensions).split(AF_STR_CommaA);
    m_dxShaderExtensions = QString(KA_STR_DirectXShaderExtensions).split(AF_STR_CommaA);
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::~kaProjectDataManager
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
kaProjectDataManager::~kaProjectDataManager()
{
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
kaProjectDataManager& KA_PROJECT_DATA_MGR_INSTANCE
{
    if (m_psMySingleInstance == NULL)
    {
        m_psMySingleInstance = new kaProjectDataManager;

        GT_ASSERT(m_psMySingleInstance);
    }

    return *m_psMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::getXMLSettingsString
// Description: Get the current debugger project settings as XML string
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
bool kaProjectDataManager::getXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = true;
    gtString buildArchitecture;
    GetProjectArchitectureAsString(buildArchitecture);
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectArchitecture, buildArchitecture);

    m_sourceFileManager->Serialize(projectAsXMLString);

    projectAsXMLString.appendFormattedString(L"<%ls>", KA_STR_programSection);

    for (auto itr : m_programsList)
    {
        projectAsXMLString.appendFormattedString(L"<%ls>", KA_STR_program);
        itr->Serialize(projectAsXMLString);
        projectAsXMLString.appendFormattedString(L"</%ls>", KA_STR_program);

    }

    projectAsXMLString.appendFormattedString(L"</%ls>", KA_STR_programSection);

    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingBuildOptionsNode, acQStringToGTString(m_buildCommandOptions));

    // get build options and compile type from setting page and set the XML node
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingShaderCompileTypeNode, acQStringToGTString(m_shaderCompileType));
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingShaderBuildOptionsNode, acQStringToGTString(m_shaderBuildCommandOptions));
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingShaderD3dBuilderPathNode, acQStringToGTString(m_shaderD3dBuilderPath));
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingShaderFxcBuilderPathNode, acQStringToGTString(m_shaderFxcBuilderPath));
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingShaderMacrosNode, acQStringToGTString(m_shaderMacros));
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingShaderIncludesNode, acQStringToGTString(m_shaderIncludes));
    afUtils::addFieldToXML(projectAsXMLString, KA_STR_projectSettingKernelMacrosNode, acQStringToGTString(m_kernelMacros));

    return retVal;
}

void kaProjectDataManager::GetProjectArchitectureAsString(gtString& buildArchitecture) const
{
    buildArchitecture = (m_buildArchitecture == kaBuildArch32_bit) ? AF_STR_loadProjectArchitecture32Bit : AF_STR_loadProjectArchitecture64Bit;
}

void kaProjectDataManager::SetProjectArchitectureFromString(const gtString& buildArchitecture)
{
    GT_IF_WITH_ASSERT(false == buildArchitecture.isEmpty())
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        if (buildArchitecture.compare(AF_STR_loadProjectArchitecture64Bit) == 0)
        {
            m_buildArchitecture = kaBuildArch64_bit;
        }
        else
        {
            m_buildArchitecture = kaBuildArch32_bit;
        }

#else
        m_buildArchitecture = kaBuildArch64_bit;
#endif
    }
}

bool kaProjectDataManager::IsCLFileSelected()
{
    bool isCLFileSelected = false;
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            if (pItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM_SHADER)
            {
                kaProgram* pProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetActiveProgram();

                if (pProgram != nullptr)
                {
                    if (pProgram->GetBuildType() == kaProgramCL)
                    {
                        isCLFileSelected = true;
                    }
                }
            }
            else if (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if (pKAData != nullptr)
                {
                    kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pKAData->filePath());

                    if (pFile != nullptr)
                    {
                        gtString buildPlatform = pFile->BuildPlatform();

                        if (buildPlatform == KA_STR_platformOpenCL_GT)
                        {
                            isCLFileSelected = true;
                        }
                    }
                }
            }
        }
    }

    return isCLFileSelected;
}

bool kaProjectDataManager::IsDXShaderSelected()
{
    bool isDXShaderSelected = false;
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            if (pItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM_SHADER)
            {
                kaProgram* pProgram = GetActiveProgram();

                if (pProgram != nullptr)
                {
                    if (pProgram->GetBuildType() == kaProgramDX)
                    {
                        isDXShaderSelected = true;
                    }
                }
            }
            else if (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE)
            {
                kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

                if (pKAData != nullptr)
                {
                    kaSourceFile* pFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(pKAData->filePath());

                    if (pFile != nullptr)
                    {
                        gtString buildPlatform = pFile->BuildPlatform();

                        if (buildPlatform == KA_STR_platformDirectX_GT)
                        {
                            isDXShaderSelected = true;
                        }
                    }
                }
            }
        }
    }

    return isDXShaderSelected;
}

bool kaProjectDataManager::IsSourceFileSelected()
{
    bool isSourceFileSelected = false;
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            if (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE)
            {
                isSourceFileSelected = true;
            }
        }
    }

    return isSourceFileSelected;
}



// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::setSettingsFromXMLString
// Description: Get the project settings from the XML string
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
bool kaProjectDataManager::setSettingsFromXMLString(const gtString& projectAsXMLString, TiXmlNode* pMainNode)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pMainNode != nullptr)
    {
        GT_UNREFERENCED_PARAMETER(projectAsXMLString);
        // Clear the project manager:
        KA_PROJECT_DATA_MGR_INSTANCE.ClearAllAndDelete();
        gtString projectArchitecture;

        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectArchitecture, projectArchitecture);
        SetProjectArchitectureFromString(projectArchitecture);

        if (pMainNode->FirstChild() != nullptr)
        {
            TiXmlElement* pFilesElement = pMainNode->FirstChild()->NextSiblingElement();
            m_sourceFileManager->DeSerialize(pFilesElement);
        }

        DeserializeProgram(pMainNode);
        gtString commandString;
        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectSettingBuildOptionsNode, commandString);
        QString commandStringQT = acGTStringToQString(commandString);
        kaApplicationCommands::instance().setBuildOptions(commandStringQT);
        kaApplicationCommands::instance().SetToolbarBuildOptions(commandStringQT);

        // set the compile type to settings page from XML
        gtString shaderCompileTypeString;
        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectSettingShaderCompileTypeNode, shaderCompileTypeString);
        commandStringQT = acGTStringToQString(shaderCompileTypeString);
        SetShaderCompileType(commandStringQT);

        // set the D3d builder path to settings page from XML
        gtString shaderD3dBuilderPathString;
        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectSettingShaderD3dBuilderPathNode, shaderD3dBuilderPathString);
        commandStringQT = acGTStringToQString(shaderD3dBuilderPathString);

        if (!commandStringQT.isEmpty())
        {
            SetShaderD3dBuilderPath(commandStringQT);
        }

        // set the Fxc builder path to settings page from XML
        gtString shaderFxcBuilderPathString;
        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectSettingShaderFxcBuilderPathNode, shaderFxcBuilderPathString);
        commandStringQT = acGTStringToQString(shaderFxcBuilderPathString);

        if (!commandStringQT.isEmpty())
        {
            SetShaderFxcBuilderPath(commandStringQT);
        }

        // set the build command macros to settings page from XML
        gtString shaderMacrosString;
        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectSettingShaderMacrosNode, shaderMacrosString);
        commandStringQT = acGTStringToQString(shaderMacrosString);
        SetShaderMacros(commandStringQT);

        // set the build command includes to settings page from XML
        gtString shaderIncludesString;
        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectSettingShaderIncludesNode, shaderIncludesString);
        commandStringQT = acGTStringToQString(shaderIncludesString);
        SetShaderIncludes(commandStringQT);

        // update the settings page with the new build options from XML
        gtString shaderCommandString;
        afUtils::getFieldFromXML(*pMainNode, KA_STR_projectSettingShaderBuildOptionsNode, shaderCommandString);
        QString shaderCommandStringQT = acGTStringToQString(shaderCommandString);
        SetShaderBuildOptions(shaderCommandStringQT);
        kaApplicationCommands::instance().SetToolbarBuildOptions(shaderCommandStringQT);

        // Fill all the cl files stored in the project manager:
        kaApplicationTreeHandler::instance()->fillTreeFromProjectManager();
        retVal = true;
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::clearAllAndDelete
// Description: delete all file data objects and clear the list
// Return Val:  void
// Author:      Roman Bober
// Date:        28/12/2015
// ---------------------------------------------------------------------------
void kaProjectDataManager::ClearAllAndDelete()
{
    // Clear all files
    osFilePath removed;

    while (m_sourceFileManager->RemoveNext(removed))
    {
        afDocUpdateManager::instance().UnregisterDocumentOfActivate(removed, this);
    }

    // Clear all programs
    m_programsList.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::dataFile
// Description: get the datafile connected to a file path
// Arguments:   osFilePath& iFilePath
// Return Val:  kaSourceFile*
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
kaSourceFile* kaProjectDataManager::dataFileByPath(const osFilePath& iFilePath)
{
    kaSourceFile* retVal = m_sourceFileManager->GetFile(iFilePath);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::AddFileOnProjectLoad
// Description: Add file to the project. We assume that the file doesn't exist
// Arguments:   osFilePath& iFilePath
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/8/2013
// ---------------------------------------------------------------------------
kaSourceFile* kaProjectDataManager::AddFileOnProjectLoad(const osFilePath& iFilePath)
{
    // Create the file item data:
    kaSourceFile* pRetVal = new kaSourceFile();
    // set the file info (this is all the data available now and that is needed):
    pRetVal->setFilePath(iFilePath);

    m_sourceFileManager->Add(pRetVal);

    // Add listening to the file update:
    afDocUpdateManager::instance().RegisterDocumentActivate(iFilePath, this, false);
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::removeFile
// Description: remove a file from the data list
// Author:      Gilad Yarnitzky
// Date:        22/8/2013
// ---------------------------------------------------------------------------
bool kaProjectDataManager::removeFile(const osFilePath& iFilePath)
{
    bool retVal = false;

    size_t  id = m_sourceFileManager->GetFileId(iFilePath);
    retVal = m_sourceFileManager->Remove(id);

    // Remove the listening to the file update:
    afDocUpdateManager::instance().UnregisterDocumentOfActivate(iFilePath, this);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::buildFunctionsList
// Description: build the kernel list for the file. If there is already a list replace only
//              the new kernels removing non existent kernels
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
void kaProjectDataManager::BuildFunctionsList(const osFilePath& iFilePath, kaSourceFile* pFileData)
{
    if (pFileData == NULL)
    {
        pFileData = dataFileByPath(iFilePath);
    }

    QString fileString;
    bool rc = kaReadFileAsQString(iFilePath, fileString);

    GT_IF_WITH_ASSERT(NULL != pFileData && rc)
    {
        if (!buildKernelList(fileString, iFilePath, pFileData))
        {
            buildEntryPointList(fileString, pFileData);
        }

        //sort vector lexicographically
        auto& functionList = pFileData->analyzeVector();

        if (functionList.size() > 0)
        {
            std::sort(functionList.begin(), functionList.end(), kaProjectDataManagerAnaylzeData::Compare());
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::getKernelData
// Description: Get data of a specific kernel
// Arguments:   int iKernelIndex
// Return Val:  kaProjectDataManagerStruct*
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
kaProjectDataManagerAnaylzeData kaProjectDataManager::getKernelData(int iKernelIndex, kaSourceFile* pCurrentFileData)
{
    kaProjectDataManagerAnaylzeData retVal;
    int numKernels = pCurrentFileData->analyzeVector().size();

    if (iKernelIndex >= 0 && iKernelIndex < numKernels)
    {
        retVal = pCurrentFileData->analyzeVector().at(iKernelIndex);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::addKernelData
// Description: Insert Kernel data, add it if it is new or overwrite if it exists
// Arguments:   kaProjectDataManagerStruct& iKernelData
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
void kaProjectDataManager::addKernelData(kaProjectDataManagerAnaylzeData& iKernelData, kaSourceFile* pCurrentFileData)
{
    int numKernels = pCurrentFileData->analyzeVector().size();

    int foundKernelIndex = -1;

    // Look for kernel of that name:
    for (int nKernel = 0; nKernel < numKernels; nKernel++)
    {
        kaProjectDataManagerAnaylzeData currentData = getKernelData(nKernel, pCurrentFileData);

        if (currentData.m_kernelName == iKernelData.m_kernelName)
        {
            foundKernelIndex = nKernel;
        }
    }

    if (-1 == foundKernelIndex)
    {
        pCurrentFileData->analyzeVector().push_back(iKernelData);
    }
    else
    {
        pCurrentFileData->analyzeVector()[foundKernelIndex] = iKernelData;
    }
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::changeKernelDataAtIndex
// Description: Change a data struct at index
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
void kaProjectDataManager::changeKernelDataAtIndex(kaProjectDataManagerAnaylzeData& iKernelData, int iKernelIndex, kaSourceFile* pCurrentFileData)
{
    int numKernels = pCurrentFileData->analyzeVector().size();

    if (iKernelIndex >= 0 && iKernelIndex < numKernels)
    {
        pCurrentFileData->analyzeVector()[iKernelIndex] = iKernelData;
    }
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::removeKernelByIndex
// Description: Remove kernel data struct by index
// Arguments:   int iKernelIndex
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
void kaProjectDataManager::removeKernelByIndex(int iKernelIndex, kaSourceFile* pCurrentFileData)
{
    int numKernels = pCurrentFileData->analyzeVector().size();

    if (iKernelIndex >= 0 && iKernelIndex < numKernels)
    {
        for (int nKernel = iKernelIndex + 1; nKernel < numKernels; nKernel++)
        {
            pCurrentFileData->analyzeVector()[nKernel - 1] = pCurrentFileData->analyzeVector()[nKernel];
        }

        pCurrentFileData->analyzeVector().resize(numKernels - 1);
    }
}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::buildKernelTable
// Description: Build the kernels list from the text
// Arguments:   const QString& text
// Return Val:  bool: true if kernels were found
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
bool kaProjectDataManager::buildKernelList(const QString& fileData, const osFilePath& filePath, kaSourceFile* pCurrentFileData) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pCurrentFileData != nullptr)
    {
        //TODO add here predefined by project macros
        vector<std::string> additionalMacros;
        const QStringList macroList = KernelMacros().split(";");

        for (const QString& str :  macroList)
        {
            ///make sure that it's real macro definition <key>=<value>
            if (str.isEmpty() == false && str.split("=").size() == 2)
            {
                additionalMacros.push_back(str.toStdString());
            }
        }

        const size_t analyseVecSize = pCurrentFileData->analyzeVector().size();
        pCurrentFileData->analyzeVector().clear();

        FillKernelNamesList(fileData, filePath, additionalMacros, pCurrentFileData->analyzeVector());


        //we return true only if added some kernels
        retVal = analyseVecSize < pCurrentFileData->analyzeVector().size();
    }
    return retVal;
}

void kaProjectDataManager::FillKernelNamesList(const QString& fileData, const osFilePath& filePath, const std::vector<std::string>& additionalMacros, gtVector<kaProjectDataManagerAnaylzeData>& kernelList)
{
    vector<PreProcessedToken> tokens;
    string sourceCodeData = fileData.toUtf8().constData();
    ExpandMacros(sourceCodeData, filePath.asString().asCharArray(), additionalMacros, tokens);

    if (tokens.size() > 0)
    {
        auto  token = tokens.begin();

        do
        {
            //advance iterator
            token = find_if(token, tokens.end(), [](const PreProcessedToken & preprocessedToken)
            {
                return preprocessedToken.value == KERNEL_FUNCTION_KERNEL_ATTRIBUTE ||
                       preprocessedToken.value == KERNEL_FUNCTION_MANGLED_NAME_ATTRIBUTE;
            });

            //we finished, break here
            if (token == tokens.end())
            {
                break;
            }

            if (token->value == KERNEL_FUNCTION_MANGLED_NAME_ATTRIBUTE)
            {
                AddMangledKernelName(token, tokens, kernelList);
            }
            else
            {
                //skip  to return value/attribute
                while (++token != tokens.end() && isspace(token->value[0]));

                if (token != tokens.end())
                {
                    //skip attributes of the function till we get to return value, attributes are all inside parentheses : __attribute__((reqd_work_group_size(LOCAL_XRES, LOCAL_YRES, 1)))
                    if (token->value == KERNEL_FUNCTION_ATTRIBUTE)
                    {
                        //skip till the first parenthese of the attribute
                        while (++token != tokens.end() && isspace(token->value[0]));

                        if (token != tokens.end() && token->value == "(")
                        {
                            size_t parentesisCount = 1;

                            while (++token != tokens.end() && parentesisCount > 0)
                            {
                                if (token->value == "(")
                                {
                                    ++parentesisCount;
                                }
                                else if (token->value == ")")
                                {
                                    --parentesisCount;
                                }

                            }
                        }

                        //skip return value
                        while (++token != tokens.end() && isspace(token->value[0]));
                    }

                    if (token != tokens.end())
                    {
                        //skip  spaces till kernel name
                        while (++token != tokens.end() && isspace(token->value[0]));

                        //we found real kernel, add it to analyze vector
                        if (token != tokens.end())
                        {
                            kaProjectDataManagerAnaylzeData newDataStruct;
                            newDataStruct.m_kernelName = token->value.c_str();
                            newDataStruct.m_lineInSourceFile = token->line;
                            kernelList.push_back(newDataStruct);
                            ++token;
                        }
                    }
                }
            }
        }
        while (token != tokens.end());
    }//if tokens not empty
}

void kaProjectDataManager::AddMangledKernelName(vector<PreProcessedToken>::iterator& token, vector<PreProcessedToken>& tokens, gtVector<kaProjectDataManagerAnaylzeData>& kernelList)
{
    //skip  to mangled name
    while (++token != tokens.end() && (isspace(token->value[0]) || token->value == "("));

    if (token != tokens.end())
    {
        kaProjectDataManagerAnaylzeData newDataStruct;
        newDataStruct.m_kernelName = token->value.c_str();
        newDataStruct.m_lineInSourceFile = token->line;
        kernelList.push_back(newDataStruct);
        //skip template name after mangled name
        token = find_if(token, tokens.end(), [](const PreProcessedToken & preprocessedToken) { return preprocessedToken.value == KERNEL_FUNCTION_KERNEL_ATTRIBUTE; });

        if (token != tokens.end())
        {
            ++token;
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::UpdateFunctionsList(gtVector<QString>& functionsFound, gtVector<int>& functionsFoundLineNumber, kaSourceFile* pCurrentFileData)
{
    GT_IF_WITH_ASSERT(pCurrentFileData != nullptr)
    {
        int numFunctionsFound = functionsFound.size();

        // First stage look in the existing list and look for deleted kernels:
        int numKernels = pCurrentFileData->analyzeVector().size();

        for (int nKernel = 0; nKernel < numKernels; nKernel++)
        {
            kaProjectDataManagerAnaylzeData currentData = getKernelData(nKernel, pCurrentFileData);
            QString currentKernelName = currentData.m_kernelName;

            bool found = false;

            for (int nKernelName = 0; nKernelName < numFunctionsFound && !found; nKernelName++)
            {
                if (functionsFound.at(nKernelName) == currentKernelName)
                {
                    found = true;
                }
            }

            if (!found && numFunctionsFound != 0)
            {
                removeKernelByIndex(nKernel, pCurrentFileData);
                nKernel--;
                numKernels--;
            }
        }

        // Second stage add all new kernels:
        for (int nKernelName = 0; nKernelName < numFunctionsFound; nKernelName++)
        {
            QString currentName = functionsFound.at(nKernelName);
            int     currentLineNumber = functionsFoundLineNumber.at(nKernelName);
            bool found = false;

            for (int nKernel = 0; nKernel < numKernels && !found; nKernel++)
            {
                QString currentKernelName = getKernelData(nKernel, pCurrentFileData).m_kernelName;

                if (functionsFound.at(nKernelName) == currentKernelName)
                {
                    found = true;
                }
            }

            if (!found)
            {
                kaProjectDataManagerAnaylzeData newDataStruct;
                newDataStruct.m_kernelName = currentName;
                newDataStruct.m_lineInSourceFile = currentLineNumber;
                pCurrentFileData->analyzeVector().push_back(newDataStruct);
            }
        }
    }
}

// ---------------------------------------------------------------------------
///TODO rewrite this code with real parser
bool kaProjectDataManager::buildEntryPointList(QString& text, kaSourceFile* pCurrentFileData)
{
    QString textWithoutComments;

    RemoveCommentsPreservingNewLines(text, textWithoutComments);


    text = textWithoutComments;

    bool retVal = false;

    gtVector<QString> entryPointsFound;
    gtVector<int> entryPointsFoundLineNumber;

    // look for {, then find the () before the { ignoring white spaces, then function name will be before the (). after that find the closing } and start over

    int pointer = text.indexOf("{");
    int textLen = text.count();

    while (pointer != -1 && pointer != textLen)
    {
        int functionStartPoint = pointer;

        // go back to prev char
        pointer--;

        while (pointer >= 0 && text.at(pointer).isSpace())
        {
            pointer--;
        }

        // if not ')' see if its case of special shader function - returnVal funcName (..) : XX
        if (pointer >= 0 && text.at(pointer) != ')')
        {
            int endParamsList = text.lastIndexOf(")", pointer);

            if (endParamsList != -1)
            {
                int tmpPointewr = endParamsList + 1;

                while (tmpPointewr < textLen && text.at(tmpPointewr).isSpace())
                {
                    tmpPointewr++;
                }

                // if yes set the point to the end of param list
                if (text.at(tmpPointewr) == ':')
                {
                    pointer = endParamsList;
                }
            }
        }

        // if its a ')' maybe its is the end of function params
        if (pointer >= 0 && text.at(pointer) == ')')
        {
            bool shaderHasParemeters = false;
            int paramEnd = pointer - 1;
            // go back to start of function params
            pointer = text.lastIndexOf("(", pointer);

            if (-1 != pointer)
            {
                int paramStart = pointer + 1;
                int functionNameEndPoint = pointer;

                // check if there is only one word in the parameters list. If there is it is probably not a valid
                // shader, it is something like the register function.
                QString paramString = text.mid(paramStart, paramEnd - paramStart + 1);
                QStringList paramStringList = paramString.split(AF_STR_SpaceA);
                // remove all none empty strings (if in the text we have "  text " then 4 strings are created in the list
                paramStringList.removeAll("");

                if (paramStringList.count() > 1)
                {
                    shaderHasParemeters = true;
                }

                //eliminate space characters between function name and left bracket
                if (text.at(pointer) == '(')
                {
                    pointer--;
                }

                while (pointer >= 0 && text.at(pointer).isSpace())
                {
                    pointer--;
                }

                // go back to function name, while not white space
                while (pointer >= 0 && text.at(pointer).isSpace() == false)
                {
                    pointer--;
                }

                // set function name start point
                int functionNameStartPoint = pointer + 1;

                // find closing bracket of function
                int numBrackets = 1;
                pointer = functionStartPoint + 1;

                while (numBrackets > 0 && pointer < textLen)
                {
                    if (text.at(pointer) == '{')
                    {
                        numBrackets++;
                    }
                    else if (text.at(pointer) == '}')
                    {
                        numBrackets--;
                    }

                    pointer++;
                }

                // get function name
                QString name = text.mid(functionNameStartPoint, functionNameEndPoint - functionNameStartPoint);

                //if cl shader ==> then it should always have parameters
                //skip "register" special keywords in naming
                if (!name.isEmpty() && name != "register" && (shaderHasParemeters == true || pCurrentFileData->IsCLShader() == false))
                {
                    bool isSemanticKeyWord = false;

                    foreach (QRegExp rx, m_sKeywordsList)
                    {
                        isSemanticKeyWord = rx.exactMatch(name);

                        if (isSemanticKeyWord)
                        {
                            break;
                        }
                    }

                    if (!isSemanticKeyWord)
                    {
                        // add name and line number to vectors
                        name = name.trimmed();
                        entryPointsFound.push_back(name);

                        QString textSoFar = text.left(functionNameEndPoint);
                        int numOfLines = textSoFar.count('\n') + 1; // add one line since there is one less \n then number of lines
                        entryPointsFoundLineNumber.push_back(numOfLines);

                        pointer = text.indexOf("{", pointer + 1);
                    }
                }
            }
        }
        else
        {
            // not a function - go to next '{'
            pointer = text.indexOf("{", functionStartPoint + 1);
        }

    }

    UpdateFunctionsList(entryPointsFound, entryPointsFoundLineNumber, pCurrentFileData);

    retVal = entryPointsFound.size() > 0;

    return retVal;
}

void kaProjectDataManager::RemoveCommentsPreservingNewLines(const QString& text, QString& textNoComments)const
{
    int curPos = 0;
    int textLen = text.count();

    while (curPos < textLen)
    {
        if (text.at(curPos) == '/')
        {
            if (curPos < textLen - 1)
            {
                if (text.at(curPos + 1) == '/')
                {
                    //we are at double slash comment start - remove it remaining new line
                    while ((curPos < textLen - 1) && text.at(curPos) != '\n')
                    {
                        curPos++;
                    }

                    textNoComments += text.at(curPos);
                }
                else if (text.at(curPos + 1) == '*')
                {
                    //we are at slash star comment - remove /*...*/
                    while ((curPos < textLen - 1) && text.at(curPos) != '*' && text.at(curPos + 1) != '/')
                    {
                        curPos++;
                    }

                    //remove */ also
                    if (curPos < textLen - 2)
                    {
                        curPos += 2;
                        textNoComments += text.at(curPos);
                    }
                }
            }
        }
        else
        {
            // code without comments = add each character
            textNoComments += text.at(curPos);
        }

        curPos++;
    }
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::RemoveBlockComments(QString& text)
{
    // find pair of /* and */ and remove the block of text. This assume there is a pair
    // If there isn't then do nothing and probably the .cl is not valid and the parsing will be faulty:
    int commentStart, commentEnd;
    bool invalidCommant = false;

    while ((commentStart = text.indexOf("/*")) != -1 && !invalidCommant)
    {
        commentEnd = text.indexOf("*/", commentStart);

        if (commentEnd != -1)
        {
            // first count how many '\n' are in that section, they are needed because they are used to count the
            // line number of the kernel so the block will be replaced with the same number of \n:
            QString blockSection = text.mid(commentStart, commentEnd - commentStart + 2);
            int numOfNewLine = blockSection.count('\n');

            // Remove the comment block:
            text.remove(commentStart, commentEnd - commentStart + 2);

            // insert the new lines:
            for (int newLineCount = 0; newLineCount < numOfNewLine; newLineCount++)
            {
                text.insert(commentStart, '\n');
            }
        }
        else
        {
            // there is no matching ending comment:
            invalidCommant = true;
        }
    }
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::RemoveLineComments(QString& text)
{
    // find pair of // and \n (or end of doc) and remove the block of text:
    int commentStart, commentEnd;

    while ((commentStart = text.indexOf("//")) != -1)
    {
        commentEnd = text.indexOf('\n', commentStart);

        if (commentEnd != -1)
        {
            // Remove between the two segments:
            text.remove(commentStart, commentEnd - commentStart + 1);
        }
        else
        {
            // Remove from the comment start until the end of the text, making sure by giving the text length:
            text.remove(commentStart, text.length());
        }

        // Add a new line to keep the same number of lines:
        text.insert(commentStart, '\n');
    }

}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::IsTemplateKernel
// Description: check- is this is a template kernel- need to skip
// Arguments:   const QString& text - the cl text
//              int & kernelNameStartingPosition - the starting position to check from
// Return Val:  bool- true is it is a template kernel
// Author:      Danana
// Date:        11/03/2013
// ---------------------------------------------------------------------------
bool kaProjectDataManager::IsTemplateKernel(int& kernelNameStartingPosition, int kernelWordWidth, const QString& text, gtVector<QString>& templatesFound, gtVector<int>& templatesFoundLineNumber)
{
    bool bRet = false;
    // look for this before the kernel "template <class T>"
    int iCloseBracket = text.lastIndexOf(">", kernelNameStartingPosition);

    if (-1 != iCloseBracket)
    {
        int iClass = text.lastIndexOf("class", iCloseBracket);

        if ((-1 != iClass) && (iCloseBracket > iClass))
        {
            int iOpenBracket = text.lastIndexOf("<", iClass);

            if ((-1 != iOpenBracket) && (iClass > iOpenBracket))
            {
                int iTemplate = text.lastIndexOf("template", iOpenBracket);

                if ((-1 != iTemplate) && (iOpenBracket > iTemplate))
                {
                    // to be sure, check that there are only white spaces between the ">" and the "kernel"
                    for (iCloseBracket++; iCloseBracket < kernelNameStartingPosition; iCloseBracket++)
                    {
                        QChar temp = text.at(iCloseBracket);

                        if (isNotWhiteSpace(temp))
                        {
                            break;
                        }
                    }

                    if (iCloseBracket == kernelNameStartingPosition - kernelWordWidth)
                    {
                        bRet = true;
                    }
                }
            }
        }
    }

    if (bRet)
    {
        // find the "void" and after it the kernel name until the opening parenthesis of the function parameters
        int iOpenParenthesis = text.indexOf("(", kernelNameStartingPosition);

        if (-1 != iOpenParenthesis)
        {
            int iVoidPos = text.lastIndexOf(" void ", iOpenParenthesis);

            if ((-1 != iVoidPos) && (iVoidPos >= kernelNameStartingPosition))
            {
                // The name is from the end of the "void" ignoring white spaces
                QString templateName = text.mid(iVoidPos + 6, iOpenParenthesis - iVoidPos - 6);
                // remove leading spaces and trailing spaces:
                templateName.remove(QRegExp(" \n\r\t"));
                // find the start of the function
                int iFunctionStart = text.indexOf("{", iOpenParenthesis);

                if (-1 != iFunctionStart)
                {
                    if (!templateName.isEmpty())
                    {
                        // check that the kernel name is not already in the list:
                        if (std::find(templatesFound.begin(), templatesFound.end(), templateName) == templatesFound.end())
                        {
                            templatesFound.push_back(templateName);
                            // Get the line number of the kernel:
                            QString textSoFar = text.left(kernelNameStartingPosition);
                            int numOfLines = textSoFar.count('\n') + 1; // add one line since there is one less \n then number of lines
                            templatesFoundLineNumber.push_back(numOfLines);
                        }
                    }
                }
                else
                {
                    // Start of function was not found so it is not a kernel
                    bRet = false;
                }
            }
            else
            {
                // definition after the kernel word were not correct so this might not be a definition of kernel after all
                bRet = false;
            }
        }
    }

    return bRet;

}

// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::IsTemplateAttributeKernel
// Description: If this is a kernel that uses a template- take the real name
// Arguments:   const QString& text - the cl text
//              int& kernelNameStartingPosition - start position to look in text
//              const QString& text - text to look in
//              gtVector<QString>& kernelsFound - list of kernels found to add the kernel name
// Return Val:  bool - true is it is
// Author:      Danana
// Date:        03/11/2013
// ---------------------------------------------------------------------------
bool kaProjectDataManager::IsTemplateAttributeKernel(int& kernelNameStartingPosition, int kernelWordWidth, const QString& text, gtVector<QString>& kernelsFound, gtVector<QString>& templateToKernelVector, gtVector<int>& kernelsFoundLineNumber)
{
    bool bRet = false;
    int iNameBracketOpen = -1;
    int iNameBracketClose = -1;

    // look for this before the kernel "template __attribute__((mangled_name(mmmKernelFloat4))) "
    int iTemplateCloseBracket = text.lastIndexOf(")", kernelNameStartingPosition);

    if (-1 != iTemplateCloseBracket)
    {
        int iAttCloseBracket = text.lastIndexOf(")", iTemplateCloseBracket - 1);

        if (-1 != iAttCloseBracket)
        {
            iNameBracketClose = text.lastIndexOf(")", iAttCloseBracket - 1);

            if (-1 != iNameBracketClose)
            {
                iNameBracketOpen = text.lastIndexOf("(", iNameBracketClose - 1);

                if (-1 != iNameBracketOpen)
                {
                    int iAttOpenBracket = text.lastIndexOf("(", iNameBracketOpen - 1);

                    if (iAttOpenBracket != -1)
                    {
                        int iTemplateOpenBracket = text.lastIndexOf("(", iAttOpenBracket - 1);

                        if (iTemplateOpenBracket != -1)
                        {
                            int iAttText = text.lastIndexOf("attribute", iTemplateOpenBracket - 1);
                            {
                                if (-1 != iAttText)
                                {
                                    int iTemplateText = text.lastIndexOf("template", iAttText - 1);

                                    if (-1 != iTemplateText)
                                    {
                                        // to be sure, check that there are only white spaces between the ">" and the "kernel"
                                        for (iTemplateCloseBracket++; iTemplateCloseBracket < kernelNameStartingPosition; iTemplateCloseBracket++)
                                        {
                                            QChar temp = text.at(iTemplateCloseBracket);

                                            if (isNotWhiteSpace(temp))
                                            {
                                                break;
                                            }
                                        }

                                        if (iTemplateCloseBracket == kernelNameStartingPosition - kernelWordWidth)
                                        {
                                            bRet = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // let's find the kernel name
    if (bRet)
    {
        // Add the kernel name to the list of kernel found:
        QString kernelName;

        for (int nChar = iNameBracketOpen + 1; nChar < iNameBracketClose; nChar++)
        {
            QChar aChar = text.at(nChar);

            if ((aChar != 9) && (aChar != 10) && (aChar != 13) && (aChar != 32))
            {
                kernelName += aChar;
            }
        }

        if (!kernelName.isEmpty())
        {
            QString templateName;

            // Get the template name of the kernel:
            int iOpenParenthesis = text.indexOf("(", kernelNameStartingPosition);

            if (-1 != iOpenParenthesis)
            {
                int iVoidPos = text.lastIndexOf(" void ", iOpenParenthesis);

                if ((-1 != iVoidPos) && (iVoidPos >= kernelNameStartingPosition))
                {
                    // The name is from the end of the "void" ignoring white spaces
                    templateName = text.mid(iVoidPos + 6, iOpenParenthesis - iVoidPos - 6);
                    // remove leading spaces and trailing spaces:
                    templateName.remove(QRegExp(" \n\r\t"));
                }
            }

            // check that the kernel name is not already in the list:
            if (std::find(kernelsFound.begin(), kernelsFound.end(), kernelName) == kernelsFound.end() && !templateName.isEmpty())
            {
                kernelsFound.push_back(kernelName);
                templateToKernelVector.push_back(templateName);
                // The line number is still unknown at this time since the template line number is still unknown and will be set later:
                kernelsFoundLineNumber.push_back(-1);
            }
        }
    }

    return bRet;

}


// ---------------------------------------------------------------------------
// Name:        kaProjectDataManager::findKernelName
// Description: Find kernel name in text from starting position and add to kernelsFound
// Arguments:   int& kernelNameStartingPosition - start position to look in text
//              const QString& text - text to look in
//              gtVector<QString>& kernelsFound - list of kernels found to add the kernel name
// Author:      Gilad Yarnitzky
// Date:        5/6/2013
// ---------------------------------------------------------------------------
void kaProjectDataManager::findKernelName(int& kernelNameStartingPosition, const QString& text, gtVector<QString>& kernelsFound, gtVector<QString>& templateToKernelVector, gtVector<int>& kernelsFoundLineNumber)
{
    // 2: Look for the "{"
    int startOfFunction = text.indexOf("{", kernelNameStartingPosition + 1);

    if (startOfFunction != -1)
    {
        // 3: look backward for the ")"
        int endOfParameters = text.lastIndexOf(")", startOfFunction);
        // make sure there are no letters between ")" and the "{"
        bool foundCharacters = false;

        if (endOfParameters != -1)
        {
            for (int nChar = endOfParameters; nChar < startOfFunction && !foundCharacters; nChar++)
            {
                if (text.at(nChar).isLetterOrNumber())
                {
                    foundCharacters = true;
                }
            }
        }

        // 4: look backward for the "("
        int startOfParameters = text.lastIndexOf("(", startOfFunction);
        int aPosition = startOfParameters - 1;

        if (startOfParameters != -1 && !foundCharacters)
        {
            // 5: look for the function name
            QString kernelName;
            bool lookingForFunctionName = true;
            bool keepLooking = true;

            do
            {
                // If this is a space it is either before the function name and we should continue looking
                // or when we are in a function name and that is the end:
                QChar currentChar = text.at(aPosition);

                if (currentChar.isSpace())
                {
                    if (lookingForFunctionName)
                    {
                        aPosition--;
                    }
                    else
                    {
                        keepLooking = false;
                    }
                }
                // add the letter to the kernel name:
                else
                {
                    lookingForFunctionName = false;
                    kernelName.prepend(currentChar);
                    aPosition--;
                }
            }
            while (keepLooking);

            // Kernel function definition is in the following format:
            // __kernel void box_filter(__global uint4* inputImage, __global uchar4* outputImage, int N)
            // look for the word "void" before the function name and after the "kernel" word
            int voidPos = text.lastIndexOf("void", aPosition);
            bool validName = false;

            if (voidPos != -1 || voidPos > kernelNameStartingPosition)
            {
                // make sure there are only white spaces and after:
                // before the void we can have something like this: __attribute__((reqd_work_group_size(256,1,1)))
                if (isOnlyWhiteSpaces(voidPos + 4, aPosition, text))
                {
                    validName = true;
                }
            }

            if (!kernelName.isEmpty() && validName)
            {
                // check that the kernel name is not already in the list:
                if (std::find(kernelsFound.begin(), kernelsFound.end(), kernelName) == kernelsFound.end())
                {
                    kernelsFound.push_back(kernelName);
                    // Get the line number of the kernel:
                    QString textSoFar = text.left(kernelNameStartingPosition);
                    int numOfLines = textSoFar.count('\n') + 1; // add one line since there is one less \n then number of lines
                    kernelsFoundLineNumber.push_back(numOfLines);

                    // no template to kernel for a normal kernel, just enter an empty name to keep all three vectors aligned
                    templateToKernelVector.push_back("");
                }
            }
        }
    }

    // Move to next character so same kernel won't be found again
    kernelNameStartingPosition += 1;
}

// ---------------------------------------------------------------------------
bool kaProjectDataManager::isOnlyWhiteSpaces(int start, int end, const QString& text)
{
    bool retVal = true;
    QChar currentChar;

    for (int nChar = start; nChar < end && retVal; nChar++)
    {
        currentChar = text.at(nChar);

        if (isNotWhiteSpace(currentChar))
        {
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::setBuildOptions(const QString& buildOptions)
{
    kaProgram*  pActiveProgram = GetActiveProgram();

    if (pActiveProgram != nullptr && pActiveProgram->GetBuildType() == kaProgramDX)
    {
        SetShaderBuildOptions(buildOptions);
    }
    else
    {
        SetKernelBuildOptions(buildOptions);
    }

    emit BuildOptionsChanged();
}

void kaProjectDataManager::SetKernelBuildOptions(const QString& buildOptions)
{
    if (m_buildCommandOptions != buildOptions)
    {
        m_buildCommandOptions = buildOptions;
    }

    // if Visual studio extension - update toolbar
    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afApplicationCommands::instance()->updateToolbarCommands();
    }
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderBuildOptions(const QString& buildOptions)
{
    m_shaderBuildCommandOptions = buildOptions;

    // if Visual studio extension - update toolbar
    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afApplicationCommands::instance()->updateToolbarCommands();
    }

    emit ShaderBuildOptionsChanged();
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderAvoidableBuildOptions(const QStringList& avoidableStrings)
{
    m_avoidableBuildOptions.clear();
    m_avoidableBuildOptions = avoidableStrings;
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderCompileType(const QString& compileType)
{
    m_shaderCompileType = compileType;

    emit ShaderCompileTypeChanged();
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderD3dBuilderPath(const QString& shaderBuilderPath)
{
    m_shaderD3dBuilderPath = shaderBuilderPath;
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderFxcBuilderPath(const QString& shaderBuilderPath)
{
    m_shaderFxcBuilderPath = shaderBuilderPath;
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderMacros(const QString& shaderMacros)
{
    m_shaderMacros = shaderMacros;
}

void kaProjectDataManager::SetKernelMacros(const QString& kernelMacros)
{
    m_kernelMacros = kernelMacros;
}
// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderIncludes(const QString& shaderIncludes)
{
    m_shaderIncludes = shaderIncludes;
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::SetShaderD3dBuildOptionsMask(const unsigned int shaderD3dBuildOptionsMask)
{
    m_shaderD3dBuildOptionsMask = shaderD3dBuildOptionsMask;
}

// ---------------------------------------------------------------------------
void kaProjectDataManager::UpdateDocument(const osFilePath& docToUpdate)
{
    BuildFunctionsList(docToUpdate, NULL);

    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        kaAppWrapper::buildToolbar()->updateUIonMDI(docToUpdate, true);
    }
}


kaPlatform ProgramTypeToPlatformType(const kaProgramTypes progamType)
{
    kaPlatform retVal = kaPlatformUnknown;

    switch (progamType)
    {
        case kaProgramCL:
            retVal = kaPlatformOpenCL;
            break;

        case kaProgramGL_Compute:
        case kaProgramGL_Rendering:
            retVal = kaPlatformOpenGL;
            break;

        case kaProgramVK_Compute:
        case kaProgramVK_Rendering:
            retVal = kaPlatformVulkan;
            break;

        case kaProgramDX:
            retVal = kaPlatformDirectX;
            break;

        default:
            retVal = kaPlatformUnknown;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
kaPlatform kaProjectDataManager::GetBuildPlatform(const gtString& programName)
{
    kaPlatform retVal = kaPlatformUnknown;
    kaProgram* program = GetProgram(programName);

    if (nullptr != program)
    {
        const kaProgramTypes progamType = program->GetBuildType();

        retVal = ProgramTypeToPlatformType(progamType);

    }

    return retVal;
}

kaPlatform kaProjectDataManager::GetPlatformByExtension(const osFilePath& filePath) const
{
    gtString fileExt;
    kaPlatform ret = kaPlatformOpenCL;
    bool isGLShader, isDXShader;
    isGLShader = isDXShader = false;
    filePath.getFileExtension(fileExt);
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    if (m_dxShaderExtensions.contains(acGTStringToQString(fileExt), Qt::CaseInsensitive))
    {
        ret = kaPlatformDirectX;
        isDXShader = true;
    }

#endif

    if (m_glShaderExtensions.contains(acGTStringToQString(fileExt), Qt::CaseInsensitive))
    {
        ret = kaPlatformOpenGL;
        isGLShader = true;
    }

    // both DX and GL shaders may have .vs extension - check file name for more info
    if (isGLShader && isDXShader)
    {
        ret = GetPlatformByFileName(filePath);
    }

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)

    if (fileExt != AF_STR_clSourceFileExtension)
    {
        ret = kaPlatformOpenGL;
    }

#endif
    return ret;
}

kaPlatform kaProjectDataManager::GetPlatformByFileName(const osFilePath& filePath) const
{
    kaPlatform platform = kaPlatformDirectX;
    gtString fileName;

    bool res = filePath.getFileName(fileName);
    GT_ASSERT(res);

    gtString lowerCaseFileName = fileName.toLowerCase();

    if (lowerCaseFileName.find(KA_STR_GLIndicator) != -1)
    {
        platform = kaPlatformOpenGL;
    }

    return platform;
}


void kaProjectDataManager::DeserializeProgram(TiXmlNode* pMainNode)
{
    for (TiXmlElement* pNode = pMainNode->FirstChildElement(); pNode != nullptr; pNode = pNode->NextSiblingElement())
    {
        gtString kaProgramSectionNodeTitle;
        kaProgramSectionNodeTitle.fromASCIIString(pNode->Value());

        if (KA_STR_programSection == kaProgramSectionNodeTitle) //programs
        {

            for (TiXmlElement* pProgramNode = pNode->FirstChildElement();
                 pProgramNode != nullptr;
                 pProgramNode = pProgramNode->NextSiblingElement())
            {
                gtString kaProgramTitle;
                kaProgramTitle.fromASCIIString(pProgramNode->Value());

                //program deserialize
                if (KA_STR_program == kaProgramTitle)
                {
                    //get program type and name , use factory to create instance,
                    //deserialize rest of members by using kaProgram:Deserialize
                    gtString programName, programTypeString;
                    afUtils::getFieldFromXML(*pProgramNode, KA_STR_programName, programName);
                    afUtils::getFieldFromXML(*pProgramNode, KA_STR_programType, programTypeString);

                    // Get the program type as enum
                    kaProgramTypes progType = kaProgramUnknown;
                    bool rc = kaProgram::GetProgramTypeFromString(programTypeString, progType);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Create the program
                        kaProgram* pProgram = kaProgramFactory::Create(progType, programName);

                        // Sanity check:
                        GT_IF_WITH_ASSERT(pProgram != nullptr)
                        {
                            pProgram->DeSerialize(pProgramNode);
                            m_programsList.push_back(pProgram);

                            // Add the program to the tree
                            if (kaApplicationTreeHandler::instance() != nullptr)
                            {
                                kaApplicationTreeHandler::instance()->AddProgram(false, pProgram);
                            }
                        }
                    }
                }
            }

            //finished to parse programs
            break;
        }
    }
}


bool kaProjectDataManager::IsRender(const kaProgram* pProgram) const
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(nullptr != pProgram)
    {
        const kaProgramTypes buildType = pProgram->GetBuildType();
        retVal = buildType == kaProgramGL_Rendering || buildType == kaProgramVK_Rendering;
    }
    return retVal;
}

bool kaProjectDataManager::IsActiveProgramPipeLine() const
{
    const bool result = IsProgramPipeLine(instance().GetActiveProgram());
    return result;

}

bool kaProjectDataManager::IsProgramPipeLine(const kaProgram* program) const
{
    bool result = false;

    if (program != nullptr)
    {
        const kaProgramTypes  buildType = program->GetBuildType();
        result = buildType == kaProgramGL_Rendering || buildType == kaProgramGL_Compute || buildType == kaProgramVK_Rendering || buildType == kaProgramVK_Compute;
    }

    return result;
}
bool kaProjectDataManager::IsActiveProgramRender() const
{
    //ugly way to not doing const cast....
    const kaProgram* pActiveProgram = instance().GetActiveProgram();
    bool result = IsRender(pActiveProgram);
    return result;
}

bool kaProjectDataManager::IsProgramGL(const kaProgram* program) const
{
    bool isGlProgram = program != nullptr && (program->GetBuildType() == kaProgramGL_Compute || program->GetBuildType() == kaProgramGL_Rendering);
    return isGlProgram;
}
bool kaProjectDataManager::IsActiveProgramGL() const
{
    //ugly way to not doing const cast....
    const kaProgram* pActiveProgram = instance().GetActiveProgram();
    bool isGlProgram = IsProgramGL(pActiveProgram);
    return isGlProgram;
}

kaProgram* kaProjectDataManager::GetProgram(const gtString& programName) const
{
    kaProgram* result = nullptr;
    const auto& itr = find_if(m_programsList.begin(), m_programsList.end(), [&](const kaProgram * program)
    {
        return  program != nullptr && program->GetProgramName() == programName;
    });

    if (itr != m_programsList.end())
    {
        result = *itr;
    }

    return result;
}

void kaProjectDataManager::GetActiveProgramBuildFiles(const AnalyzerBuildArchitecture buildArch, FPathToOutFPathsMap& result)
{
    gtList<osFilePath> resultFilePaths;
    const kaProgram* pActiveProgram = GetActiveProgram();
    GT_IF_WITH_ASSERT(pActiveProgram != nullptr)
    {
        const gtVector<int> fileIds = pActiveProgram->GetFileIDsVector();
        bool isGlProgram = pActiveProgram->GetBuildType() == kaProgramGL_Rendering || pActiveProgram->GetBuildType() == kaProgramGL_Compute;
        osFilePath filePath;

        for (int id : fileIds)
        {
            filePath.clear();

            if (id < 0)
            {
                continue;
            }

            GetFilePathByID(id, filePath);
            GetActiveProgramBuildFiles(filePath, buildArch, result);

            //we extract only one build file for GL programs
            if (result.empty() == false && isGlProgram)
            {
                break;
            }
        }
    }

}

void kaProjectDataManager::GetActiveProgramBuildFiles(const osFilePath& filePath, const AnalyzerBuildArchitecture buildArch, FPathToOutFPathsMap& result)
{
    gtString fileNameFilter;
    kaProgram* pActiveProgram = GetActiveProgram();
    GT_IF_WITH_ASSERT(pActiveProgram != nullptr)
    {
        if (IsActiveProgramPipeLine())
        {
            //only for vulkan we filter files on disk by stage name
            if (pActiveProgram->GetBuildType() == kaProgramVK_Compute || pActiveProgram->GetBuildType() == kaProgramVK_Rendering)
            {
                kaPipelinedProgram* pProgram = dynamic_cast<kaPipelinedProgram*>(GetActiveProgram());
                GT_ASSERT(pProgram != nullptr);
                const int  fielID = GetFileID(filePath);
                kaPipelinedProgram::PipelinedStage stage = pProgram->GetFileRenderingStage(fielID);
                fileNameFilter = pProgram->GetRenderingStageAsString(stage);
            }
        }
        else
        {
            filePath.getFileName(fileNameFilter);
        }

        fileNameFilter.prepend(L"*").append(L"*." KA_STR_kernelViewBinExtension);

        osDirectory out32Dir, out64Dir;
        pActiveProgram->GetAndCreateOutputDirectories(out32Dir, out64Dir, false, false);
        osDirectory outDir = (kaBuildArch32_bit == buildArch) ? out32Dir : out64Dir;
        gtList<osFilePath> resultFilePaths;

        GT_ASSERT(outDir.getContainedFilePaths(fileNameFilter, osDirectory::SORT_BY_DATE_DESCENDING, resultFilePaths));

        if (resultFilePaths.empty() == false)
        {
            result[filePath] = resultFilePaths;
        }
    }

}

void kaProjectDataManager::RemoveProgram(kaProgram* pProgram, bool shouldRemoveBinariesFromDisk)
{
    GT_IF_WITH_ASSERT(pProgram != nullptr)
    {
        osFilePath programPath = kaApplicationCommands::instance().OutputFilePathForCurrentProject();
        programPath.appendSubDirectory(pProgram->GetProgramName());
        osDirectory programDir(programPath);

        if (shouldRemoveBinariesFromDisk)
        {
            osDirectory output32, output64;
            pProgram->GetAndCreateOutputDirectories(output32, output64, false, false);

            if (output32.exists())
            {
                output32.deleteRecursively();
            }

            if (output64.exists())
            {
                output64.deleteRecursively();
            }

            for (int i = 0; i < (int)m_programsList.size(); i++)
            {
                if (pProgram == m_programsList[i])
                {
                    m_programsList.removeItem(i);
                    break;
                }
            }
        }

        if (programDir.exists())
        {
            programDir.deleteRecursively();
        }

        m_sourceFileManager->RemoveRecipient(pProgram);
    }
}

bool kaProjectDataManager::RenameFile(const osFilePath& oldFilePath, const osFilePath& newFilePath, const osFilePath& newDirPath)
{
    bool retVal = false;

    kaSourceFile* pFile = m_sourceFileManager->GetFile(oldFilePath);

    if (pFile != nullptr)
    {
        pFile->setFilePath(newFilePath);
        pFile->setBuildDirectory(osDirectory(newDirPath));
        retVal = true;
    }

    return retVal;
}

void kaProjectDataManager::OnDocumentSaved(QString filePathAsStr)
{
    gtString fileChanged(acQStringToGTString(filePathAsStr));
    // check if this is a file of interest update the
    kaSourceFile* pCurrentFile = dataFileByPath(fileChanged);

    if (pCurrentFile != nullptr)
    {
        osFilePath changeFilePath(fileChanged);
        BuildFunctionsList(changeFilePath, pCurrentFile);

        kaBuildToolbar* pKAToolBar = kaAppWrapper::instance().buildToolbar();

        if (pKAToolBar != nullptr)
        {
            pKAToolBar->RebuildKernelsAndEntryList(pCurrentFile, true);
        }
    }
}

/// Set current file information
void kaProjectDataManager::SetCurrentFileData(kaFileInformation fileInfoType, gtString& fileInfo)
{
    kaApplicationTreeHandler* pTreeHanlder = kaApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pTreeHanlder != nullptr)
    {
        osFilePath filePath;

        if (pTreeHanlder->GetActiveItemFilePath(filePath))
        {
            // update the files list
            kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(filePath);

            if (nullptr != pCurrentFile)
            {
                switch (fileInfoType)
                {
                    case kaFilePlatform:
                        pCurrentFile->SetBuildPlatform(fileInfo);
                        break;

                    //    case kaFileArchitecture:
                    //   pCurrentFile->SetBuildArchitechture(fileInfo);
                    //      break;

                    case kaFileDXShaderProfile:
                        pCurrentFile->setBuildProfile(fileInfo);
                        break;

                    case kaFileEntryPoint:
                        pCurrentFile->SetEntryPointFunction(fileInfo);
                        break;

                    case kaFileGLShaderType:
                        pCurrentFile->SetGLShaderType(fileInfo);
                        break;
                }
            }
        }
    }
}

/// Get the file Info
gtString kaProjectDataManager::CurrentFileInfo(kaFileInformation fileInfoType) const
{
    gtString retStr;

    kaApplicationTreeHandler* pTreeHanlder = kaApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pTreeHanlder != nullptr)
    {
        osFilePath filePath;

        if (pTreeHanlder->GetActiveItemFilePath(filePath))
        {
            // update the files list
            kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(filePath);

            if (nullptr != pCurrentFile)
            {
                switch (fileInfoType)
                {
                    case kaFilePlatform:
                        retStr = pCurrentFile->BuildPlatform();
                        break;

                    case kaFileDXShaderProfile:
                        retStr = pCurrentFile->BuildProfile();
                        break;

                    case kaFileEntryPoint:
                        retStr = pCurrentFile->EntryPointFunction();
                        break;

                    case kaFileGLShaderType:
                        retStr = pCurrentFile->GetGLShaderType();
                        break;
                }
            }
        }
    }

    return retStr;
}
//--------------------------------------------------------------------------


bool kaProjectDataManager::IsBuilt(const kaProgram* pProgram, const int fileId, const AnalyzerBuildArchitecture buildArch) const
{
    GT_ASSERT(pProgram != nullptr);
    osDirectory out32Dir, out64Dir;
    pProgram->GetAndCreateOutputDirectories(out32Dir, out64Dir, false, false);
    osDirectory outDir = kaBuildArch32_bit == buildArch ? out32Dir : out64Dir;
    osFilePath filePath;
    GetFilePathByID(fileId, filePath);

    //build filter for binary output file
    gtString binFileNameFilter;

    if (false == IsRender(pProgram))
    {
        filePath.getFileName(binFileNameFilter);
    }
    //get CLI stage name
    else
    {
        const kaRenderingProgram* pRenderProgram = dynamic_cast<const kaRenderingProgram*>(pProgram);
        GT_ASSERT(pRenderProgram != nullptr);
        kaPipelinedProgram::PipelinedStage renderStage = pRenderProgram->GetFileRenderingStage(fileId);
        binFileNameFilter = kaRenderingProgram::GetRenderingStageAsCLIString(renderStage);
    }

    binFileNameFilter.prepend(L"*").append(L"*." KA_STR_buildMainBinaryFileName);

    //find this file
    gtList<osFilePath> filePaths;
    outDir.getContainedFilePaths(binFileNameFilter, osDirectory::SORT_BY_DATE_ASCENDING, filePaths);

    return filePaths.empty() == false;
}

bool kaProjectDataManager::IsBuilt(const kaProgram* pProgram, const AnalyzerBuildArchitecture buildArch) const
{
    bool bRes = false;
    GT_ASSERT(pProgram != nullptr);
    osDirectory out32Dir, out64Dir;
    pProgram->GetAndCreateOutputDirectories(out32Dir, out64Dir, false, false);
    osDirectory outDir = kaBuildArch32_bit == buildArch ? out32Dir : out64Dir;

    const gtVector<int>& fileIDsVector = pProgram->GetFileIDsVector();

    //program build if it has any files within it
    if (fileIDsVector.empty() == false)
    {
        gtList<osFilePath> filePaths;

        gtString filter = L"*.";
        kaProgramTypes programBuildType = pProgram->GetBuildType();
        filter.append((programBuildType == kaProgramVK_Compute || programBuildType == kaProgramVK_Rendering) ? KA_STR_kernelViewILExtension : KA_STR_buildMainBinaryFileName);

        //get list of files from output folder
        if (outDir.getContainedFilePaths(filter, osDirectory::SORT_BY_DATE_ASCENDING, filePaths) &&
            filePaths.empty() == false)
        {
            //if it's a gl program we need at one bin file
            if (IsProgramGL(pProgram))
            {
                bRes = true;
            }
            else
            {
                //run on all files and check if they have il files in folder
                for (const int id : fileIDsVector)
                {

                    gtString fileName;

                    //if not rendering program find by file name
                    if (false == IsProgramPipeLine(pProgram))
                    {
                        osFilePath filePath;
                        GetFilePathByID(id, filePath);
                        filePath.getFileName(fileName);
                    }

                    //check if file with current id is contain in filePaths
                    auto itr = std::find_if(filePaths.begin(), filePaths.end(), [&](const  osFilePath & currentFilePath)
                    {
                        return static_cast<int>(string::npos) != currentFilePath.asString().find(fileName);
                    });

                    //if at least one source file not build we return false
                    if (false == (bRes = itr != filePaths.end()))
                    {
                        break;
                    }
                }//for
            }//else

        }
    }

    return bRes;
}

int kaProjectDataManager::GetFileID(const osFilePath& filePath) const
{
    // search for a file with passed filePath in a map
    int fileID = m_sourceFileManager->GetFileId(filePath);

    return fileID;
}

gtList<int> kaProjectDataManager::GetFileIDs(const gtVector<osFilePath>& filePaths) const
{
    gtList<int> result;

    for (const auto& filePath : filePaths)
    {
        int id = GetFileID(filePath);

        if (id >= 0)
        {
            result.push_back(id);
        }
    }

    return result;
}

void kaProjectDataManager::GetFilePathByID(int fileID, osFilePath& filePath) const
{
    m_sourceFileManager->GetFilePathByID(fileID, filePath);
}

std::set<osFilePath> kaProjectDataManager::GetFilePathsByIDs(const gtVector<int>& fileIDs) const
{
    set<osFilePath> filePathsSet;

    for (int id : fileIDs)
    {
        osFilePath filePath;
        KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(id, filePath);
        filePathsSet.insert(filePath);
    }

    return filePathsSet;
}


void kaProjectDataManager::Connect(int sourceFileId, kaProgram* program)
{
    m_sourceFileManager->Register(sourceFileId, program);
}


void kaProjectDataManager::Disconnect(int sourceFileId, kaProgram* program)
{
    m_sourceFileManager->Unregister(sourceFileId, program);
}

void kaProjectDataManager::SetProjectArchitecture(AnalyzerBuildArchitecture buildArch)
{
    m_buildArchitecture = buildArch;
}

void kaProjectDataManager::GetInitialEntryPointOrKernel(gtString& functionName)const
{
    // get selected file
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        //Get selected item data
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (pKAData != nullptr)
            {
                osFilePath activeDocPath(pKAData->filePath());
                kaSourceFile* pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(activeDocPath);
                pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(activeDocPath);
                GT_IF_WITH_ASSERT(pCurrentFile != nullptr)
                {
                    gtString fileFunction = pCurrentFile->EntryPointFunction();

                    if (fileFunction.isEmpty())
                    {
                        if (pCurrentFile->analyzeVector().size() > 0)
                        {
                            // get the first item from list and set the data manager too
                            functionName = acQStringToGTString(pCurrentFile->analyzeVector()[pCurrentFile->analyzeVector().size() - 1].m_kernelName);
                        }

                        pCurrentFile->SetEntryPointFunction(functionName);
                    }
                    else
                    {
                        functionName = fileFunction;
                    }

                    kaProgram* pProgram = pKAData->GetProgram();

                    if (pProgram != nullptr)
                    {
                        int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(activeDocPath);
                        gtString folderFunction;
                        kaProgramTypes programType = pProgram->GetBuildType();

                        if (programType == kaProgramDX)
                        {
                            kaDxFolder* pDXFolder = dynamic_cast<kaDxFolder*>(pProgram);

                            if (pDXFolder != nullptr)
                            {
                                // get the active file entry point/function name
                                pDXFolder->GetFileSelectedEntryPoint(fileId, folderFunction);

                                // If entry point/function name is empty
                                if (folderFunction.isEmpty())
                                {
                                    pDXFolder->SetFileSelectedEntryPoint(fileId, functionName);
                                }
                                else if (functionName != folderFunction)
                                {
                                    functionName = folderFunction;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void kaProjectDataManager::OpenSourceFileOnGivenLine(int lineNumber)const
{
    // get selected file
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        //Get selected item data
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (pKAData != nullptr)
            {
                kaApplicationCommands::instance().OpenSourceFile(pKAData, lineNumber, false);
            }
        }
    }

}

void kaProjectDataManager::GetSelectedDXShaderType(gtString& fileType)const
{
    gtString fileProfile;
    //Get active program
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        //Get selected item data
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (pKAData != nullptr)
            {
                int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(pKAData->filePath());
                kaDxFolder* pDXFolder = dynamic_cast<kaDxFolder*>(pKAData->GetProgram());

                if (pDXFolder != nullptr)
                {
                    pDXFolder->GetFileSelectedShaderType(fileId, fileType);

                    if (fileType.isEmpty())
                    {
                        gtString entryPoint;
                        GetInitialEntryPointOrKernel(entryPoint);

                        if (!entryPoint.isEmpty())
                        {
                            gtString profileCandidate = L"";
                            kaApplicationCommands::instance().DetectDXShaderProfileFromEntryPointName(acGTStringToQString(entryPoint), profileCandidate);

                            if (!profileCandidate.isEmpty())
                            {
                                gtString prevShaderModel = pDXFolder->GetProgramLevelShaderModel();
                                pDXFolder->SetFileProfile(fileId, profileCandidate);

                                // Detected shader profile sets latest model by default - change if needed
                                if (pDXFolder->GetProgramLevelShaderModel() != prevShaderModel)
                                {
                                    pDXFolder->SetProgramLevelShaderModel(prevShaderModel);
                                }

                                pDXFolder->GetFileSelectedShaderType(fileId, fileType);
                            }
                        }
                    }
                }
            }
        }
    }
}

void kaProjectDataManager::SetEntryPointOrKernel(const gtString& functionName, int& fileId, int& lineNumber, gtString& typeName)const
{
    QString functionNameAsQStr = acGTStringToQString(functionName);
    kaSourceFile* pCurrentFile = nullptr;
    // get selected file
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        //Get selected item data
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (pKAData != nullptr)
            {
                osFilePath activeDocPath(pKAData->filePath());
                pCurrentFile = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(activeDocPath);
                GT_IF_WITH_ASSERT(pCurrentFile != nullptr)
                {
                    fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(activeDocPath);
                    pCurrentFile->SetEntryPointFunction(functionName);
                    lineNumber = -1;
                    int numFunctions = pCurrentFile->analyzeVector().size();

                    for (int nFunction = 0; nFunction < numFunctions; nFunction++)
                    {
                        if (pCurrentFile->analyzeVector()[nFunction].m_kernelName == functionNameAsQStr)
                        {
                            lineNumber = pCurrentFile->analyzeVector()[nFunction].m_lineInSourceFile;
                            break;
                        }
                    }

                    kaProgram* pProgram = pKAData->GetProgram();

                    if (pProgram != nullptr)
                    {
                        if (pProgram->GetBuildType() == kaProgramDX)
                        {
                            kaDxFolder* pDXFolder = dynamic_cast<kaDxFolder*>(pProgram);

                            if (pDXFolder != nullptr)
                            {
                                pDXFolder->SetFileSelectedEntryPoint(fileId, functionName);
                            }

                            gtString profileCandidate = L"";
                            kaApplicationCommands::instance().DetectDXShaderProfileFromEntryPointName(acGTStringToQString(functionName), profileCandidate);

                            if (!profileCandidate.isEmpty())
                            {
                                gtString prevShaderModel = pDXFolder->GetProgramLevelShaderModel();
                                pDXFolder->SetFileProfile(fileId, profileCandidate);

                                // Detected shader profile sets latest model by default - change if needed
                                if (pDXFolder->GetProgramLevelShaderModel() != prevShaderModel)
                                {
                                    pDXFolder->SetProgramLevelShaderModel(prevShaderModel);
                                }

                                pDXFolder->GetFileSelectedShaderType(fileId, typeName);
                            }
                        }
                    }
                }
            }
        }
    }
}

/// Returns active program
kaProgram* kaProjectDataManager::GetActiveProgram()
{
    kaProgram* pRetVal = nullptr;

    if (kaApplicationTreeHandler::instance() != nullptr)
    {
        pRetVal = kaApplicationTreeHandler::instance()->GetActiveProgram();
    }

    return pRetVal;

}

void kaProjectDataManager::GetActiveProgramms(std::vector<kaProgram*>& programms)
{
    if (kaApplicationTreeHandler::instance() != nullptr)
    {
        kaApplicationTreeHandler::instance()->GetActiveProgramms(programms);
    }
}

void kaProjectDataManager::SetLastBuildProgram(kaProgram* pProgram)
{
    if (pProgram)
    {
         m_pLastBuildProgram[pProgram->GetProgramName().asCharArray()]= pProgram;
    }
}

const gtVector<osFilePath>& kaProjectDataManager::GetLastBuildFiles()const
{
    return m_lastBuildFiles;
}

void kaProjectDataManager::SetLastBuildFiles(const gtVector<osFilePath>& lastBuiltFiles)
{
    m_lastBuildFiles.clear();
    m_lastBuildFiles = lastBuiltFiles;
}

//--------------------------------------------------------------------

kaSourceFile::kaSourceFile() : m_buildPlatform(KA_STR_platformOpenCL_GT),
    m_fileType(kaFileTypeUnknown)
{
}

void kaSourceFile::setFilePath(const osFilePath& iFilePath)
{
    m_filePath = iFilePath;

    SetAtributesByName();
}

void kaSourceFile::SetAtributesByName()
{

    // Set the platform
    kaPlatform filePlatform = KA_PROJECT_DATA_MGR_INSTANCE.GetPlatformByExtension(m_filePath);

    switch (filePlatform)
    {
        case kaPlatformOpenGL:
            SetBuildPlatform(KA_STR_platformOpenGL_GT);
            {
                SetGLShaderTypeByFile();
            }
            break;

        case kaPlatformDirectX:
            SetBuildPlatform(KA_STR_platformDirectX_GT);
            break;

        default:
            SetBuildPlatform(KA_STR_platformOpenCL_GT);
            break;
    }

}

void kaSourceFile::SetGLShaderTypeByFile()
{
    gtString fileExt;
    gtString fileName;
    m_filePath.getFileExtension(fileExt);
    m_filePath.getFileName(fileName);
    fileName = fileName.toLowerCase();
    QString fileExtStr = acGTStringToQString(fileExt);

    const QStringList& glShaderTypes = KA_PROJECT_DATA_MGR_INSTANCE.GetGLShaderTypes();

    if (fileExtStr.compare(KA_STR_CommonGLShaderExtension, Qt::CaseInsensitive) == 0)
    {
        bool typeDetected = false;
        const gtVector<QStringList>& glShadersDetectorsVector = KA_PROJECT_DATA_MGR_INSTANCE.GetGLShaderDetectors();
        // if the extension is glsl try to detect type from file name

        for (int i = 0; i < (int)glShadersDetectorsVector.size(); i++)
        {
            DetectGLShaderTypeByKeywordsInFileName(acGTStringToQString(fileName), glShadersDetectorsVector[i], typeDetected);

            if (typeDetected)
            {
                SetGLShaderType(acQStringToGTString(glShaderTypes[i]));
                break;
            }
        }

        if (!typeDetected)
        {
            //Could not detect shader type - set Fragment by default
            SetGLShaderType(acQStringToGTString(glShaderTypes[FRAGMENT]));
            m_fileType = kaFileTypeGLSLFrag;
        }
    }
    else
    {
        int extentionIndex = KA_PROJECT_DATA_MGR_INSTANCE.GetGLShaderExtensions().indexOf(acGTStringToQString(fileExt).toLower());

        if (-1 != extentionIndex)
        {
            switch (extentionIndex)
            {
                case 0:
                case 1:
                    SetGLShaderType(acQStringToGTString(glShaderTypes[FRAGMENT]));
                    m_fileType = kaFileTypeGLSLFrag;
                    break;

                case 2:
                case 3:
                    SetGLShaderType(acQStringToGTString(glShaderTypes[VERTEX]));
                    m_fileType = kaFileTypeGLSLVert;
                    break;

                case 4:
                case 5:
                    SetGLShaderType(acQStringToGTString(glShaderTypes[COMPUTE]));
                    m_fileType = kaFileTypeGLSLComp;
                    break;

                case 6:
                case 7:
                    SetGLShaderType(acQStringToGTString(glShaderTypes[GEOMETRY]));
                    break;

                case 8:
                    SetGLShaderType(acQStringToGTString(glShaderTypes[TESSCONT]));
                    m_fileType = kaFileTypeGLSLTesc;
                    break;

                case 9:
                    SetGLShaderType(acQStringToGTString(glShaderTypes[TESSEVAL]));
                    m_fileType = kaFileTypeGLSLTese;
                    break;

                default:
                    SetGLShaderType(acQStringToGTString(glShaderTypes[FRAGMENT]));
                    m_fileType = kaFileTypeGLSLFrag;
                    break;
            }
        }
        else
        {
            // setting Fragment shader if file extension wasn't found in gl shader extensions list
            SetGLShaderType(acQStringToGTString(glShaderTypes[FRAGMENT]));
            m_fileType = kaFileTypeGLSLFrag;
        }
    }
}

void kaSourceFile::DetectGLShaderTypeByKeywordsInFileName(const QString& fileName, const QStringList& glShaderKeywords, bool& typeDetected)
{
    typeDetected = false;

    for (const QString& keyWord : glShaderKeywords)
    {
        if (fileName.contains(keyWord, Qt::CaseInsensitive))
        {
            typeDetected = true;
            break;
        }
    }
}

bool kaSourceFile::IsCLShader() const
{
    return m_fileType == kaFileTypeOpenCL;
}

void kaSourceFile::setBuildProfile(gtString buildProfile)
{
    m_buildProfile = buildProfile;

    // if Visual studio extension - update toolbar
    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afApplicationCommands::instance()->updateToolbarCommands();
    }
}

void kaSourceFile::SetEntryPointFunction(gtString entryPoint)
{
    if (entryPoint.length() > 1)
    {
        m_entryPointFunction = entryPoint;

        // if Visual studio extension - update toolbar
        if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            afApplicationCommands::instance()->updateToolbarCommands();
        }
    }
}

void kaSourceFile::SetGLShaderType(gtString glShaderType)
{
    m_glShaderType = glShaderType;

    // if Visual studio extension - update toolbar
    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afApplicationCommands::instance()->updateToolbarCommands();
    }
}

void kaSourceFile::SetBuildPlatform(const gtString& buildPlatform)
{
    m_buildPlatform = buildPlatform;

    if (m_buildPlatform == KA_STR_platformOpenGL_GT  && m_glShaderType.isEmpty())
    {
        SetGLShaderTypeByFile();
    }

    // if Visual studio extension - update toolbar
    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afApplicationCommands::instance()->updateToolbarCommands();
    }
}

void kaSourceFile::Serialize(gtString& xmlString)
{
    xmlString.appendFormattedString(L"<%ls>", KA_STR_projectSettingFilesInfoNode);

    afUtils::addFieldToXML(xmlString, KA_STR_sourceFileId, gtString(to_wstring(id()).c_str()));

    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingFilePathNode, filePath().asString());
    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingFileType, gtString(to_wstring(FileType()).c_str()));
    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingFileExecutionPathNode, buildDirectory().directoryPath().asString());

    QStringList& deviceListToSave = buildFiles();

    // join the string list to one long string to be saved:
    int numStrings = deviceListToSave.count();
    QString saveString;

    for (int nString = 0; nString < numStrings; nString++)
    {
        saveString += (deviceListToSave[nString] + ',');
    }

    gtString saveStringAsStr = acQStringToGTString(saveString);

    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingDevicesNode, saveStringAsStr);

    // set shader target and entry point
    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingShaderPlatform, BuildPlatform());
    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingShaderProfile, BuildProfile());
    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingShaderEntryPoint, EntryPointFunction());
    afUtils::addFieldToXML(xmlString, KA_STR_projectSettingShaderGLType, GetGLShaderType());

    xmlString.appendFormattedString(L"</%ls>", KA_STR_projectSettingFilesInfoNode);
}

void kaSourceFile::DeSerialize(TiXmlElement* pNode)
{
    GT_ASSERT(pNode != nullptr);
    gtString kaFileTitle;
    kaFileTitle.fromASCIIString(pNode->Value());

    if (KA_STR_projectSettingFilesInfoNode == kaFileTitle)
    {
        gtString filePath, directoryPath, devicesList, idStr;
        int fileType = kaFileTypeUnknown;
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingFilePathNode, filePath);
        afUtils::getFieldFromXML(*pNode, KA_STR_sourceFileId, idStr);
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingFileExecutionPathNode, directoryPath);
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingDevicesNode, devicesList);
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingFileType, fileType);

        m_fileType = static_cast<kaFileTypes>(fileType);
        GT_ASSERT(idStr.toIntNumber(m_id));

        // Add the data to the project data manager, the data manager update the tree
        osFilePath loadedFilePath;
        loadedFilePath.setFullPathFromString(filePath);
        setFilePath(loadedFilePath);

        osFilePath tempFilePath(directoryPath);
        tempFilePath.reinterpretAsDirectory();
        osDirectory loadedDirectory(tempFilePath);
        setBuildDirectory(loadedDirectory);

        QStringList deviceListLoaded = acGTStringToQString(devicesList).split(AF_STR_CommaA);
        // discard last string which is empty:
        deviceListLoaded.removeLast();
        setBuildFiles(deviceListLoaded);

        // Get the file information: Platform, Architecture, Target, Entrypoint, GLType
        gtString buildPlatform;
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingShaderPlatform, buildPlatform);
        SetBuildPlatform(buildPlatform);

        gtString buildTarget;
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingShaderProfile, buildTarget);
        setBuildProfile(buildTarget);

        gtString entryPoint;
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingShaderEntryPoint, entryPoint);
        SetEntryPointFunction(entryPoint);

        gtString glType;
        afUtils::getFieldFromXML(*pNode, KA_STR_projectSettingShaderGLType, glType);
        SetGLShaderType(glType);

        if (BuildPlatform().isEmpty())
        {
            SetAtributesByName();
        }
    }
}
