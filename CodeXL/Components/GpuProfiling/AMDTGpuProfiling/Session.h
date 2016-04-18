//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Session.h $
/// \version $Revision: #41 $
/// \brief :  This file contains GPUSessionTreeItemData class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Session.h#41 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#ifndef _SESSION_H_
#define _SESSION_H_

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QStringList>
#include <QMap>
#include <QList>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>

// Local:
#include <AMDTGpuProfiling/OccupancyInfo.h>
#include <AMDTGpuProfiling/ProjectSettings.h>
#include <AMDTGpuProfiling/Util.h>


/// Store each session settings
class AMDT_GPU_PROF_API GPUSessionTreeItemData : public SessionTreeNodeData
{
    Q_OBJECT

public:

    /// Default constructor
    GPUSessionTreeItemData();

    /// Initializes a new instance of the GPUSessionTreeItemData class
    /// \param strName the session name
    /// \param strWorkingDirectory the project's working directory
    /// \param strSessionFilePath the session file path
    /// \param strProjName project name
    /// \param profileType the type of profile performed
    /// \param isImported is the session imported
    GPUSessionTreeItemData(const QString& strName,
                           const QString& strWorkingDirectory,
                           const QString& strSessionFilePath,
                           const QString& strProjName,
                           GPUProfileType profileType,
                           bool isImported);

    /// Copy constructor
    /// \param other the other session to copy from
    GPUSessionTreeItemData(const GPUSessionTreeItemData& other);

    /// Destructor
    virtual ~GPUSessionTreeItemData();

    /// Sets the session display name into the property and file
    /// \param displayName the new display name
    /// \param[out] errorMessage error string
    /// \return true if successful, false otherwise
    bool UpdateDisplayNameInOutputFile(const QString& displayName, QString& errorMessage);

    /// Sets the session display name (overrides SessionTreeNodeData)
    /// \param displayName the new display name
    virtual void SetDisplayName(const QString& displayName);

    /// Gets the type of profile for this session
    /// \return the type of profile for this session
    GPUProfileType GetProfileType() const;

    /// Gets the Occupancy file name, AdditionalFiles list includes occupancy file
    /// \return the Occupancy file name
    QString GetOccupancyFile() const;

    /// Gets the number of properties in the session file
    /// \return the number of properties in the session file
    int GetPropertyCount();

    /// Gets the specified property's value for this session.
    /// writes the properties in the same order in the result file
    /// \param[in] strPropName the property whose value is needed
    /// \param[out] strPropValue the value of the requested property
    /// \return true if the property is found, false otherwise
    bool GetProperty(const QString& strPropName, QString& strPropValue);

    /// Gets the major version number of the session file
    /// \return the major version number of the session file
    int GetVersionMajor() const;

    /// Gets the minor version number of the session file
    /// \return the minor version number of the session file
    int GetVersionMinor() const;

    /// Gets the group name, if project name exists, return project name, otherwise, return exe name if exists
    /// \return the group name, if project name exists, return project name, otherwise, return exe name if exists
    QString GetGroupName() const;

    /// Occupancy file accessors:
    void SetOccupancyFile(const QString& occupancyFile) {m_occupancyFile = occupancyFile;}
    QString OccupancyFile() const {return m_occupancyFile;}

    /// Gets the occupancy table map. From thread ID to OccupancyInfo list
    /// \return the occupancy table map. From thread ID to OccupancyInfo list
    const OccupancyTable& LoadAndGetOccupancyTable();

    /// Clears any data associated with this session
    virtual void FlushData();

    /// Search for additional files for this session
    /// This function will be called
    /// 1. When a new session is created.
    /// 2. When we load previous session.
    /// If new session-related files are generated after session object has been created,
    /// use AddAdditionalFile to add them.
    void SearchForAdditionalFiles();

    /// Add additional file to the session
    /// \param file file to be added
    void AddAdditionalFile(const QString& file);

    /// Gets the list of additional files for this session
    /// \return the list of additional files for this session
    const QStringList& GetAdditionalFiles();

    /// Sets the list of additional files for this session
    void SetAdditionalFiles(const QStringList& additionalFilesList);

