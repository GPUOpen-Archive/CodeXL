//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterGroup.h $
/// \version $Revision: #5 $
/// \brief :  This file contains CounterGroup class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterGroup.h#5 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _COUNTERGROUP_H_
#define _COUNTERGROUP_H_

#ifdef _WIN32
    #pragma warning(push, 1)
#endif
#include <QtCore>
#ifdef _WIN32
    #pragma warning(pop)
#endif


/// This class represent a group of counters.  It contains a group name and
/// a list of counter names in this group.
class CounterGroup
{
public:
    /// Initializes a new instance of the CounterGroup class
    /// \param strName the group name
    CounterGroup(const QString& strName);

    /// Add a counter name to the internal list
    /// \param strCounterName the counter name
    void AddCounterName(const QString& strCounterName);

    /// Get the total count of the counter names in the group
    /// \return the number of counters
    int GetCount() const;

    /// Get the counter name based on an index of the list
    /// \param index an index of the list
    /// \param strCounterName the output counter name
    /// \return true if successful, false otherwise
    bool GetCounterName(int index, QString& strCounterName);

    /// Get counter group name
    /// \return counter group name
    const QString GetCounterGroupName() const { return m_groupName; }

    /// typedef for the function pointer passed into SortCounters
    typedef void(*SortFunc)(QStringList&);

    /// Sort the counters using the provided sort function
    /// \param sortFunc the function to call with the list of counters
    void SortCounters(SortFunc sortFunc);

private:
    /// Disable copy constructor
    /// \param obj obj
    CounterGroup(const CounterGroup& obj);

    /// Disable assignment operator
    /// \param obj class object
    /// \return left hand side object
    CounterGroup& operator = (const CounterGroup& obj);

    QStringList m_counterNames; ///< A list of counter names
    QString     m_groupName;    ///< Counter group name
};


#endif // _COUNTERGROUP_H_
