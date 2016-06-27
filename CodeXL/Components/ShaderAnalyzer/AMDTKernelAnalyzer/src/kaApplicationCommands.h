//------------------------------ kaApplicationCommands.h ------------------------------

#ifndef __KAAPPLICATIONCOMMANDS_H
#define __KAAPPLICATIONCOMMANDS_H

// Qt
#include <QString>

// std:
#include <set>
#include <string>
using std::set;
using std::string;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>

class afApplicationTree;
class QTreeWidgetItem;
class kaTreeDataExtension;
class afQMdiSubWindow;

// ----------------------------------------------------------------------------------
// Class Name:          kaApplicationCommands
// General Description: executes the KA commands either from the main menu or the tree
//                      context menu (with the updateUI functionality also)
//
// Author:              Gilad Yarnitzky
// Creation Date:       22/8/2013
// ----------------------------------------------------------------------------------
class KA_API kaApplicationCommands
{
public:
    static kaApplicationCommands& instance();
    virtual ~kaApplicationCommands();

    /// Is executing the add file command. The command opens an add file dialog, and return the created file path, and the associated program for this file
    void AddFileCommand(gtVector<osFilePath>& addedFilePaths);

    void onUpdateAddFileCommand(bool& isEnabled);

    /// Perform the new OpenCL file command:
    /// \param shouldForceProjectCreation when true, the function will always create default new project
    /// \param [out] newFilePath the created file path
    /// \param [in + out] pAssociatedProgram the requested program on which the file should be attached, or the output program if the user selected
    /// \param stage the stage to which the program will be associated (for rendering programs)
    void NewFileCommand(bool shouldForceProjectCreation, osFilePath& newFilePath, kaProgram*& pAssociatedProgram, kaPipelinedProgram::PipelinedStage stage);

    /// Perform the new program command:
    /// \param shouldForceProjectCreation when true, the function will always create default new project
    kaProgram* NewProgramCommand(bool shouldForceProjectCreation);

    /// Add an OpenCL file:
    /// \param clFilePath the cl file path
    /// \param displayExistingFileErr if to display error when a file to add already exist
    /// \return true iff the operation succeeded
    bool AddSourceFile(const osFilePath& clFilePath, bool displayExistingFileErr = true);

    /// Is handling the activate of source files tree item:
    /// \param pSourceFileItemData is representing the tree item data for the source
    /// \param lineNumber the line in which the file should be opened
    /// \param forceopen will force opening the file and not check if there is an mdi file open and use it
    void OpenSourceFile(kaTreeDataExtension* pSourceFileItemData, int lineNumber, bool forceopen = true);

    /// Parse an ISA file content to its sections:
    /// \param isaFilePath the ISA file path
    /// \param dsamText[out] the disassembly section
    /// \param scShaderText[out] SC shader section
    /// \param csDataText[out] CS data section
    /// \return true iff the found and opened, and the 3 sections found and extracted
    bool ParseISAFile(const osFilePath& isaFilePath, QString& dsamText, QString& scShaderText, QString& csDataText);

    void buildCommand(const gtVector<osFilePath>& filePathsVector);

    void BuildSingleProgram(kaProgram* pProgram, const gtVector<osFilePath>& filePathsVector);

    void buildProgramCommand(kaProgram* pProgram);

    /// Cancels the current build.
    void cancelBuildCommand();

    void onUpdateBuildCommand(bool& isEnabled);

    /// enables command if build is in progress
    /// \param [out] isEnabled
    void onUpdateCancelBuildCommand(bool& isEnabled);

    void onUpdateKAModeCommand(bool& isEnabled, bool& isActionChecked);

    void openUpdateContainingFolder(bool& isEnabled);

    /// check if the mdi is a cl file:
    bool isMDIWindowIsCLFile(osFilePath& filePath);

    /// Get the current active files path
    unsigned int activeCLFiles(gtVector<osFilePath>& filesPath);

    /// Sets the KA settings dialog commands builds text edit control to be modified when the toolbar is changed
    /// This is a patch until the mechanism is updated
    void SetCommandTextEdit(QTextEdit* pTextEdit) { m_pSettingCommandTextEdit = pTextEdit; }

    /// Set and get the build options
    void setBuildOptions(const QString& buildOptions);

    void SetToolbarBuildOptions(const QString& buildOptions);

    /// Create the specific file subdir path based on a supplied filePath
    /// \param in fileNamePath file name path with the file name to base the new output dir on
    /// \param in\output the generated path do add the file name to
    void AppendFileSubPath(const osFilePath& fileNamePath, osFilePath& outputPath) const;

