// Qt.
#include <QString>

// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>


// Local.
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/src/kaProgram.h>

//Boost
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include <boost/filesystem.hpp>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #pragma GCC diagnostic pop
#endif


#ifndef kaUtils_h__
#define kaUtils_h__

class kaUtils
{
public:
    // Get kernel name from identify file path
    static QString GetKernelNameFromPath(const osFilePath& identifyFilePath);
    static gtString ToGtString(const boost::filesystem::path& path);
    static QString ProgramTypeToPlatformString(kaProgramTypes programType);
    static kaFileTypes PipelineStageToFileType(kaProgramTypes programType, kaPipelinedProgram::PipelinedStage stage);
    static gtString PipeLineStageToLongFormat(const gtString& pipeLineStageShortFormat);
    static void TreeItemTypeToPipeLineStageString(afTreeItemType fileType, gtString& shaderName);
    static gtString PipeLineStageToLongFormat(const kaPipelinedProgram::PipelinedStage stage);
private:


    // No instances for this class.
    kaUtils();
    ~kaUtils();
};

#endif // kaUtils_h__
