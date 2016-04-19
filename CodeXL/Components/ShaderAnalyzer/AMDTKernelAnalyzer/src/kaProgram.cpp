//------------------------------ kaProgram.cpp ------------------------------
// TinyXml:
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaProgram.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

const int PROFILE_CHARS_NUM = 6;

kaProgram::kaProgram() :
    m_programName(L""), m_buildType(kaProgramUnknown), m_isEmpty(true)
{

}

kaProgram::~kaProgram()
{

}


void kaProgram::SerializeProgramHeader(gtString& xmlString) const
{
    afUtils::addFieldToXML(xmlString, KA_STR_programName, m_programName);

    afUtils::addFieldToXML(xmlString, KA_STR_programType, kaProgram::GetProgramTypeAsString(m_buildType));
}

void kaProgram::Serialize(gtString& xmlString)
{
    SerializeProgramHeader(xmlString);

    // Open the source files section
    xmlString.appendFormattedString(L"<%ls>", KA_STR_sourceFilesSection);

    for (int i = 0; i < (int)m_fileIDsVector.size(); i++)
    {
        xmlString.appendFormattedString(L"<%ls>", KA_STR_sourceFile);
        afUtils::addFieldToXML(xmlString, KA_STR_sourceFileId, m_fileIDsVector[i]);
        xmlString.appendFormattedString(L"</%ls>", KA_STR_sourceFile);
    }

    // Close the source files section
    xmlString.appendFormattedString(L"</%ls>", KA_STR_sourceFilesSection);
}


void kaProgram::DeserializeProgramHeader(TiXmlElement* pProgramNode)
{
    // Get the program base properties
    gtString outputdir64, outputdir32, programType, buildBitness;
    afUtils::getFieldFromXML(*pProgramNode, KA_STR_programName, m_programName);

    afUtils::getFieldFromXML(*pProgramNode, KA_STR_programType, programType);

    bool rc = kaProgram::GetProgramTypeFromString(programType, m_buildType);
    GT_ASSERT(rc);
}

void kaProgram::DeSerialize(TiXmlElement* pProgramNode)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pProgramNode != nullptr)
    {
        DeserializeProgramHeader(pProgramNode);

        // Read the list of source files
        gtString sourceFilesTagName = KA_STR_sourceFilesSection;
        gtString sourceFileTagName = KA_STR_sourceFile;
        TiXmlElement* pSourceFilesElement = pProgramNode->FirstChildElement(sourceFilesTagName.asASCIICharArray());

        if (pSourceFilesElement != nullptr)
        {
            TiXmlElement* pSourceFileElement = pSourceFilesElement->FirstChildElement(sourceFileTagName.asASCIICharArray());

            for (; pSourceFileElement != nullptr; pSourceFileElement = pSourceFileElement->NextSiblingElement())
            {
                gtString fileStageStr;
                int fileId = -1;
                afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_sourceFileId, fileId);
                afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_sourceFileStage, fileStageStr);

                AddUniqueFileId(fileId);
            }
        }

    }
}

kaProgram* kaProgram::Clone() const
{
    return new kaProgram(*this);
}

void kaProgram::OnFileRemove(const int fileId)
{
    for (int i = 0; i < (int)m_fileIDsVector.size(); i++)
    {
        if (m_fileIDsVector[i] == fileId)
        {
            m_fileIDsVector.removeItem(i);

            if (m_fileIDsVector.empty())
            {
                m_isEmpty = true;
            }

            break;
        }
    }
}

void kaProgram::FilterByIds(const gtList<int>& ids)
{
    auto itr = m_fileIDsVector.begin();

    while (itr != m_fileIDsVector.end())
    {
        bool found = (std::find(ids.begin(), ids.end(), *itr) != ids.end());

        //erase id, since it's no in list
        if (found == false)
        {
            itr = m_fileIDsVector.erase(itr);

            if (m_fileIDsVector.empty() && !m_isEmpty)
            {
                m_isEmpty = true;

                if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
                {
                    afApplicationCommands::instance()->updateToolbarCommands();
                }
            }
        }
        else
        {
            ++itr;
        }
    }
}
bool kaProgram::HasFile(int fileId, afTreeItemType itemType) const
{
    GT_UNREFERENCED_PARAMETER(itemType);
    bool retVal = false;
    auto iter = gtFind(m_fileIDsVector.begin(), m_fileIDsVector.end(), fileId);
    retVal = iter != m_fileIDsVector.end();
    return retVal;
}