    /// Update all views related to specific file
    void updateOpenViews(const osFilePath& filePath);

    /// Creates a default file to the current project:
    osFilePath CreateDefaultFile(QString& fileNameFromUser, QString& fileExt, QString filePlatform = QString());

    void CreateDefaultKernelFile(const osFilePath& selectedNewFile, QString& fileExt, QString filePlatform = QString());

    /// Adds cl files from given path to project
    /// \param[in] sourceDir  path container separated by semicolons
    void AddCLFilesToProject(const gtString& sourceDirsString);

    /// Adds open cl files to tree
    /// \param[in] CLFilesToAdd files to add
    /// \param[in] displayExistingFileErr if to display error when a file to add already exist
    void AddFilesToTree(const gtVector<osFilePath>& clFilesToAddVector, bool displayExistingFileErr = true);

    /// Activate the MDI tree item taking into account the special case of devices.cxltxt file
    /// \param[in] filePath - the file path that the tree node is associate with
    void ActivateMDITreeItem(const osFilePath& filePath);

    /// Get selected devices in the format backend manager needs:
    void getSelectedDevices(set<string>& selectedDeviceName) const;

    /// get default d3d compiler dll file path
    /// \param (out) filePathStr is the default dll file path as string
    void GetD3DCompilerDefaultDllPath(QString& filePathStr);

    /// Check save for each sub window that holds the specific file path
    /// the af function can not be used since it dose not know the ka multi source view
    /// This function should save the first modified instance of the source.
    /// \param filePath the file path which is supposed to be saved
    void SaveAllMDISubWindowsForFilePath(const osFilePath& filePath);

    /// Calculate the path for the output directory for filePath:
    /// \param filePath the kernel / shader file path
    /// \param shouldCreate should the directory be created?
    osFilePath GetOutputDirectoryFilePath(const osFilePath& filePath, bool shouldCreate);

    /// detect DX shader profile from entry point name
    /// \param [in] entryPoint
    /// \param [out] profileCandidate
    void DetectDXShaderProfileFromEntryPointName(const QString& entryPoint, gtString& profileCandidate);

    /// Creates default program and adds sequential number
    /// \param programType  identifies the user selected type
    /// \return pointer to created program
    kaProgram* CreateDefaultProgram(kaProgramTypes programType);

    /// Creates new program name, program itself  and its containing folder
    /// returns new created program 
    /// \param programType based on programType  we create corresponding program name
    kaProgram* CreateProgram(const kaProgramTypes programType) const;

    /// Get the output directory based on the filePath
    osFilePath OutputFilePathForCurrentProject() const;

private:
    /// returns chosen program type followed by serial number
    /// \param currentName program name
    QString FindDefaultProgramName(const QString& currentName) const;

    ///saves given program source files to disk and erases this program previous output files
    void UpdateProgramInputAndOutputFiles(const gtVector<osFilePath>& filePathsVector, kaProgram* pProgram, const bool eraseAll = false);
    /// build non rendering programs
    void buildNonRendering(const gtVector<osFilePath>& filePathsVector, kaProgram* pProgram);

    /// Check if the build options specify OpenCL 2.0 support
    bool isOpenCL2Build() const;

    /// Only my instance() function can create me:
    kaApplicationCommands();

    /// set the target for the new file by its extension. set the data manager for the current file
    /// \param filePath is the new file path
    void SetNewFileProfile(const osFilePath& filePath, const QString& entryPoint = "");

    /// Erase program related output files. if a file path is supplied then only files related to that file path will be erase
    /// if not all output dir will be erased. Bitness of the program will be used to select right output dir to erase
    void EraseProgramFilePathOutput(kaProgram* pProgram, const osFilePath& filePath, const bool eraseAll = false);

    /// Only kaSingletonsDelete should delete me:
    friend class kaSingletonsDelete;

    /// The single instance of this class:
    static kaApplicationCommands* m_psMySingleInstance;

    /// last used path to add file command
    osFilePath m_lastAddFilePath;

    /// pointer the the KA project setting command text edit:
    QTextEdit* m_pSettingCommandTextEdit;

    /// Last activated MDI tree item file Path
    osFilePath m_MDITreeItemfilePath;

    /// All possible kernel/shader extensions list
    QStringList m_extensionsList;

    /// DX shaders profiles list
    QStringList m_dxShaderProfilesList;
};

#endif // __KAAPPLICATIONCOMMANDS_H