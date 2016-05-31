#include <AMDTKernelAnalyzer/src/kaUtils.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>


QString kaUtils::GetKernelNameFromPath(const osFilePath& identifyFilePath)
{
    QString retStr;

    // Get the kernel name from the identifyFilePath: the file name is program name/output32/devices_kernelname_filename.txt
    // since the kernel name can have "_" and we can't know where it ends and where the filename start we need to find the program name
    // and look in its files to find the file name.

    // get program name:
    boost::filesystem::path path(identifyFilePath.asString().asCharArray());
    gtString programName = ToGtString(path.parent_path().parent_path().filename());
    GT_IF_WITH_ASSERT(programName.isEmpty() == false)
    {
        kaProgram* pKernelProgram = KA_PROJECT_DATA_MGR_INSTANCE.GetProgram(programName);
        // find the file name in the identifyFilePath so it can be removed and we'll be left with the kernel name
        GT_IF_WITH_ASSERT(pKernelProgram != nullptr)
        {
            // take the identifyFilePath and remove the "device" section
            gtString fileName;
            gtString kernelName;
            identifyFilePath.getFileName(fileName);
            QString sectionSeperator(KA_STR_fileSectionSeperator);
            int deviceEnd = fileName.find(acQStringToGTString(sectionSeperator));
            GT_IF_WITH_ASSERT(deviceEnd != -1 && deviceEnd < fileName.length() - 1)
            {
                fileName.getSubString(deviceEnd + 1, fileName.length(), kernelName);
            }
            const kaPipelinedProgram*  pIsPipeLineProgram = dynamic_cast<kaPipelinedProgram*>(pKernelProgram);

            //for pipeline programs we just take the kernel name as is
            if (pIsPipeLineProgram != nullptr)
            {
                kernelName = PipeLineStageToLongFormat(kernelName);
                kernelName.append(L" ").append(KA_STR_sourceFileStageShader);
                retStr = (acGTStringToQString(kernelName));
            }
            else
            {
                // pass through all the files in the program look for a source file in the remaining file name and what is left is the kernel name:
                gtVector<int> fileRefsVec = pKernelProgram->GetFileIDsVector();
                int numFileRef = fileRefsVec.size();

                osFilePath currentFilePath;

                for (int nRef = 0; nRef < numFileRef; nRef++)
                {
                    currentFilePath.clear();
                    KA_PROJECT_DATA_MGR_INSTANCE.GetFilePathByID(fileRefsVec[nRef], currentFilePath);
                    //clear variable
                    fileName.makeEmpty();
                    currentFilePath.getFileName(fileName);
                    int fileNamePos = kernelName.reverseFind(fileName);

                    if (fileNamePos != -1 && fileNamePos > 1)
                    {
                        gtString kernelNameAsStr;

                        if (pKernelProgram->GetBuildType() == kaProgramCL || pKernelProgram->GetBuildType() == kaProgramDX)
                        {
                            kernelName.getSubString(0, fileNamePos - 2, kernelNameAsStr);
                        }
                        else
                        {
                            kernelNameAsStr = kernelName;
                        }

                        retStr = (acGTStringToQString(kernelNameAsStr));
                        break;
                    }
                }
            }//else non pipeline
        }
    }
    return retStr;
}

gtString kaUtils::ToGtString(const boost::filesystem::path& path)
{

#if  AMDT_BUILD_TARGET == AMDT_LINUX_OS
    gtString result = gtString().fromASCIIString(path.generic_string().c_str());
#else
    gtString result = path.c_str();
#endif


    return result;
}

QString kaUtils::ProgramTypeToPlatformString(kaProgramTypes programType)
{
    QString ret;

    switch (programType)
    {
        case kaProgramGL_Rendering:
        case kaProgramGL_Compute:
            ret = KA_STR_platformOpenGL;
            break;

        case kaProgramVK_Rendering:
        case kaProgramVK_Compute:
            ret = KA_STR_platformVulkan;
            break;

        case kaProgramCL:
            ret = KA_STR_platformOpenCL;
            break;

        case kaProgramDX:
            ret = KA_STR_platformDirectX;
            break;

        case kaProgramUnknown:
            GT_ASSERT_EX(false, L"Unknown program type");

        default:
            break;
    }

    return ret;
}


