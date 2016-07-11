//------------------------------ kaProjectDataManager.h ------------------------------

#ifndef __KAPROJECTDATAMANAGER_h
#define __KAPROJECTDATAMANAGER_h

/// C++
#include <memory>
#include <unordered_map>


/// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

/// infra
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtMap.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIDocUpdateHandler.h>

/// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>
#include <AMDTKernelAnalyzer/src/kaProgram.h>
#include <AMDTKernelAnalyzer/src/kaFileManager.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>

#define KA_PROJECT_DATA_MGR_INSTANCE kaProjectDataManager::instance()

class TiXmlNode;

enum kaFileInformation
{
    kaFilePlatform = 0,
    kaFileDXShaderProfile,
    kaFileEntryPoint,
    kaFileGLShaderType
};

enum kaGLShaderTypes
{
    FRAGMENT = 0,
    VERTEX,
    COMPUTE,
    GEOMETRY,
    TESSCONT,
    TESSEVAL,
    GLSHADERTYPES_TOTAL,
};

class kaProjectDataManagerAnaylzeData
{
public:
    kaProjectDataManagerAnaylzeData();
    ~kaProjectDataManagerAnaylzeData() {};
    QString m_kernelName;
    int m_globalWorkSize[3];
    int m_localWorkSize[3];
    int m_loopIterations;
    int m_lineInSourceFile;

    /// Copy the data from the default execution values:
    void CopyDataFromExecutionData(const kaKernelExecutionDataStruct& executionData);
    struct Compare
    {
        bool operator()(const kaProjectDataManagerAnaylzeData& a, const kaProjectDataManagerAnaylzeData& b)
        {
            return a.m_kernelName < b.m_kernelName;
        }
    } ;


};

class KA_API kaSourceFile : public apXmlSerializable
{
public:
    kaSourceFile();
    ~kaSourceFile() {};


    virtual void Serialize(gtString&) override;
    virtual void DeSerialize(TiXmlElement* pNode) override;

    int  id() const { return m_id; }
    void setId(const int id) { m_id = id; }
    osFilePath& filePath() { return m_filePath; }
    void setFilePath(const osFilePath& iFilePath);

    const osDirectory& buildDirectory()const { return m_buildDir; }
    void setBuildDirectory(const osDirectory& iBuildDir) { m_buildDir = iBuildDir; }

    QStringList& buildFiles() { return m_buildFiles; }
    void setBuildFiles(QStringList& iBuildList) { m_buildFiles = iBuildList; }

    gtVector<kaProjectDataManagerAnaylzeData>& analyzeVector() { return m_analyzeDataVector; }

    /// get file build platform
    /// \returns the build platform
    const gtString& BuildPlatform() const { return m_buildPlatform; }

    /// set file build platform
    /// \param buildPlatform the build platform to be set
    void SetBuildPlatform(const gtString& buildPlatform);

    /// get file build profile
    /// \returns the build profile
    gtString& BuildProfile() { return m_buildProfile; }

    /// set file build profile
    /// \param buildProfile the build profile to be set
    void setBuildProfile(gtString buildProfile);

    /// get GL shader file build type
    /// \returns the build type
    const gtString& GetGLShaderType() const { return m_glShaderType; }

    /// set GL shader file build type
    /// \param glShaderType the GL shader build type to be set
    void SetGLShaderType(gtString glShaderType);

    /// get file entry point function
    /// \returns the entry point function
    gtString& EntryPointFunction() { return m_entryPointFunction; }

    /// set file entry point function
    /// \param entryPoint the entry point function to be set
    void SetEntryPointFunction(gtString entryPoint);

    /// Set attributes of the file by its name
    void SetAtributesByName();

    /// Set attributes of the file by its name
    void SetGLShaderTypeByFile();

    /// try to detect GL Shader type by searching in file name on of passed key words
    /// \param [in] fileName
    /// \param [in] keyWords list
    /// \param [out] typeDetected flag
    void DetectGLShaderTypeByKeywordsInFileName(const QString& fileName, const QStringList& keyWords, bool& typeDetected);

    /// file type setter
    void SetFileType(kaFileTypes fileType) { m_fileType = fileType; }

    /// file type getter
    kaFileTypes FileType() const { return m_fileType; }

    bool       IsCLShader() const;


private:
    /// File Full directory:
    osFilePath m_filePath;
    /// latest build directory:
    osDirectory m_buildDir;
    /// List of output build results
    QStringList m_buildFiles;

    gtVector<kaProjectDataManagerAnaylzeData> m_analyzeDataVector;