bool kaProgram::HasFile(const osFilePath& filePath, afTreeItemType itemType) const
{
    bool retVal = false;

    int fileId = KA_PROJECT_DATA_MGR_INSTANCE.GetFileID(filePath);
    retVal = HasFile(fileId, itemType);
    return retVal;
}

void kaProgram::SetProgramName(const gtString& name)
{
    m_programName = name;

    // By default the program's display name is the program's name.
    m_displayName = name;
}

void kaProgram::AddUniqueFileId(const int fileId)
{
    // Avoid multiple references in the files vector
    if (gtFind(m_fileIDsVector.begin(), m_fileIDsVector.end(), fileId) == m_fileIDsVector.end())
    {
        m_fileIDsVector.push_back(fileId);

        if (m_isEmpty)
        {
            m_isEmpty = false;

            if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
            {
                afApplicationCommands::instance()->updateToolbarCommands();
            }
        }
    }
}

void kaProgram::GetProgramFiles(gtVector<osFilePath>& filesPath)const
{
    osFilePath filePath;
    const gtVector<int>& idVector = GetFileIDsVector();

    for (int it : idVector)
    {
        KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(it, filePath);
        filesPath.push_back(filePath);
    }
}


kaRenderingProgram::kaRenderingProgram()
{
    // Initialize the file references vector with all the rendering stages
    for (int i = (int)kaRenderingProgram::KA_PIPELINE_STAGE_VERTEX; i <= kaRenderingProgram::KA_PIPELINE_STAGE_FRAG; i++)
    {
        m_fileIDsVector.push_back(-1);
    }
}

kaRenderingProgram::~kaRenderingProgram()
{

}

void kaRenderingProgram::Serialize(gtString& xmlString)
{
    SerializeProgramHeader(xmlString);

    // Open the source files section
    xmlString.appendFormattedString(L"<%ls>", KA_STR_sourceFilesSection);

    for (int i = KA_PIPELINE_STAGE_VERTEX; i <= KA_PIPELINE_STAGE_FRAG; i++)
    {
        kaPipelinedProgram::PipelinedStage stage = (kaPipelinedProgram::PipelinedStage)i;
        gtString renderingStageStr;
        renderingStageStr = GetRenderingStageAsString((PipelinedStage)i);
        GT_IF_WITH_ASSERT(i < (int)m_fileIDsVector.size())
        {
            gtString stageName = GetRenderingStageAsString(stage);
            xmlString.appendFormattedString(L"<%ls>", KA_STR_sourceFile);
            afUtils::addFieldToXML(xmlString, KA_STR_sourceFileId, m_fileIDsVector[i]);
            afUtils::addFieldToXML(xmlString, KA_STR_sourceFileStage, stageName);
            xmlString.appendFormattedString(L"</%ls>", KA_STR_sourceFile);
        }
    }

    // Close the source files section
    xmlString.appendFormattedString(L"</%ls>", KA_STR_sourceFilesSection);
}

void kaRenderingProgram::DeSerialize(TiXmlElement* pProgramNode)
{
    DeserializeProgramHeader(pProgramNode);

    // Read the list of source files
    gtString sourceFilesTagName = KA_STR_sourceFilesSection;
    gtString sourceFileTagName = KA_STR_sourceFile;
    TiXmlElement* pSourceFilesElement = pProgramNode->FirstChildElement(sourceFilesTagName.asASCIICharArray());

    if (pSourceFilesElement != nullptr)
    {
        TiXmlElement* pSourceFileElement = pSourceFilesElement->FirstChildElement(sourceFileTagName.asASCIICharArray());

        for (; pSourceFileElement != nullptr; pSourceFileElement = pSourceFileElement->NextSiblingElement())
        {
            gtString fileStageStr;
            int fileId = -1;
            afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_sourceFileId, fileId);
            afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_sourceFileStage, fileStageStr);

            kaPipelinedProgram::PipelinedStage stage = kaRenderingProgram::GetRenderingStageTypeFromString(fileStageStr);
            GT_IF_WITH_ASSERT(stage < (int)m_fileIDsVector.size())
            {
                m_fileIDsVector[stage] = fileId;
            }
        }
    }
}

