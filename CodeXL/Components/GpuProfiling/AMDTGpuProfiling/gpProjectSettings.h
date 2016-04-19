//------------------------------ gpProjectSettings.h ------------------------------

#ifndef __GPPROJECTSETTINGS_H
#define __GPPROJECTSETTINGS_H


// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>

struct gpPerfCounter
{
    int m_id;
    QString m_name;
    QString m_description;
    QString m_parent;
    QString m_dataType;
    QString m_usage;
};

#define GP_DEFAULT_PORT 8080
#define GP_MAX_PORT_NUMBER 65535

// ----------------------------------------------------------------------------------
// Class Name:           gpProjectSettings
// General Description:  This class contain the data of the frame analysis project settings
// ----------------------------------------------------------------------------------
class gpProjectSettings
{
public:
    gpProjectSettings();
    ~gpProjectSettings();

    void Initialize();

    /// Enumeration defining the user API connection preference
    enum eConnectionType
    {
        egpProcessConnection = 0,
        egpFirstDXInProcessConnection,
        egpFirstDX12Connection
    };

    /// should the connection be automatic (-1 marks that is should be marked as should but should not since this is the first time it is connected)
    int m_shouldConnectAutomatically;

    /// type of connection selected by user
    eConnectionType m_connection;

    /// Port used for connection to the server
    unsigned short m_serverConnectionPort;

    /// connect to process number
    QString m_processNumber;

    /// First DX12 in this process
    QString m_processName;

    /// lists of preset counters
    std::map<QString, QStringList>& PresetCountersLists() { return m_presetCountersLists; };

    /// Get the counters list from the server
    const std::map<QString, gpPerfCounter*>& CountersMap() const { return m_countersMap; };

    /// Saves the presets lists of counters sets. This is needed when a users creates a new set through the counters selection dialog
    void SavePresetCountersLists();

    /// Add a counter to the list of selected counters
    /// \param counterId the counter to add
    void AddSelectedCounter(int counterId);

    /// Get the vector of selected counters
    const gtVector<int>& SessionSelectedCounters() const { return m_sessionSelectedCounters; };

    /// Get counters information:
    /// Counters available in the server. Preset lists and in those lists remove counters that are not available in the server.
    void GetDeviceCountersInformation();

    /// Loads the presets lists of counters sets.
    void LoadPresetCountersLists();

private:

    /// lists of preset counters
    std::map<QString, QStringList> m_presetCountersLists;

    /// counters list from server
    std::map<QString, gpPerfCounter*> m_countersMap;

    /// Will contain the list of selected counters IDs
    gtVector<int> m_sessionSelectedCounters;
};


#endif //__GPPROJECTSETTINGS_H