    /// build platform
    gtString m_buildPlatform;

    /// build profile
    gtString m_buildProfile;

    /// entry point function
    gtString m_entryPointFunction;

    /// GL shader type
    gtString m_glShaderType;

    /// file type
    kaFileTypes m_fileType;

    int m_id = -1;

};

class KA_API kaProjectDataManager : public QObject, public afIDocUpdateHandler
{
    Q_OBJECT

public:
    static kaProjectDataManager& instance();
    virtual ~kaProjectDataManager();

    /// write and read the data from xml
    bool getXMLSettingsString(gtString& projectAsXMLString);

    bool setSettingsFromXMLString(const gtString& projectAsXMLString, TiXmlNode* pMainNode);

    /// clear all data and delete them:
    void ClearAllAndDelete();

    const gtMap<int, kaSourceFile*>& GetSourceFilesMap() const { return m_sourceFileManager->GetSourceFilesMap(); }
    gtList<kaSourceFile*> GetSourceFilesByIds(const gtVector<int>& fileIds) const { return m_sourceFileManager->GetSourceFilesByIds(fileIds); }

    kaSourceFile* dataFileByPath(const osFilePath& iFilePath);

    kaSourceFile* AddFileOnProjectLoad(const osFilePath& iFilePath);
    bool removeFile(const osFilePath& iFilePath);

    /// Rename the file path from oldFilePath to newFilePath:
    /// \param oldFilePath the old file path
    /// \param newFilePath the new file path
    /// \param newDirPath will contain the new directory path (to be stored as build directory)
    bool RenameFile(const osFilePath& oldFilePath, const osFilePath& newFilePath, const osFilePath& newDirPath);

    /// build the kernel list for the file:
    void BuildFunctionsList(const osFilePath& iFilePath, kaSourceFile* pFileData);

    /// Set the build options:
    void setBuildOptions(const QString& buildOptions);


    /// Sets kernel build options to local member
    /// \param buildOptions build options
    void SetKernelBuildOptions(const QString& buildOptions);

    /// Set the Shader build options:
    /// \param buildOptions is the options to be set
    void SetShaderBuildOptions(const QString& buildOptions);

    /// set the strings in build option string to be ignored
    /// \param avoidableStrings strings list to be set
    void SetShaderAvoidableBuildOptions(const QStringList& avoidableStrings);

    /// Set the Shader compile type:
    /// \param compileType is the compile Type to be set
    void SetShaderCompileType(const QString& compileType);

    /// set the Shader D3d compiler path
    /// \param shaderBuilderPath is the Shader D3d compiler path
    void SetShaderD3dBuilderPath(const QString& shaderBuilderPath);

    /// set the Shader Fxc compiler path
    /// \param shaderBuilderPath is the Shader Fxc compiler path
    void SetShaderFxcBuilderPath(const QString& shaderBuilderPath);

    /// sets Shader predefined macros
    /// \shaderMacros is the Shader predefined macros
    void SetShaderMacros(const QString& shaderMacros);

    /// sets kernel predefined macros
    /// \shaderMacros is the Shader predefined macros
    void SetKernelMacros(const QString& kernelMacros);

    /// sets Shader additional include directories
    /// \param shaderIncludes is the Shader additional include directories
    void SetShaderIncludes(const QString& shaderIncludes);

    /// sets Shader build options mask for D3d compile type
    /// \param shaderD3dBuildOptionsMask is the Shader build options mask
    void SetShaderD3dBuildOptionsMask(const unsigned int shaderD3dBuildOptionsMask);

    /// Get the build options:
    /// \returns the build options
    QString& BuildOptions() { return m_buildCommandOptions; }

    /// Get the Shader build options:
    /// \returns the shader build options
    QString& ShaderBuildOptions() { return m_shaderBuildCommandOptions; }

    /// get the strings in build option string to be ignored
    /// \returns m_avoidableBuildOptions is the strings list to be ignored
    QStringList& ShaderAvoidableBuildOptions() { return m_avoidableBuildOptions; }

    /// Get the shader compile type
    /// \returns the shader compile type
    QString& ShaderCompileType() { return m_shaderCompileType; }

    /// get the Shader D3d compiler path
    /// \returns Shader D3d compiler path
    QString& ShaderD3dBuilderPath() { return m_shaderD3dBuilderPath; }

    /// get the Shader Fxc compiler path
    /// \returns Shader Fxc compiler path
    QString& ShaderFxcBuilderPath() { return m_shaderFxcBuilderPath; }