kaProgram* kaRenderingProgram::Clone() const
{
    return new kaRenderingProgram(*this);
}

bool kaRenderingProgram::HasFile(int fileId, afTreeItemType itemType) const
{
    bool retVal = false;

    kaPipelinedProgram::PipelinedStage stage = TreeItemTypeToRenderingStage(itemType);

    if (stage != KA_PIPELINE_STAGE_NONE)
    {
        retVal = (m_fileIDsVector[stage] == fileId);
    }
    else
    {
        retVal = kaProgram::HasFile(fileId, AF_TREE_ITEM_ITEM_NONE);
    }

    return retVal;
}

void kaPipelinedProgram::OnFileRemove(const int fileId)
{
    PipelinedStage stage = GetFileRenderingStage(fileId);
    SetFileID(stage, -1);
}

kaComputeProgram::kaComputeProgram()
{
    m_fileIDsVector.push_back(-1);
}

kaComputeProgram::~kaComputeProgram()
{

}

kaProgram* kaComputeProgram::Clone() const
{
    return new kaComputeProgram(*this);
}

bool kaComputeProgram::SetFileID(PipelinedStage refType, int fileId)
{
    bool result = false;
    GT_IF_WITH_ASSERT(refType == KA_PIPELINE_STAGE_COMP)
    {
        SetFileID(fileId);
        result = true;
    }
    return result;

}

void kaComputeProgram::SetFileID(int fileId)
{
    GT_IF_WITH_ASSERT(m_fileIDsVector.size() == 1)
    {
        m_fileIDsVector[0] = fileId;
        bool isEmpty = true;
        bool isUpdateNeeded = false;
        isEmpty &= (m_fileIDsVector[0] == -1);

        if (!m_isEmpty && isEmpty)
        {
            m_isEmpty = true;
            isUpdateNeeded = true;
        }
        else if (m_isEmpty && !isEmpty)
        {
            m_isEmpty = false;
            isUpdateNeeded = true;
        }

        if (isUpdateNeeded)
        {
            if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
            {
                afApplicationCommands::instance()->updateToolbarCommands();
            }
        }
    }
}

int kaComputeProgram::GetFileID() const
{
    int retVal = -1;

    GT_IF_WITH_ASSERT(m_fileIDsVector.size() == 1)
    {
        retVal = m_fileIDsVector[0];
    }

    return retVal;
}

kaPipelinedProgram::PipelinedStage kaComputeProgram::GetRenderingStageTypeFromString(const gtString& renderingStageStr) const
{
    GT_UNREFERENCED_PARAMETER(renderingStageStr);
    return KA_PIPELINE_STAGE_COMP;
}

bool kaComputeProgram::GetFilePath(PipelinedStage stage, osFilePath& filePath) const
{
    bool ret = false;

    if (stage == PipelinedStage::KA_PIPELINE_STAGE_COMP)
    {
        int fileId = GetFileID();
        ret = fileId >= 0;

        if (ret)
        {
            KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(fileId, filePath);
        }
    }

    return ret;
}

gtString kaComputeProgram::GetRenderingStageAsString(const PipelinedStage pipelineStage) const
{
    gtString retVal;

    switch (pipelineStage)
    {
        case kaRenderingProgram::KA_PIPELINE_STAGE_COMP:
            retVal = KA_STR_sourceFileStageComputeShader;
            break;

        default:
        case kaRenderingProgram::KA_PIPELINE_STAGE_NONE:
            GT_ASSERT_EX(false, L"Unsupported stage");
            break;
    }

    return retVal;
}

