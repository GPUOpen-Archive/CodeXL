//------------------------------ kaProgram.h ------------------------------

#ifndef __KAPROGRAM_h
#define __KAPROGRAM_h

/// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

/// infra
#include <AMDTAPIClasses/Include/apSerializable.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtSet.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIDocUpdateHandler.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

/// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/src/kaFileManager.h>



class KA_API kaProgram : public  apXmlSerializable, public kaFileManagerChangeRecipient
{
public:


    kaProgram();
    kaProgram(const kaProgram&) = default;
    virtual ~kaProgram();

    virtual void Serialize(gtString& xmlString) override;
    virtual void DeSerialize(TiXmlElement* pProgramNode) override;
    virtual kaProgram* Clone() const;
    virtual void OnFileRemove(const int fileId);

    /// Does the program contain the requested file id?
    /// For compute programs, the function will look for the file path.
    /// For rendering programs, the function will look at a specific stage
    /// \param fileId the file id
    /// \return true if the file is contained in the program
    virtual bool HasFile(int fileId, afTreeItemType itemType) const;

    /// Does the program contain the requested file path?
    /// For compute programs, the function will look for the file path.
    /// For rendering programs, the function will look at a specific stage
    /// \param fileId the file path
    /// \return true if the file is contained in the program
    virtual bool HasFile(const osFilePath& filePath, afTreeItemType itemType) const;
    virtual bool HasFile() const { return m_fileIDsVector.empty() == false; }

    /// returns program build platform
    kaProgramTypes GetBuildType() const { return m_buildType; }

    /// sets program build platform
    void SetBuildType(kaProgramTypes buildType) { m_buildType = buildType; }

    /// sets program name
    void SetProgramName(const gtString& name);

    ///gets program name
    gtString GetProgramName() const { return m_programName; }

    /// sets the program's display name
    void SetProgramDisplayName(const gtString& name) { m_displayName = name; }

    /// gets the program's display name
    gtString GetProgramDisplayName() const { return m_displayName; }

    /// Get the output directories
    void GetAndCreateOutputDirectories(osDirectory& output32Dir, osDirectory& output64Dir, const bool create32BitFolder = true, const bool create64BitFolder = true) const;

    ///Runs on all source files ids(m_fileIDsVector) for this program
    ///and erases the ones that are not inside argument ids list
    /// \param [in] ids
    void FilterByIds(const gtList<int>& ids);

    /// returns text string
    static gtString GetProgramTypeAsString(kaProgramTypes programType);

    /// Get the enumeration for the program type from the string
    /// \param the program type as string
    /// \param the program type as kaProgramTypes
    /// \return true for success
    static bool GetProgramTypeFromString(const gtString& programTypeAsString, kaProgramTypes& programType);

    /// Get the enumeration for the program type from the file path extensions
    /// \param the file path
    /// \param the program type as kaProgramTypes
    /// \return true for success
    static bool GetProgramTypeFromFileExtention(const osFilePath& filePath, kaProgramTypes& programType);

    /// Get the enumeration for the program type from the file paths extensions, 
    /// if files have  different program types result would be program type unknown
    /// \param the file paths
    /// \param the program type as kaProgramTypes
    /// \return true for if all files have same program type extensions
    static bool GetProgramTypeFromFileExtention(const gtVector<osFilePath>& filePaths, kaProgramTypes& programType);

    /// Get the files vector
    const gtVector<int>& GetFileIDsVector() const { return m_fileIDsVector; };

    /// Fills filePaths with program files
    /// \param [out] filePaths
    void GetProgramFiles(gtVector<osFilePath>& filesPath)const;

protected:
    void SerializeProgramHeader(gtString& xmlString) const;
    void DeserializeProgramHeader(TiXmlElement* pProgramNode);
    virtual void AddUniqueFileId(const int fileId);
protected:
    /// program name
    gtString m_programName;

    /// program display name (for use in build output)
    gtString m_displayName;

    /// build platform
    kaProgramTypes m_buildType;


    /// This vector holds the file references for the current program
    gtVector<int> m_fileIDsVector;

    /// This flag stores the state of a program - is it empty or has files
    bool m_isEmpty;
};

class KA_API kaPipelinedProgram : public kaProgram
{
public:
    enum PipelinedStage
    {
        KA_PIPELINE_STAGE_NONE = -1,
        KA_PIPELINE_STAGE_VERTEX = 0,
        KA_PIPELINE_STAGE_TESE,
        KA_PIPELINE_STAGE_TESC,
        KA_PIPELINE_STAGE_GEOM,
        KA_PIPELINE_STAGE_FRAG,
        KA_PIPELINE_STAGE_COMP,
        KA_PIPELINE_STAGE_LAST
    };

    virtual PipelinedStage  GetRenderingStageTypeFromString(const gtString& renderingStageStr) const = 0;
    /// Get the file path for the requested stage
    /// \return true for success, false otherwise
    virtual bool GetFilePath(PipelinedStage stage, osFilePath& filePath) const = 0;
    virtual gtString GetRenderingStageAsString(const PipelinedStage pipelineStage) const = 0;
    virtual bool HasFile() const override;
    /// Get the rendering stage for the given file
    virtual PipelinedStage GetFileRenderingStage(int fileId) const = 0;
    virtual void OnFileRemove(const int fileId) override;
    virtual bool SetFileID(PipelinedStage refType, int fileRefId) = 0;
};
class KA_API kaRenderingProgram : public kaPipelinedProgram
{
public:

    kaRenderingProgram();
    virtual ~kaRenderingProgram();
    virtual void Serialize(gtString& xmlString) override;
    virtual void DeSerialize(TiXmlElement* pProgramNode) override;
    virtual kaProgram* Clone() const override;


    /// Does the program have a file with the fileId in the stage related to itemType?
    virtual bool HasFile(int fileId, afTreeItemType itemType) const;

    /// Set the file id for the requested stage
    /// \param the stage for which the file id should be set
    /// \param fileRefId file id for this stage
    /// \return true for success
    virtual bool SetFileID(PipelinedStage refType, int fileRefId) override;

    virtual PipelinedStage GetFileRenderingStage(int fileId) const override;
    /// Get the file id for the requested stage
    /// \return file id for the requested stage (-1 for a non set file)
    int GetFileID(PipelinedStage stage) const;

    /// Get the file path for the requested stage
    /// \return true for success, false otherwise
    bool GetFilePath(PipelinedStage stage, osFilePath& filePath) const override;

    /// Get the string representation of the file path for the requested stage
    /// \return true for success, false otherwise
    bool GetFilePath(PipelinedStage stage, gtString& filePath) const;

    /// Get the file path for the requested stage
    /// \return true for success, false otherwise
    void GetPipelinePaths(kaPipelineShaders& filePath) const;

    /// Get the reference for the member according to the XML string. Member is one of the stages id:
    PipelinedStage GetRenderingStageTypeFromString(const gtString& renderingStageStr) const override;
    virtual gtString GetRenderingStageAsString(const PipelinedStage renderingStage) const override;
    static gtString GetRenderingStageAsCLIString(PipelinedStage stage);
    static PipelinedStage TreeItemTypeToRenderingStage(afTreeItemType itemType);
    static PipelinedStage FileTypeToRenderStage(kaFileTypes fileType);
    static kaFileTypes RenderStageToFileType(PipelinedStage stage);
};


class KA_API kaComputeProgram : public kaPipelinedProgram
{
public:
    kaComputeProgram();
    virtual ~kaComputeProgram();
    virtual kaProgram* Clone() const override;

    virtual bool SetFileID(PipelinedStage refType, int fileId) override;
    void SetFileID(int fileId);
    int GetFileID() const;

    virtual kaPipelinedProgram::PipelinedStage GetRenderingStageTypeFromString(const gtString& renderingStageStr) const override;
    /// Get the file path for the requested stage
    /// \return true for success, false otherwise
    virtual bool GetFilePath(PipelinedStage stage, osFilePath& filePath) const override;
    virtual gtString GetRenderingStageAsString(const PipelinedStage pipelineStage) const override;
    virtual PipelinedStage GetFileRenderingStage(int fileId) const override;
protected:
    virtual void AddUniqueFileId(const int fileId) override;

};


class KA_API kaNonPipelinedProgram : public kaProgram
{
public:
    kaNonPipelinedProgram() {};
    virtual ~kaNonPipelinedProgram();
    virtual kaProgram* Clone() const override;

    /// Add a file id to the vector of files
    void AddFile(int fileId);

    /// Remove a file id from the vector of files
    virtual void RemoveFile(int fileId);
};

class KA_API kaProgramFactory
{
public:
    static kaProgram* Create(const kaProgramTypes programType, const gtString& programName);
private:
    kaProgramFactory() = delete;

};

class KA_API kaDxFolder :
    public kaNonPipelinedProgram
{
public:

    virtual void Serialize(gtString& xmlString) override;
    virtual void DeSerialize(TiXmlElement* pProgramNode) override;
    virtual kaProgram* Clone() const override;

    /// Get the selected entry point for the file.
    bool GetFileSelectedEntryPoint(int fileId, gtString& entryPoint) const;

    /// Get the selected entry point for the file.
    bool GetFileShaderModel(int fileId, gtString& model) const;
    /// Get file profile (combination of type and model).
    bool GetFileProfile(int fileId, gtString& profile) const;

    /// Set file profile (e.g. cs_5_0 -> Type = Compute; Model = 5_0 ).
    bool SetFileProfile(int fileId, const gtString& profile);

    /// Get the selected entry point for the file.
    bool GetFileSelectedShaderType(int fileId, gtString& type) const;

    void SetFileModel(int fileId, const gtString& model);
    void SetFileModel(const gtString& model);
    void SetFileSelectedType(int fileId, const gtString& model);
    void SetFileSelectedEntryPoint(int fileId, const gtString& model);
    void RemoveFile(int fileId);
    gtString GetProgramLevelShaderModel()const { return m_programShaderModel; }
    void SetProgramLevelShaderModel(gtString shaderModel);

private:

    // Structure to hold DX-specific file properties.
    struct kaDxFolderFileProperties
    {
        gtString m_model;
        gtString m_type;
        gtString m_entryPoint;
    };

    gtMap<int, kaDxFolderFileProperties> m_fileProperties;
    gtString m_programShaderModel;
};

#endif //__KAPROGRAM_h