    /// get Shader predefined macros
    /// \returns Shader predefined macros
    const QString& ShaderMacros() const { return m_shaderMacros; }

    /// get Kernel predefined macros
    /// \returns Shader predefined macros
    const QString& KernelMacros() const { return m_kernelMacros; }
    /// get Shader additional include directories
    /// \returns Shader additional include directories
    QString& ShaderIncludes() { return m_shaderIncludes; }

    /// Get the build architecture (32-bit, 64-bit, etc.) of the active file
    AnalyzerBuildArchitecture GetBuildArchitecture() const { return m_buildArchitecture; }

    /// Set current file information
    void SetCurrentFileData(kaFileInformation fileInfoType, gtString& fileInfo);

    /// Get the file Info
    gtString CurrentFileInfo(kaFileInformation fileInfoType) const;

    /// String for the Shader build options mask for D3d compile type
    /// \returns Shader build options mask
    unsigned int ShaderD3dBuildOptionsMask() { return m_shaderD3dBuildOptionsMask; }

    /// Handler the document update event:
    virtual void UpdateDocument(const osFilePath& docToUpdate);

    /// Get the build platform enum
    kaPlatform GetBuildPlatform(const gtString& programName);

    /// get the file platform by its extension
    /// \param filePath is the file path + name + extension
    /// \returns the file platform (enum)
    kaPlatform GetPlatformByExtension(const osFilePath& filePath) const;

    /// GetGLShadersTypes
    /// \returns list of GL Shaders types
    const QStringList& GetGLShaderTypes()const { return m_glShaderTypesList; }


    /// GetGLShaderExtensions
    /// \returns list of GL Shader extensions
    const QStringList& GetGLShaderExtensions()const { return m_glShaderExtensions; }

    /// GetDXShaderExtensions
    /// \returns list of DX Shader extensions
    const QStringList& GetDXShaderExtensions()const { return m_dxShaderExtensions; }

    /// GetGLShaderDetectors
    /// \returns vector of lists of GL Shader detectors words
    const gtVector<QStringList>& GetGLShaderDetectors()const { return m_glShaderDetectorsVector; }

    /// Adds a program to the programs list
    /// \param [in] kaProgram*
    void AddProgram(kaProgram* pProgram) { m_programsList.push_back(pProgram); }

    /// Get the programs
    const gtPtrVector<kaProgram*>& GetPrograms() { return m_programsList; }

    /// Get program by its name
    kaProgram* GetProgram(const gtString& programName) const;

    //returns map : there key original file name and value list of binary files that were built
    typedef gtMap <osFilePath, gtList<osFilePath>> FPathToOutFPathsMap;
    void GetActiveProgramBuildFiles(const AnalyzerBuildArchitecture buildArch, FPathToOutFPathsMap& result);
    void GetActiveProgramBuildFiles(const osFilePath& filePath, const AnalyzerBuildArchitecture buildArch, FPathToOutFPathsMap& result);

    /// Remove a program pointer from the list of programs
    /// \param pProgram the program to delete
    /// \param shouldRemoveBinariesFromDisk should the output folders be deleted?
    void RemoveProgram(kaProgram* pProgram, bool shouldRemoveBinariesFromDisk);

    bool IsBuilt(const kaProgram* pProgram,  int fileId, const AnalyzerBuildArchitecture buildArch) const;
    bool IsBuilt(const kaProgram* pProgram, const AnalyzerBuildArchitecture buildArch) const;

    /// Returns ID of the file from files map
    /// \param [in] filePath
    int GetFileID(const osFilePath& filePath) const;
    gtList<int> GetFileIDs(const gtVector<osFilePath>& filePaths) const;

    /// Returns path of the file from files map
    /// \param [in] fileID
    /// \param [out] filePath
    void GetFilePathByID(int fileID, osFilePath& filePath) const;

    /// Returns file paths of the given file ids as std::set
    /// \param [in] fileIDs -
    std::set<osFilePath> GetFilePathsByIDs(const gtVector<int>& fileIDs) const;

    /// Creates a pair in kaSourceFileManager multimap
    /// \param [in] sourceFileId
    /// \param [in] pProgram
    void   Connect(int sourceFileId, kaProgram* pProgram);

    /// Removes a pair from kaSourceFileManager multimap
    /// \param [in] sourceFileId
    /// \param [in] pProgram
    void   Disconnect(int sourceFileId, kaProgram* pProgram);

    /// Sets active program
    /// \param pProgram


