//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppColors.h
///
//==================================================================================

//------TEMPORARY FILE-------------

#ifndef __PPCOLORS
#define __PPCOLORS
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4718)
#endif

#include <QtCore>

#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>

static const QColor PP_OTHERS_COUNTER_COLOR(0, 170, 181, 20);
static const QColor& PP_TOTAL_COUNTER_COLOR = acQAMD_CYAN_OVERLAP_COLOUR;

static const QColor& PP_ACTIVE_RANGE_AXES_COLOR = acQAMD_GRAY1_COLOUR;

static const QString& PP_TOOLTIP_BACKGROUND_COLOR = "rgba(199, 200, 202, 100)";
static const QString& PP_TOOLTIP_BORDER_COLOR = acQAMD_ORANGE_OVERLAP_COLOUR.name(QColor::HexArgb);


class ppColorsMap
{
public:

    /// this function returns the single-tone instance of ppHierarchyMap
    /// returns reference to ppHierarchyMap single instance
    static ppColorsMap& Instance();

    /// gets counter color
    /// \param counterName the counter name
    /// \returns the color for this counter
    QColor GetColorForCounterName(const unsigned int& counterId);

private:

    /// constrictor
    ppColorsMap();

    /// counters to colors map
    QMap<int, QColor> m_colorsMap;

    /// number of dGpus (for dynamic color calculations)
    unsigned int m_dgpusCount;

    /// single-tone instance
    static ppColorsMap* m_colorsSingleInstance;
};



/// this class is temporary int his place, until changes in version 1.8.
class ppHierarchyMap
{
public:

    /// this function returns the single-tone instance of ppHierarchyMap
    /// returns reference to ppHierarchyMap single instance
    static ppHierarchyMap& Instance();

    /// does the specific counter has a parent
    /// \param counterName is the child counter
    /// \returns true if the counter has a parent
    bool IsChildCounter(const QString& counterName) const;

    /// gets the parent counter name
    /// \param counterName is the child counter
    /// \returns the parent counter name
    QString GetCounterParent(const QString& counterName) const;

    /// get the list of children for a specific counter
    /// \param counterName is the parent counter
    /// \returns a list of children counters
    QList<QString> GetChildrenListForCounter(const QString& counterName);

private:

    /// constrictor
    ppHierarchyMap();

    // map of child - parent. hold only the counters that have parents
    QMap<QString, QString> m_hierarchyMap;

    /// single-tone instance
    static ppHierarchyMap* m_hierarchyMapSingleInstance;
};

#ifdef _WIN32
    #pragma warning( pop )
#endif

#endif //__PPCOLORS