kaPipelinedProgram::PipelinedStage kaComputeProgram::GetFileRenderingStage(int fileId) const
{
    PipelinedStage result = KA_PIPELINE_STAGE_NONE;

    if (m_fileIDsVector.size() > 0 && m_fileIDsVector[0] == fileId)
    {
        result = KA_PIPELINE_STAGE_COMP;
    }

    return result;
}

void kaComputeProgram::AddUniqueFileId(const int fileId)
{
    SetFileID(fileId);
}

kaNonPipelinedProgram::~kaNonPipelinedProgram()
{

}


kaProgram* kaNonPipelinedProgram::Clone() const
{
    return new kaNonPipelinedProgram(*this);
}


void kaNonPipelinedProgram::AddFile(int fileId)
{
    AddUniqueFileId(fileId);
}

void kaNonPipelinedProgram::RemoveFile(int fileId)
{
    for (int i = 0; i < (int)m_fileIDsVector.size(); i++)
    {
        if (m_fileIDsVector[i] == fileId)
        {
            m_fileIDsVector.removeItem(i);

            if (m_fileIDsVector.empty() && !m_isEmpty)
            {
                m_isEmpty = true;

                if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
                {
                    afApplicationCommands::instance()->updateToolbarCommands();
                }
            }

            break;
        }
    }
}

kaProgram* kaProgramFactory::Create(const kaProgramTypes programType, const gtString& programName)
{
    kaProgram* pProgram = nullptr;

    switch (programType)
    {
        case kaProgramGL_Rendering:
            pProgram = new kaRenderingProgram;
            break;

        case kaProgramGL_Compute:
            pProgram = new kaComputeProgram;
            break;

        case kaProgramVK_Rendering:
            pProgram = new kaRenderingProgram;
            break;

        case kaProgramVK_Compute:
            pProgram = new kaComputeProgram;
            break;

        case kaProgramCL:
            pProgram = new kaNonPipelinedProgram;
            break;

        case kaProgramDX:
            pProgram = new kaDxFolder;
            break;

        default:
            break;
    }

    GT_IF_WITH_ASSERT(pProgram != nullptr)
    {
        pProgram->SetProgramName(programName);
        pProgram->SetBuildType(programType);
    }

    return pProgram;
}


void kaProgram::GetAndCreateOutputDirectories(osDirectory& output32Dir, osDirectory& output64Dir, const bool create32BitFolder /*= true*/, const bool create64BitFolder /*= true*/) const
{
    // Get the current file path
    osFilePath programPath = kaApplicationCommands::instance().OutputFilePathForCurrentProject();
    programPath.appendSubDirectory(m_programName);
    osDirectory programDir(programPath);

    osFilePath programOutput32Path = programPath;
    programOutput32Path.appendSubDirectory(KA_STR_FileSystemOutputDir32);
    output32Dir.setDirectoryPath(programOutput32Path);

    if (create32BitFolder)
    {
        output32Dir.create();
    }

    osFilePath programOutput64Path = programPath;
    programOutput64Path.appendSubDirectory(KA_STR_FileSystemOutputDir64);
    output64Dir.setDirectoryPath(programOutput64Path);

    if (create64BitFolder)
    {
        output64Dir.create();
    }

}

gtString kaProgram::GetProgramTypeAsString(kaProgramTypes programType)
{
    gtString typeText = KA_STR_programTypeUnknown;

    switch (programType)
    {
        case kaProgramCL:
            typeText = KA_STR_programTypeCL;
            break;

        case kaProgramDX:
            typeText = KA_STR_programTypeDX;
            break;

        case kaProgramGL_Rendering:
            typeText = KA_STR_programTypeGL_Rendering;
            break;

        case kaProgramGL_Compute:
            typeText = KA_STR_programTypeGL_Compute;
            break;

        case kaProgramVK_Rendering:
            typeText = KA_STR_programTypeVK_Rendering;
            break;

        case kaProgramVK_Compute:
            typeText = KA_STR_programTypeVK_Compute;
            break;

        default:
            typeText = KA_STR_programTypeUnknown;
            break;
    }

    return typeText;
}