    /// Returns active program
    kaProgram* GetActiveProgram();

    /// Returns active programms
    void GetActiveProgramms(std::vector<kaProgram*>& programms);

    /// Sets last build program
    /// \param [in] pProgram
    void SetLastBuildProgram(kaProgram* pProgram);

    /// Gets last build program
    /// \return m_pLastBuildProgram
    std::unordered_map<wstring, kaProgram*>& GetLastBuildProgram() { return m_pLastBuildProgram; }

    bool IsRender(const kaProgram* pProgram) const;
    bool IsActiveProgramRender() const;
    bool IsActiveProgramPipeLine() const;
    bool IsProgramPipeLine(const kaProgram* program) const;
    bool IsActiveProgramGL() const;
    bool IsProgramGL(const kaProgram* program) const;

    /// get project build architecture
    /// \returns the build architecture (bitness)
    AnalyzerBuildArchitecture GetProjectArchitecture() const { return m_buildArchitecture; }

    /// get project build architecture as string
    void GetProjectArchitectureAsString(gtString& buildArchitecture)const;

    /// set project build architecture
    /// \param bitness the build architecture to be set
    void SetProjectArchitecture(AnalyzerBuildArchitecture buildArch);

    /// set project build architecture from string
    /// \param bitness project build architecture to be set
    void SetProjectArchitectureFromString(const gtString& buildArch);

    /// returns true if DX shader is currently selected in CodeXL explorer tree
    bool IsDXShaderSelected();

    /// returns true if CL file is currently selected in CodeXL explorer tree
    bool IsCLFileSelected();

    /// returns true if source pool file is selected
    bool IsSourceFileSelected();

    /// sets the default initial entry point or kernel and returns the function name
    /// \param [out] functionName
    void GetInitialEntryPointOrKernel(gtString& functionName)const;


    /// sets  entry point or kernel and returns line number
    /// \param [in] functionName
    /// \param [out] lineNumber
    void SetEntryPointOrKernel(const gtString& functionName, int& fileId, int& lineNumber, gtString& typeName)const;


    /// gets dx shader type by selected entry point
    /// \param [out] fileType
    void GetSelectedDXShaderType(gtString& fileType)const;

    /// opens source file on given line
    /// \param [in] line number
    void OpenSourceFileOnGivenLine(int lineNumber)const;

    /// returns last build files vector
    /// \return m_lastBuiltFiles
    const gtVector<osFilePath>& GetLastBuildFiles()const;

    /// sets last build files vector
    /// \param [in] lastBultFiles
    void SetLastBuildFiles(const gtVector<osFilePath>& lastBuiltFiles);

signals:
    void BuildOptionsChanged();

    /// signal for shader build options changed
    void ShaderBuildOptionsChanged();

    /// slot for shader compile type changed
    void ShaderCompileTypeChanged();

public slots:
    /// Handle save source
    void OnDocumentSaved(QString filePathAsStr);

private:
    friend class FillKernelNamesList_CheckAttributes_Test;
    /// service functions for building kernel list:
    kaProjectDataManagerAnaylzeData getKernelData(int iKernelIndex, kaSourceFile* pCurrentFileData);
    void addKernelData(kaProjectDataManagerAnaylzeData& iKernelData, kaSourceFile* pCurrentFileData);
    void changeKernelDataAtIndex(kaProjectDataManagerAnaylzeData& iKernelData, int iKernelIndex, kaSourceFile* pCurrentFileData);
    void removeKernelByIndex(int iKernelIndex, kaSourceFile* pCurrentFileData);
    /// build kernels list for a text file based on .cl file format
    /// \param in text file source
    /// \param in pCurrentFileData file data into which to merge
    /// \return true if at least one kerenl found in source file
    bool buildKernelList(const QString& fileData, const osFilePath& filePath, kaSourceFile* pCurrentFileData) const;

    /// Fills kernel list from given source file, i.e. we do macro expansion for given file, parse file after it, locate all kernel functions and fill those functions into output parameter
    /// \param in fileData - given file content
    /// \param in filePath - given file location
    /// \param in additionalMacros during file macro preprocessing this macros list used for as reference
    /// \param out kernelList kernel list, that was extracted from given file , after macro expansion
    /// \return true if found kernel function in file
    static bool FillKernelNamesList(const QString& fileData, const osFilePath& filePath, const std::vector<std::string>& additionalMacros, gtVector<kaProjectDataManagerAnaylzeData>& kernelList);
    static bool AddMangledKernelName(std::vector<PreProcessedToken>::iterator& token, std::vector<PreProcessedToken>& tokens, gtVector<kaProjectDataManagerAnaylzeData>& kernelList);

