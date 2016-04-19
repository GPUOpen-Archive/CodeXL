//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file StdAfx.h
/// \brief The common header file for this project.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/StdAfx.h#36 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#ifndef _STDAFX_H
#define _STDAFX_H

#include <QtCore>
#include <QtWidgets>
#include <QList>
#include <QMap>
#include <QString>
#include <QVector>
#include <qcolor.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Enable the unreferenced warning for windows (to avoid linux compile errors):
    #pragma warning(default:4101)
    #pragma warning(default:4100)
    #pragma warning(default:4706)
#endif


//infrastructure
#include <AMDTBaseTools/Include/gtAssert.h>

#include <AMDTCpuPerfEventUtils/inc/DcConfig.h>
#include <AMDTCpuPerfEventUtils/inc/ViewConfig.h>
#include <AMDTCpuPerfEventUtils/inc/EventEncoding.h>

#define RETURN_FALSE_IF_NULL(pointer) { if (nullptr == pointer) { return false; }}

#define RETURN_NULL_IF_NULL(pointer)  { if (nullptr == pointer) { return nullptr; } }

#define RETURN_IF_NULL(pointer) { if (nullptr == pointer) { return; } }

#define MAX_EVENTNUM        4
#define MAX_CPUS            256
#define ONCHIP_LOCAL_APIC   0x200
#define TM_CHAR QChar (8482)
#define INVALID_TAB_INDEX -1

const QString UNKNOWN_MODULE_PID(QObject::tr("unknown module pid"));
#define UNKNOWN_KERNEL_SAMPLES  L"Unknown Kernel Samples"

// Data aggregation choices for the system data tab.
enum SYSTEM_DATA_TAB_CONTENT
{
    AGGREGATE_BY_MODULES,
    AGGREGATE_BY_PROCESSES
};

class ViewConfig;
class ViewElementType
{
public:
    QString path;
    bool modifiable;
    ViewConfig* pView;
    ViewElementType()
    {
        pView = nullptr;
    };
    ~ViewElementType()
    {
        if (nullptr != pView)
        {
            delete pView;
        }
    };
};

typedef QMap<QString, ViewElementType> ConfigMap;

typedef QMap<EventMaskType, float> EventNormValueMap;

class SampleKeyType
{
public:
    int cpu;
    EventMaskType event;
    SampleKeyType();
    SampleKeyType(int a, EventMaskType b);

    inline bool operator< (const SampleKeyType& other) const
    {
        if (cpu < other.cpu)
        {
            return true;
        }

        if ((cpu == other.cpu) && (event < other.event))
        {
            return true;
        }

        return false;
    };

};

typedef QMap<unsigned int, unsigned int> DupEventMap;
typedef QMap<SampleKeyType, int> TempEventIndexMap;
typedef QMap<SampleKeyType, unsigned long> OneSampleMap;
typedef QVector <float> DataArray;


typedef QMap<unsigned int, unsigned int> OffsetLinenumMap;
typedef QVector<QColor> ColorList;

enum
{
    LONG_STR_MAX = 20
};

#ifdef _x86_64_
    #define LONG_FORMAT "0x%016lx"
#else
    #define LONG_FORMAT "0x%08lx"
#endif

enum PID_HARDCODED_TYPE
{
    SHOW_ALL_PIDS = 0
};

enum TID_HARDCODED_TYPE
{
    SHOW_ALL_TIDS = 0
};

#define NO_SYMBOL_STR  "NO SYMBOL"
#define NO_SYMBOL_WSTR L"NO SYMBOL"
#define NO_SYMBOL_STR_LEN (sizeof(NO_SYMBOL_STR) - 1)

const QString CA_NO_SYMBOL(QObject::tr(NO_SYMBOL_STR));

const QString CACHE_FILE_MAP(QObject::tr("/CacheFile.map"));
const QString CACHE_STR(QObject::tr("Cached "));
typedef QMap<QString, QString> CacheFileMap;

//also used for storing relocated source files if caching off;
bool ReadSessionCacheFileMap(const QString& sessionDir, CacheFileMap& cache);
bool WriteSessionCacheFileMap(const QString& sessionDir, CacheFileMap& cache);
bool GetCachedFile(QString sessionDir, QString filePath, QString& cachedPath);
bool CacheFile(const QString& sessionDir, QString filePath, const QString& altSource = "", bool symsToo = false);
bool CacheRelocatedSource(const QString& sessionDir, QString filePath, const QString& relocationPath, bool cache = false, bool symsToo = false);
QString FindModuleFile(QWidget* pParent, const QString& originalPath);

#define TBS_FILE_COMMENT    "#"
#define TBS_LINE_DELIMITER  ","
#define TBS_FILE_VAR        "!"
enum HardcodedEventType
{
    NON_LOCAL_EVENT = 0xe9
};

#define NEW_SESSION_NAME    "Session"
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    const char PATH_SLASH = '\\';
#else
    const char PATH_SLASH = '/';
#endif

#define MSG_VSI_PROFILE_FINISHED    WM_USER+4
#define MSG_CA_WAKEUP   WM_USER+5


// This vector contains the order of indexes to the available data for what's shown
typedef QVector<int> IndexVector;
// Given a cpu/event key, get the column index
typedef QMap <SampleKeyType, int> CpuEventViewIndexMap;
struct ComplexComponents
{
    int complexType;
    int columnIndex;
    int op1Index;
    float op1NormValue;
    int op2Index;
    float op2NormValue;
};

typedef QList<ComplexComponents> ListOfComplex;
// This map will allow changes to either of the dependent values
// to recalculate the complex column during aggregation
// Every complex column (currently) depends on two operand columns -
// the complex column data will be added to the list at each index,
// since one column change may affect multiple complex columns.
typedef QMap <int, ListOfComplex> ComplexDependentMap;
typedef QMap <int, int> TotalIndexMap;

typedef QVector<unsigned int> CoreFilter;
//The number of miliSeconds to display the message that a search failed
#define BAD_SEARCH_DELAY 3000

#endif // _STDAFX_H