    /// Rewrites the session file, updating the properties section
    /// \param errorMessage error string returned if function fails
    /// \return true if successful, false otherwise
    bool UpdatePropertiesSection(QString& errorMessage);

    /// Indicates whether or not the session file is valid (has a required property)
    /// \return a flag indicating whether or not the session file is valid (has a required property)
    bool IsSessionFileValid();

    /// Sets the additional files and output files folder
    /// \param newFolder the session new folder
    void SetFilesFolder(const gtString& newFolder);

    /// Resets the occupancy flag load flag:
    void ResetOccupancyFileLoad() {m_occupancyFileLoadExecuted = false;}

    /// Gets the property that is holding the API (CL/ HSA):
    /// \return the api to trace
    virtual APIToTrace GetAPIToTrace() const { return m_sessionAPIToTrace; }

    /// Gets occupancy table map. From thread ID to list of occupancy info
    OccupancyTable& GetOccupancyTable() { return m_occupancyTable; }

    /// Return the session CSV file. In VS this it return the mapped file
    void GetSessionCSVFile(osFilePath& sessionCSVFile) const;

protected:
    /// Checks whether a given file should be considered an additional file for this session
    /// \param fileInfo the file to check
    /// \return true, if the file is an additional file for this session, false otherwise
    virtual bool IsAdditionalFile(const QFileInfo& fileInfo);

    /// Clears the loaded property list, resets the associated members, and then rereads the properties from disk
    virtual void ResetProperties();

    /// Gets the property that is required to be in the session in order for it to be considered "valid"
    /// \return the property that is required to be in the session in order for it to be considered "valid".
    virtual QString GetValidSessionProperty() const { return ""; }

    QStringList m_additionalFiles; ///< Additional files that are associated with this session

private:

    /// Disable assignment operator
    /// \param obj the input object
    /// \return a reference of the object
    GPUSessionTreeItemData& operator= (const GPUSessionTreeItemData& obj);

    /// Load properties, set flag to true, save file pointer
    /// \return True if property section is loaded
    bool LoadProperties(const QString& strSessionFilePath);

    /// Parse property from input string
    /// \param input input string
    void AddProperty(const QString& input);

    bool                               m_isPropertySectionLoaded; ///< Flag indicating if the properties section has been loaded
    int                                m_propertyLinesCount;      ///< Number of lines for property section
    bool                               m_propertyAdded;           ///< Flag indicating that a property was added
    bool                               m_occupancyFileIsLoaded;   ///< Value indicating whether occupancy file is loaded
    bool                               m_occupancyFileLoadExecuted;///< Value indicating whether occupancy file load was executed
    GPUProfileType                     m_profileType;             ///< Gets the type of profile for this session
    QString                            m_occupancyFile;           ///< the Occupancy file name, AdditionalFiles list includes occupancy file
    QList<QPair<QString, QString> >    m_properties;              ///< Gets the properties for this session. This is a QList of QPairs so that order of the properties is retained
    int                                m_versionMajor;            ///< Gets major version number
    int                                m_versionMinor;            ///< Gets minor version number
    OccupancyTable                     m_occupancyTable;          ///< Gets occupancy table map. From thread ID to list of occupancy info
    APIToTrace                         m_sessionAPIToTrace;       ///< Gets the API to trace. Currently applicable only for GPU perfcounters
};

/// Class representing API trace session
class TraceSession : public GPUSessionTreeItemData
{

    Q_OBJECT

public:
    /// Initializes a new instance of the TraceSession class.
    /// \param strName the session name
    /// \param strWorkingDirectory the project's working directory
    /// \param strSessionFilePath the session file path
    /// \param strSessionOutputFile the session's output file
    /// \param strProjName project name
    /// \param isImported is the session imported
    TraceSession(const QString& strName,
                 const QString& strWorkingDirectory,
                 const QString& strSessionFilePath,
                 const QString& strProjName,
                 bool isImported);

    /// Destructor
    virtual ~TraceSession();

    /// Gets the perf marker file-name
    /// \return perf marker file-name
    QString GetPerfMarkerFile() const { return m_perfMarkerFile; }

    /// Gets the list of excluded APIs for this session
    /// \param[out] excludedAPIsList the list of excluded APIs
    /// \return true if there are excludedAPIs for this session, false otherwise
    bool GetExcludedAPIs(QStringList& excludedAPIsList);