    /// build entry point list for a text file based on .hlsl file format
    /// \param in text file source
    /// \param in pCurrentFileData file data into which to merge
    bool buildEntryPointList(QString& text, kaSourceFile* pCurrentFileData);


    /// remove comments from shader source file to prevent entry points detection failure
    void RemoveCommentsPreservingNewLines(const QString& originalText, QString& textWithoutComments)const;

    void findKernelName(int& kernelNameStartingPosition, const QString& text, gtVector<QString>& kernelsFound, gtVector<QString>& templateToKernelVector, gtVector<int>& kernelsFoundLineNumber);
    bool IsTemplateKernel(int& kernelNameStartingPosition, int kernelWordWidth, const QString& text, gtVector<QString>& templatesFound, gtVector<int>& templatesFoundLineNumber);
    bool IsTemplateAttributeKernel(int& kernelNameStartingPosition, int kernelWordWidth, const QString& text, gtVector<QString>& kernelsFound, gtVector<QString>& templateToKernelVector, gtVector<int>& kernelsFoundLineNumber);
    bool isOnlyWhiteSpaces(int stat, int end, const QString& text);
    inline bool isNotWhiteSpace(QChar& aChar) { bool retVal = false; if ((aChar != 9) && (aChar != 10) && (aChar != 13) && (aChar != 32)) { retVal = true; } return retVal; };

    /// Update The existing list of functions that can be kernels or entry points based on the found list of kernels or entry points list:
    /// takes the existing list of kernels or entry points and merge them with the functionsFound list (also syncing the line number list using the functionsFoundLineNumber)
    /// \param in functionsFound new found function names
    /// \param in functionsFoundLineNumber function line numbers
    /// \param in pCurrentFileData file data into which to merge
    void UpdateFunctionsList(gtVector<QString>& functionsFound, gtVector<int>& functionsFoundLineNumber, kaSourceFile* pCurrentFileData);

    /// Remove block comments: comments starting with /* and ending with */
    void RemoveBlockComments(QString& text);

    /// Remove line Comments: comments starting with // until the end of line
    void RemoveLineComments(QString& text);
    /// Only my instance() function can create me:
    kaProjectDataManager();

    /// get the file platform by its extension
    /// \param filePath is the file path + name + extension
    kaPlatform GetPlatformByFileName(const osFilePath& filePath) const;
    void DeserializeProgram(TiXmlNode* pMainNode);

    /// Only kaSingletonsDelete should delete me:
    friend class kaSingletonsDelete;

    /// The single instance of this class:
    static kaProjectDataManager* m_psMySingleInstance;

    /// files added to the manager:

    std::unique_ptr<kaFileManager> m_sourceFileManager;

    /// String for the build command toolbar:
    QString m_buildCommandOptions;

    /// String for the Shader build command toolbar
    QString m_shaderBuildCommandOptions;

    /// String for the Shader compile type
    QString m_shaderCompileType;

    /// string in build options to be ignored
    QStringList m_avoidableBuildOptions;

    /// String for the Shader D3d compiler path
    QString m_shaderD3dBuilderPath;

    /// String for the Shader Fxc compiler path
    QString m_shaderFxcBuilderPath;

    /// String for the Shader predefined macros
    QString m_shaderMacros;

    /// String for the kerenl predefined macros
    QString m_kernelMacros;
    /// String for the Shader additional include directories
    QString m_shaderIncludes;

    /// String for the Shader build options mask for D3d compile type
    unsigned int m_shaderD3dBuildOptionsMask;

    static QList<QRegExp> m_sKeywordsList;

    /// List of GL shaders types
    QStringList m_glShaderTypesList;

    /// Vector of GL shaders detectors
    gtVector <QStringList> m_glShaderDetectorsVector;

    /// List of GL shaders extensions
    QStringList m_glShaderExtensions;

    /// List of DX shaders extensions
    QStringList m_dxShaderExtensions;

    /// Programs added to the manager:
    gtPtrVector<kaProgram*> m_programsList;

    /// build architecture (bitness)
    AnalyzerBuildArchitecture m_buildArchitecture;

    /// vector of last build files
    gtVector<osFilePath> m_lastBuildFiles;

    /// maps last built program name to last built program
    std::unordered_map<std::wstring, kaProgram*> m_pLastBuildProgram;
};
#endif //__KAPROJECTDATAMANAGER_h