bool kaProgram::GetProgramTypeFromString(const gtString& programTypeAsString, kaProgramTypes& programType)
{
    bool retVal = true;

    programType = kaProgramUnknown;

    if (programTypeAsString == KA_STR_programTypeGL_Rendering)
    {
        programType = kaProgramGL_Rendering;
    }
    else if (programTypeAsString == KA_STR_programTypeGL_Compute)
    {
        programType = kaProgramGL_Compute;
    }
    else if (programTypeAsString == KA_STR_programTypeVK_Rendering)
    {
        programType = kaProgramVK_Rendering;
    }
    else if (programTypeAsString == KA_STR_programTypeVK_Compute)
    {
        programType = kaProgramVK_Compute;
    }
    else if (programTypeAsString == KA_STR_programTypeCL)
    {
        programType = kaProgramCL;
    }
    else if (programTypeAsString == KA_STR_programTypeDX)
    {
        programType = kaProgramDX;
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

kaPipelinedProgram::PipelinedStage kaRenderingProgram::GetFileRenderingStage(int fileId) const
{
    PipelinedStage result = KA_PIPELINE_STAGE_NONE;

    //if valid FileId
    if (fileId > -1)
    {
        gtVector<int>::const_iterator  it = std::find_if(m_fileIDsVector.begin(), m_fileIDsVector.end(), [&](const int id)
        {
            return id == fileId;
        });

        if (it != m_fileIDsVector.end())
        {
            //get vector index and convert it to render stage
            result = static_cast<PipelinedStage>(it - m_fileIDsVector.begin());
        }
    }

    return result;
}

kaPipelinedProgram::PipelinedStage kaRenderingProgram::GetRenderingStageTypeFromString(const gtString& renderingStageStr) const
{
    kaPipelinedProgram::PipelinedStage retVal = KA_PIPELINE_STAGE_NONE;

    gtString typeStr = renderingStageStr;
    typeStr.toLowerCase();

    if (renderingStageStr == KA_STR_sourceFileStageVertex || renderingStageStr.find(gtString(KA_STR_fileGLVert).toLowerCase()) == 0)
    {
        retVal = kaRenderingProgram::KA_PIPELINE_STAGE_VERTEX;
    }
    else if (renderingStageStr == KA_STR_sourceFileStageTesc || renderingStageStr.find(gtString(KA_STR_fileGLTesc).toLowerCase()) == 0)
    {
        retVal = kaRenderingProgram::KA_PIPELINE_STAGE_TESC;
    }
    else if (renderingStageStr == KA_STR_sourceFileStageTese || renderingStageStr.find(gtString(KA_STR_fileGLTese).toLowerCase()) == 0)
    {
        retVal = kaRenderingProgram::KA_PIPELINE_STAGE_TESE;
    }
    else if (renderingStageStr == KA_STR_sourceFileStageGeom || renderingStageStr.find(gtString(KA_STR_fileGLGeom).toLowerCase()) == 0)
    {
        retVal = kaRenderingProgram::KA_PIPELINE_STAGE_GEOM;
    }
    else if (renderingStageStr == KA_STR_sourceFileStageFrag || renderingStageStr.find(gtString(KA_STR_fileGLFrag).toLowerCase()) == 0)
    {
        retVal = kaRenderingProgram::KA_PIPELINE_STAGE_FRAG;
    }

    else
    {
        GT_ASSERT_EX(false, L"Unsupported render stage");
    }

    return retVal;
}
//Convert render stage to CLI string
gtString kaRenderingProgram::GetRenderingStageAsCLIString(PipelinedStage stage)
{
    gtString retVal;

    switch (stage)
    {
        case kaRenderingProgram::KA_PIPELINE_STAGE_VERTEX:
            retVal = KA_STR_CLI_VERTEX_ABBREVIATION;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_TESE:
            retVal = KA_STR_CLI_TESS_EVAL_ABBREVIATION;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_TESC:
            retVal = KA_STR_CLI_TESS_CTRL_ABBREVIATION;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_GEOM:
            retVal = KA_STR_CLI_GEOMETRY_ABBREVIATION;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_FRAG:
            retVal = KA_STR_CLI_FRAGMENT_ABBREVIATION;
            break;

        default:
        case kaRenderingProgram::KA_PIPELINE_STAGE_NONE:
            GT_ASSERT_EX(false, L"Unsupported stage");
            break;
    }

    return retVal;
}


gtString kaRenderingProgram::GetRenderingStageAsString(const PipelinedStage stage) const
{
    gtString retVal;

    switch (stage)
    {
        case kaRenderingProgram::KA_PIPELINE_STAGE_VERTEX:
            retVal = KA_STR_sourceFileStageVertex;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_TESE:
            retVal = KA_STR_sourceFileStageTese;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_TESC:
            retVal = KA_STR_sourceFileStageTesc;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_GEOM:
            retVal = KA_STR_sourceFileStageGeom;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_FRAG:
            retVal = KA_STR_sourceFileStageFrag;
            break;

        default:
        case kaRenderingProgram::KA_PIPELINE_STAGE_NONE:
            GT_ASSERT_EX(false, L"Unsupported stage");
            break;
    }

    return retVal;
}


kaPipelinedProgram::PipelinedStage kaRenderingProgram::TreeItemTypeToRenderingStage(afTreeItemType itemType)
{
    PipelinedStage retVal = kaRenderingProgram::KA_PIPELINE_STAGE_NONE;

    switch (itemType)
    {
        case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_GEOM;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_FRAG;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_TESC;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_TESE;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_VERTEX;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_COMP;
            break;

        default:
            break;
    }

    return retVal;
}

kaPipelinedProgram::PipelinedStage kaRenderingProgram::FileTypeToRenderStage(kaFileTypes fileType)
{
    PipelinedStage retVal = KA_PIPELINE_STAGE_NONE;

    switch (fileType)
    {
        case kaFileTypeGLSLFrag:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_FRAG;
            break;

        case kaFileTypeGLSLVert:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_VERTEX;
            break;

        case kaFileTypeGLSLGeom:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_GEOM;
            break;

        case kaFileTypeGLSLTesc:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_TESC;
            break;

        case kaFileTypeGLSLTese:
            retVal = kaRenderingProgram::KA_PIPELINE_STAGE_TESE;
            break;

        default:
            break;
    }

    return retVal;
}

kaFileTypes kaRenderingProgram::RenderStageToFileType(PipelinedStage stage)
{
    kaFileTypes retVal = kaFileTypeUnknown;

    switch (stage)
    {
        case kaRenderingProgram::KA_PIPELINE_STAGE_VERTEX:
            retVal = kaFileTypeGLSLVert;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_TESE:
            retVal = kaFileTypeGLSLTese;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_TESC:
            retVal = kaFileTypeGLSLTesc;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_GEOM:
            retVal = kaFileTypeGLSLGeom;
            break;

        case kaRenderingProgram::KA_PIPELINE_STAGE_FRAG:
            retVal = kaFileTypeGLSLFrag;
            break;

        default:
            break;
    }

    return retVal;
}

int kaRenderingProgram::GetFileID(PipelinedStage refType) const
{
    int retVal = -1;

    if ((refType != KA_PIPELINE_STAGE_NONE) && (refType < (int)m_fileIDsVector.size()))
    {
        retVal = m_fileIDsVector[refType];
    }

    return retVal;
}

bool kaRenderingProgram::GetFilePath(PipelinedStage refType, osFilePath& filePath) const
{
    bool ret = false;
    int shaderId = GetFileID(refType);

    if (shaderId > -1)
    {
        KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(shaderId, filePath);
        ret = true;
    }

    return ret;
}

bool kaRenderingProgram::GetFilePath(PipelinedStage refType, gtString& filePathString) const
{
    bool ret = false;
    int shaderId = GetFileID(refType);

    if (shaderId > -1)
    {
        osFilePath filePath;
        KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(shaderId, filePath);
        filePathString = filePath.asString();
        ret = true;
    }

    return ret;
}

void kaRenderingProgram::GetPipelinePaths(kaPipelineShaders& filePath) const
{
    GetFilePath(KA_PIPELINE_STAGE_VERTEX, filePath.m_vertexShader);
    GetFilePath(KA_PIPELINE_STAGE_TESC, filePath.m_tessControlShader);
    GetFilePath(KA_PIPELINE_STAGE_TESE, filePath.m_tessEvaluationShader);
    GetFilePath(KA_PIPELINE_STAGE_GEOM, filePath.m_geometryShader);
    GetFilePath(KA_PIPELINE_STAGE_FRAG, filePath.m_fragmentShader);
}

bool kaRenderingProgram::SetFileID(PipelinedStage refType, int fileRefId)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((refType != KA_PIPELINE_STAGE_NONE) && (refType < (int)m_fileIDsVector.size()))

    {
        m_fileIDsVector[refType] = fileRefId;
        bool isEmpty = true;
        bool isUpdateNeeded = false;

        for (int it : m_fileIDsVector)
        {
            isEmpty &= (it == -1);
        }

        if (!m_isEmpty && isEmpty)
        {
            m_isEmpty = true;
            isUpdateNeeded = true;
        }
        else if (m_isEmpty && !isEmpty)
        {
            m_isEmpty = false;
            isUpdateNeeded = true;
        }

        if (isUpdateNeeded)
        {
            if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
            {
                afApplicationCommands::instance()->updateToolbarCommands();
            }
        }
    }

    return retVal;
}

bool kaDxFolder::GetFileSelectedEntryPoint(int fileId, gtString& entryPoint) const
{
    bool retVal = false;
    entryPoint.makeEmpty();

    auto iter = m_fileProperties.find(fileId);

    if (iter != m_fileProperties.end())
    {
        entryPoint = iter->second.m_entryPoint;
        retVal = !entryPoint.isEmpty();
    }

    return retVal;
}

bool kaDxFolder::GetFileShaderModel(int fileId, gtString& model) const
{
    bool retVal = false;
    model.makeEmpty();

    auto iter = m_fileProperties.find(fileId);

    if (iter != m_fileProperties.end())
    {
        model = iter->second.m_model;
        retVal = !model.isEmpty();
    }

    return retVal;
}

bool kaDxFolder::GetFileProfile(int fileId, gtString& profile) const
{
    gtString fileModel, fileType;
    bool retVal = false;

    if (true == (retVal = GetFileShaderModel(fileId, fileModel) && GetFileSelectedShaderType(fileId, fileType)))
    {
        fileType = fileType.toLowerCase();
        profile << fileType.truncate(0, 0) << L"s_" << fileModel;
        retVal = !profile.isEmpty();
    }

    return retVal;
}

bool kaDxFolder::SetFileProfile(int fileId, const gtString& profile)
{
    bool retVal = false;
    // profile should be six characters long, e.g. cs_5_0
    GT_IF_WITH_ASSERT(!profile.isEmpty() && profile.length() == PROFILE_CHARS_NUM)
    {
        gtString fileModel, fileType;

        auto iter = m_fileProperties.find(fileId);

        if (iter != m_fileProperties.end())
        {
            QStringList shaderTypes = QString(KA_STR_toolbarDXShaderTypes).split(" ");
            QStringList shaderTypesShort = QString(KA_STR_toolbarDXShaderTypesShort).split(" ");

            if (shaderTypes.size() == shaderTypesShort.size())
            {
                for (int i = 0; i < shaderTypes.size(); ++i)
                {
                    if (acGTStringToQString(profile).contains(shaderTypesShort[i]))
                    {
                        fileType = acQStringToGTString(shaderTypes[i]);
                        profile.getSubString(3, 5, fileModel);
                        retVal = (!fileType.isEmpty() && !fileModel.isEmpty());
                        SetFileModel(fileId, fileModel);
                        SetFileSelectedType(fileId, fileType);
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

bool kaDxFolder::GetFileSelectedShaderType(int fileId, gtString& type) const
{
    bool retVal = false;
    type.makeEmpty();
    auto iter = m_fileProperties.find(fileId);

    if (iter != m_fileProperties.end())
    {
        type = iter->second.m_type;
        retVal = !type.isEmpty();
    }

    return retVal;
}

void kaDxFolder::SetFileModel(int fileId, const gtString& model)
{
    m_fileProperties[fileId].m_model = model;
}

void kaDxFolder::SetFileModel(const gtString& model)
{
    for (auto& it : m_fileProperties)
    {
        it.second.m_model = model;
    }
}

void kaDxFolder::SetFileSelectedType(int fileId, const gtString& type)
{
    m_fileProperties[fileId].m_type = type;
}

void kaDxFolder::SetFileSelectedEntryPoint(int fileId, const gtString& selectedEntryPoint)
{
    m_fileProperties[fileId].m_entryPoint = selectedEntryPoint;
}

void kaDxFolder::Serialize(gtString& xmlString)
{
    SerializeProgramHeader(xmlString);

    if (m_fileProperties.size() > 0)
    {

        // Open the source files section
        xmlString.appendFormattedString(L"<%ls>", KA_STR_sourceFilesSection);

        for (auto it : m_fileProperties)
        {
            xmlString.appendFormattedString(L"<%ls>", KA_STR_sourceFile);
            afUtils::addFieldToXML(xmlString, KA_STR_sourceFileId, it.first);
            afUtils::addFieldToXML(xmlString, KA_STR_filePropertiesModel, it.second.m_model);
            afUtils::addFieldToXML(xmlString, KA_STR_filePropertiesType, it.second.m_type);
            afUtils::addFieldToXML(xmlString, KA_STR_filePropertiesEntryPoint, it.second.m_entryPoint);
            xmlString.appendFormattedString(L"</%ls>", KA_STR_sourceFile);
        }

        // Close the source files section
        xmlString.appendFormattedString(L"</%ls>", KA_STR_sourceFilesSection);
    }
}

void kaDxFolder::DeSerialize(TiXmlElement* pProgramNode)
{
    // Call the base class implementation
    DeserializeProgramHeader(pProgramNode);

    // Sanity check:
    GT_IF_WITH_ASSERT(pProgramNode != nullptr)
    {
        // Read the list of source files
        gtString sourceFilesTagName = KA_STR_sourceFilesSection;
        gtString sourceFileTagName = KA_STR_sourceFile;

        TiXmlElement* pSourceFilesElement = pProgramNode->FirstChildElement(sourceFilesTagName.asASCIICharArray());

        if (pSourceFilesElement != nullptr)
        {
            TiXmlElement* pSourceFileElement = pSourceFilesElement->FirstChildElement(sourceFileTagName.asASCIICharArray());

            for (; pSourceFileElement != nullptr; pSourceFileElement = pSourceFileElement->NextSiblingElement())
            {
                int fileId = -1;
                gtString sourceFileModel;
                gtString sourceFileType;
                gtString sourceFileEntryPoint;
                afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_sourceFileId, fileId);
                afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_filePropertiesModel, sourceFileModel);
                afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_filePropertiesType, sourceFileType);
                afUtils::getFieldFromXML(*pSourceFileElement, KA_STR_filePropertiesEntryPoint, sourceFileEntryPoint);

                SetFileModel(fileId, sourceFileModel);
                SetFileSelectedType(fileId, sourceFileType);
                SetFileSelectedEntryPoint(fileId, sourceFileEntryPoint);
                AddFile(fileId);
            }
        }
    }
}

kaProgram* kaDxFolder::Clone() const
{
    return  new kaDxFolder(*this);
}

void kaDxFolder::RemoveFile(int fileId)
{
    auto iter = m_fileProperties.find(fileId);

    if (iter != m_fileProperties.end())
    {
        m_fileProperties.erase(iter);
    }

    kaNonPipelinedProgram::RemoveFile(fileId);
}

void kaDxFolder::SetProgramLevelShaderModel(gtString shaderModel)
{
    m_programShaderModel = shaderModel;
    SetFileModel(m_programShaderModel);
}

bool kaPipelinedProgram::HasFile() const
{
    bool result = false;

    if (m_fileIDsVector.empty() == false)
    {
        result = find_if(m_fileIDsVector.begin(), m_fileIDsVector.end(), [](int fileId) { return fileId != -1; }) != m_fileIDsVector.end();
    }

    return result;
}