kaFileTypes kaUtils::PipelineStageToFileType(kaProgramTypes programType, kaPipelinedProgram::PipelinedStage stage)
{
    kaFileTypes fileType = kaFileTypeUnknown;

    switch (programType)
    {
        case kaProgramCL:
            fileType = kaFileTypeOpenCL;
            break;

        case kaProgramGL_Compute:
        case kaProgramVK_Compute:
        {
            if (stage == kaPipelinedProgram::PipelinedStage::KA_PIPELINE_STAGE_COMP)
            {
                fileType = kaFileTypeGLSLComp;
            }
            else
            {
                fileType = kaFileTypeUnknown;
            }
        }
        break;

        case kaProgramGL_Rendering:
        case kaProgramVK_Rendering:
        {
            switch (stage)
            {
                case kaPipelinedProgram::PipelinedStage::KA_PIPELINE_STAGE_VERTEX:
                    fileType = kaFileTypeGLSLVert;
                    break;

                case kaPipelinedProgram::PipelinedStage::KA_PIPELINE_STAGE_TESC:
                    fileType = kaFileTypeGLSLTesc;
                    break;

                case kaPipelinedProgram::PipelinedStage::KA_PIPELINE_STAGE_TESE:
                    fileType = kaFileTypeGLSLTese;
                    break;

                case kaPipelinedProgram::PipelinedStage::KA_PIPELINE_STAGE_GEOM:
                    fileType = kaFileTypeGLSLGeom;
                    break;

                case kaPipelinedProgram::PipelinedStage::KA_PIPELINE_STAGE_FRAG:
                    fileType = kaFileTypeGLSLFrag;
                    break;

                default:
                    GT_ASSERT_EX(false, L"Unknown stage");
                    fileType = kaFileTypeUnknown;
                    break;
            }

            break;
        }

        default:
            fileType = kaFileTypeUnknown;
            break;
    }

    return fileType;
}

gtString kaUtils::PipeLineStageToLongFormat(const gtString& pipeLineStageShortFormat)
{
    gtString result;

    if (0 == pipeLineStageShortFormat.compareNoCase(KA_STR_CLI_VERTEX_ABBREVIATION))
    {
        result = KA_STR_CLI_VERTEX_LONG;
    }
    else if (0 == pipeLineStageShortFormat.compareNoCase(KA_STR_CLI_TESS_CTRL_ABBREVIATION))
    {
        result = KA_STR_CLI_TESS_CTRL_LONG;
    }
    else if (0 == pipeLineStageShortFormat.compareNoCase(KA_STR_CLI_TESS_EVAL_ABBREVIATION))
    {
        result = KA_STR_CLI_TESS_EVAL_LONG;
    }
    else if (0 == pipeLineStageShortFormat.compareNoCase(KA_STR_CLI_GEOMETRY_ABBREVIATION))
    {
        result = KA_STR_CLI_GEOMETRY_LONG;
    }

    if (0 == pipeLineStageShortFormat.compareNoCase(KA_STR_CLI_FRAGMENT_ABBREVIATION))
    {
        result = KA_STR_CLI_FRAGMENT_LONG;
    }
    else if (0 == pipeLineStageShortFormat.compareNoCase(KA_STR_CLI_COMP_ABBREVIATION))
    {
        result = KA_STR_CLI_COMPUTE_LONG;
    }

    return result;
}

void kaUtils::TreeItemTypeToPipeLineStageString(afTreeItemType fileType, gtString& shaderName)
{
    shaderName.makeEmpty();

    switch (fileType)
    {
        case AF_TREE_ITEM_KA_PROGRAM_GL_VERT:
            shaderName = KA_STR_CLI_VERTEX_LONG;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_TESC:
            shaderName = KA_STR_CLI_TESS_CTRL_LONG;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_TESE:
            shaderName = KA_STR_CLI_TESS_EVAL_LONG;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_GEOM:
            shaderName = KA_STR_CLI_GEOMETRY_LONG;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_FRAG:
            shaderName = KA_STR_CLI_FRAGMENT_LONG;
            break;

        case AF_TREE_ITEM_KA_PROGRAM_GL_COMP:
            shaderName = KA_STR_CLI_COMPUTE_LONG;
            break;

        default:
            break;
    }
}

gtString kaUtils::PipeLineStageToLongFormat(const kaPipelinedProgram::PipelinedStage stage)
{
    gtString result;

    switch (stage)
    {
        case kaPipelinedProgram::KA_PIPELINE_STAGE_VERTEX:
            result = KA_STR_CLI_VERTEX_LONG;
            break;

        case kaPipelinedProgram::KA_PIPELINE_STAGE_TESE:
            result = KA_STR_CLI_TESS_EVAL_LONG;
            break;

        case kaPipelinedProgram::KA_PIPELINE_STAGE_TESC:
            result = KA_STR_CLI_TESS_CTRL_LONG;
            break;

        case kaPipelinedProgram::KA_PIPELINE_STAGE_GEOM:
            result = KA_STR_CLI_GEOMETRY_LONG;
            break;

        case kaPipelinedProgram::KA_PIPELINE_STAGE_FRAG:
            result = KA_STR_CLI_FRAGMENT_LONG;
            break;

        case kaPipelinedProgram::KA_PIPELINE_STAGE_COMP:
            result = KA_STR_CLI_COMPUTE_LONG;
            break;

        default:
            GT_ASSERT(false);
            break;
    }

    return result;
}



kaUtils::kaUtils()
{
}


kaUtils::~kaUtils()
{
}
