//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSessionController.h
///
//==================================================================================

//------------------------------ ppSessionController.h ------------------------------

#ifndef __PPSESSIONCONTROLLER_H
#define __PPSESSIONCONTROLLER_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// DB data types
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

// Powerprofiler midtier classes
#include <AMDTPowerProfilingMidTier/include/PowerProfilerBL.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Qt:
//#include <qstring.h>
//#include <qcolor.h>
#include <QtCore>

// Local.
#include <AMDTPowerProfiling/src/ppColors.h>

#define PP_TIMELINE_GRAPHS_DEFAULT_INITIAL_RANGE 10000  // in milliseconds

// ----------------------------------------------------------------------------------
// Class Name:          ppSessionController
// General Description: A class controlling a power profiling session. The class provides
//                      data required by the view.
// Author:              Sigal Algranaty
// Creation Date:       14/10/2014
// ----------------------------------------------------------------------------------
class PP_API ppSessionController
{

public:
    // Constructor:
    ppSessionController();
    virtual ~ppSessionController();

    enum SessionState
    {
        PP_SESSION_STATE_NEW = 0,
        PP_SESSION_STATE_RUNNING,
        PP_SESSION_STATE_COMPLETED
    };

    /// Open the database:
    void OpenDB();

    /// Close the DB connection:
    void CloseDB();

    /// Set the session state:
    void SetState(SessionState state) { m_currentSessionState = state; };

    /// Set the session state:
    void SetDBFilePath(const osFilePath& dbPath) { m_dbFilePath = dbPath; };

    // Get the session path:
    const osFilePath& DBFilePath() { return m_dbFilePath; };

    /// Accecssors:
    ppSessionController::SessionState GetSessionState() const { return m_currentSessionState; };

    /// DB / Offline data accessors:
    void GetSessionTimeRange(SamplingTimeRange& samplingTimeRange);

    PowerProfilerBL& GetProfilerBL() { return m_powerProfilingBL; }

    /// Counters functions:
    QString GetCounterNameById(int counterId);

    /// Get the counter ID by its name:
    /// \param counterName the counter name
    /// \return the counter id or -1 if not found
    int GetCounterIDByName(const QString& counterName) ;

    /// check if the counter units are percent
    /// \param counterId is the counter id
    /// \returns true if the counter units are percent units
    bool IsCounterInPercentUnits(int counterId);

    /// gets the color for the counter graph
    /// \param counterId is the counter id
    /// \returns the color for the counter
    QColor GetColorForCounter(int counterId);

    /// does the counter has a parent counter
    /// \param counterId is the child counter  id
    /// \returns true if the counter is a child counter
    bool IsChildCounter(int counterId);

    /// gets the parent counter name
    /// \param counterId is the child counter  id
    /// \returns the parent counter name
    QString GetCounterParent(int counterId);

    /// gets the description of the counter
    /// \param counterId is the id of the counter
    /// \returns the description for the counter
    QString GetCounterDescription(int counterId);

    /// Get enabled counters in a specified DB by category and type. Used to get counters for graph:
    /// \param searchTypeVector is the input searched type
    /// \param searchCategory is the input searched category
    /// \param countersVector is the output vector
    void GetEnabledCountersByTypeAndCategory(const gtVector<AMDTDeviceType>& searchTypeVector, AMDTPwrCategory searchCategory, gtVector<int>& countersVector);

    /// Ge the device type that corresponds to the device Id. If the Play button was clicked the data is
    /// retrieved from the DB, otherwise it is retrieved from the backend.
    bool GetDeviceType(int deviceId, AMDTDeviceType& deviceType);

    /// APU Counter:
    int GetAPUCounterID();

    /// Sampling time interval:
    unsigned int GetSamplingTimeInterval();

    /// Get the tree of system devices from the backend and cache it for use in later calls to this function.
    const gtList<PPDevice*>& GetSystemDevices();

    /// Get the list of counter descriptors from the source that is applicable to the session state:
    /// If session is complete - get the counter descriptors from the DB.
    /// Otherwise - get the counter descriptors from the Power Profiler backend.
    /// In both cases the descriptors are cached so additional calls to this function will retrieve
    /// them from the local cache.
    const gtMap<int, AMDTPwrCounterDesc*>& GetAllCounterDescriptions();

    /// Get the descriptor for a specific counter
    const AMDTPwrCounterDesc* GetCounterDescriptor(int counterId);

    /// Close all the database connections:
    void CloseDBConnections() { m_powerProfilingBL.CloseAllConnections(); }


protected:

    /// Get session counters from DB information
    /// \param deviceTypes is a list of counter types
    /// \param counterCategory is the counter category
    /// \param counterIds is the output list of counter ids
    void GetSessionCountersFromDB(const gtVector<AMDTDeviceType>& deviceTypes, AMDTPwrCategory counterCategory, gtVector<int>& counterIds);

    /// Midtier BL class
    PowerProfilerBL m_powerProfilingBL;

    /// Contain the current session state:
    SessionState m_currentSessionState;

    /// Contain the DB file path:
    osFilePath m_dbFilePath;

    /// Local cache for the tree of system devices that is retrieved from the back end
    gtList<PPDevice*> m_systemDevices;

    /// Local cache for the list of all counter descriptors that were retrieved from the backend.
    gtMap<int, AMDTPwrCounterDesc*> m_allCounterDescriptions;

    // Stores the details of all enabled counters as a cache to data retrieved from the DB.
    gtMap<int, AMDTPwrCounterDesc*> m_allCountersDetailsDbCache;

    // Cache for the APU Power counter id.
    int m_apuCounterId;

};

#endif //__PPSESSIONCONTROLLER_H