    /// Clears any data associated with this session
    virtual void FlushData();

protected:
    /// Checks whether a given file should be considered an additional file for this session
    /// \param fileInfo the file to check
    /// \return true, if the file is an additional file for this session, false otherwise
    virtual bool IsAdditionalFile(const QFileInfo& fileInfo);

    /// Clears the loaded property list, resets the associated members, and then rereads the properties from disk
    virtual void ResetProperties();

    /// Gets the property that is required to be in the session in order for it to be considered "valid"
    /// \return the property that is required to be in the session in order for it to be considered "valid".
    virtual QString GetValidSessionProperty() const;

private:

    QString                         m_perfMarkerFile;      ///< Performance marker file
    bool                            m_exlcudedAPIsChecked; ///< Flag indicating whether or not the excluded APIs have been checked
    QStringList                     m_excludedAPIs;        ///< List of APIs excluded for this session
};

/// Class representing Performance counter GPUSessionTreeItemData
class PerformanceCounterSession : public GPUSessionTreeItemData
{
    Q_OBJECT

public:
    /// Initializes a new instance of the PerformanceCounterSession class.
    /// \param strName the session name
    /// \param strWorkingDirectory the project's working directory
    /// \param strSessionFilePath the session file path
    /// \param strSessionOutputFile the session's output file
    /// \param strProjName project name
    /// \param isImported is the session imported
    PerformanceCounterSession(const QString& strName,
                              const QString& strWorkingDirectory,
                              const QString& strSessionFilePath,
                              const QString& strProjName,
                              bool isImported);

    /// Destructor
    virtual ~PerformanceCounterSession();

    /// Gets the session temporary file path
    osFilePath GetSessionTempFile() const {return m_sessionTemporaryFile;};

    /// Sets the session temporary file path
    /// \param sessionFile  the session file
    void SetSessionTempFile(const osFilePath& sessionFile) {m_sessionTemporaryFile = sessionFile;};

    /// Is called after a session is renamed. The function renames the temporary file from: oldName.gpsession to newName.gpsession
    void UpdateRenamePCTmpFile();

    /// Override the session (in VS we should go to the session csv file, and not to the gpsession temp file)
    /// \return the session directory in which the session files are located
    virtual osDirectory SessionDir() const;


protected:
    /// Checks whether a given file should be considered an additional file for this session
    /// \param fileInfo the file to check
    /// \return true, if the file is an additional file for this session, false otherwise
    virtual bool IsAdditionalFile(const QFileInfo& fileInfo);

    /// Gets the property that is required to be in the session in order for it to be considered "valid"
    /// \return the property that is required to be in the session in order for it to be considered "valid".
    virtual QString GetValidSessionProperty() const;

private:
    osFilePath m_sessionTemporaryFile;
};


class AMDT_GPU_PROF_API gpSessionTreeNodeData : public GPUSessionTreeItemData
{
    Q_OBJECT

public:
    /// Initializes a new instance of the PerformanceCounterSession class.
    /// \param strName the session name
    /// \param strWorkingDirectory the project's working directory
    /// \param strSessionFilePath the session file path
    /// \param strSessionOutputFile the session's output file
    /// \param strProjName project name
    /// \param isImported is the session imported
    gpSessionTreeNodeData(const QString& strName,
                          const QString& strWorkingDirectory,
                          const QString& strSessionFilePath,
                          const QString& strProjName,
                          bool isImported);

    virtual ~gpSessionTreeNodeData();

    /// HTML content title (override for non-profile items)
    /// htmlContent the HTML content
    virtual void StartSessionHTML(afHTMLContent& htmlContent) const;

    /// Delete session files
    /// \param report Report of deletion, one line for each item
    /// \return true if could delete
    virtual bool DeleteSessionFilesFromDisk(QString& report);

    /// Contain a vector of captured frame indices
    QVector<int> m_capturedFramesIndices;

    /// Contain the specific tree item file path (timeline / profile / etc')
    osFilePath m_frameFilePath;

    /// Frame index
    int m_frameIndex;

    QString m_strSessionFilePath;
};


#endif // _SESSION_H_

