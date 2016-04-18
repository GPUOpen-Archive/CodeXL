//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDisplayInfo.cpp
///
//==================================================================================

//------------------------------ tpDisplayInfo.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>

// Local:
#include <inc/tpDisplayInfo.h>
#include <inc/StringConstants.h>

tpDisplayInfo* tpDisplayInfo::m_spMySingleInstance = nullptr;

tpDisplayInfo::tpDisplayInfo()
{
}

tpDisplayInfo::~tpDisplayInfo()
{

}

tpDisplayInfo& tpDisplayInfo::Instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == nullptr)
    {
        // Create it:
        m_spMySingleInstance = new tpDisplayInfo;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

void tpDisplayInfo::GetThreadStateDisplayData(AMDTThreadState state, QColor& stateColor, QString& stateString)
{
    switch (state)
    {
        case AMDT_THREAD_STATE_INITIALIZED:
            stateString = CP_STR_ThreadStateInitialized;
            stateColor = QColor::fromRgb(255, 235, 100);
            break;

        case AMDT_THREAD_STATE_READY:
            stateString = CP_STR_ThreadStateReady;
            stateColor = QColor::fromRgb(122, 122, 100);
            break;

        case AMDT_THREAD_STATE_RUNNING:
            stateString = CP_STR_ThreadStateRunning;
            stateColor = Qt::red;
            break;

        case AMDT_THREAD_STATE_STANDBY:
            stateString = CP_STR_ThreadStateStandBy;
            stateColor = QColor::fromRgb(222, 222, 0);
            break;

        case AMDT_THREAD_STATE_TERMINATED:
            stateString = CP_STR_ThreadStateTerminated;
            stateColor = QColor::fromRgb(0, 222, 80);
            break;

        case AMDT_THREAD_STATE_WAITING:
            stateString = CP_STR_ThreadStateWaiting;
            stateColor = Qt::green;
            break;

        case AMDT_THREAD_STATE_TRANSITION:
            stateString = CP_STR_ThreadStateTransition;
            stateColor = QColor::fromRgb(255, 53, 220);
            break;

        case AMDT_THREAD_STATE_DEFERREDREADY:
            stateString = CP_STR_ThreadStateDeferredReady;
            stateColor = QColor::fromRgb(145, 181, 220);
            break;

        case AMDT_THREAD_STATE_GATEWAIT:
            stateString = CP_STR_ThreadStateGatewait;
            stateColor = QColor::fromRgb(255, 216, 32);
            break;

        default:
            break;
    }
}

QColor tpDisplayInfo::GetColorForCore(int coreIndex, int amountOfCores)
{
    static QColor firstCoreColor = QColor::fromRgb(240, 227, 255);
    static QColor lastCoreColor = QColor::fromRgb(53, 28, 117);

    int redDiff = (firstCoreColor.red() - lastCoreColor.red()) / amountOfCores;
    int red = qMin(firstCoreColor.red(), lastCoreColor.red()) + redDiff * coreIndex;

    int greenDiff = (firstCoreColor.green() - lastCoreColor.green()) / amountOfCores;
    int blue = qMin(firstCoreColor.green(), lastCoreColor.green()) + greenDiff * coreIndex;

    int blueDiff = (firstCoreColor.blue() - lastCoreColor.blue()) / amountOfCores;
    int green = qMin(firstCoreColor.blue(), lastCoreColor.blue()) + blueDiff * coreIndex;

    return QColor::fromRgb(red, blue, green);
}

void tpDisplayInfo::GetIconForCore(QIcon& icon, int coreIndex, int amountOfCores)
{
    QColor color = GetColorForCore(coreIndex, amountOfCores);

    QPixmap* pPixmap = new QPixmap(12, 12);

    QPainter* pPaint = new QPainter(pPixmap);
    pPaint->setBrush(QBrush(color, Qt::SolidPattern));
    pPaint->fillRect(0, 0, 16, 16, color);

    icon = QIcon(*pPixmap);
}

void tpDisplayInfo::TimeStampToString(AMDTUInt64 timestamp, QString& timestampStr)
{
    GT_UNREFERENCED_PARAMETER(timestamp);
    GT_UNREFERENCED_PARAMETER(timestampStr);
    // We get the samples in file time. Nanoseconds = FT * 100:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    FILETIME ft;
    LARGE_INTEGER ts;
    SYSTEMTIME st;
    AMDTUInt64 nanoseconds;

    ts.QuadPart = timestamp;
    ft.dwHighDateTime = ts.HighPart;
    ft.dwLowDateTime = ts.LowPart;
    FileTimeToSystemTime(&ft, &st);
    nanoseconds = (ts.QuadPart % 10000000) * 100;

    osTimeInterval interval(nanoseconds);
    gtString str = interval.AsString();
    timestampStr = acGTStringToQString(str);
#else
#pragma message ("TODO: TP : implement for linux")
    GT_ASSERT_EX(false, L"This function should be implemented for linux");
#endif
}

QString tpDisplayInfo::WaitReasonAsString(AMDTThreadWaitReason reason)
{
    QString retVal;

    switch (reason)
    {
        case AMDT_THREAD_WAIT_REASON_EXECUTIVE:
            retVal = "Waiting for a component of the Windows NT Executive";
            break;

        case AMDT_THREAD_WAIT_REASON_FREEPAGE:
            retVal = "Waiting for a page to be freed";
            break;

        case AMDT_THREAD_WAIT_REASON_PAGEIN:
            retVal = "- Waiting for a page to be mapped or copied";
            break;

        case AMDT_THREAD_WAIT_REASON_POOLALLOCATION:
            retVal = "Waiting for space to be allocated in the paged or nonpaged pool";
            break;

        case AMDT_THREAD_WAIT_REASON_DELAYEXECUTION:
            retVal = "Waiting for an Execution Delay to be resolved";
            break;

        case AMDT_THREAD_WAIT_REASON_SUSPENDED:
            retVal = "Suspended";
            break;

        case AMDT_THREAD_WAIT_REASON_USERREQUEST:
            retVal = "Waiting for a user request";
            break;

        case AMDT_THREAD_WAIT_REASON_WREXECUTIVE:
            retVal = "Waiting for a component of the Windows NT Executive";
            break;

        case AMDT_THREAD_WAIT_REASON_WRFREEPAGE:
            retVal = "Waiting for a page to be freed";
            break;

        case AMDT_THREAD_WAIT_REASON_WRPAGEIN:
            retVal = "Waiting for a page to be mapped or copied";
            break;

        case AMDT_THREAD_WAIT_REASON_WRPOOLALLOCATION:
            retVal = "Waiting for space to be allocated in the paged or nonpaged pool";
            break;

        case AMDT_THREAD_WAIT_REASON_WRDELAYEXECUTION:
            retVal = "Waiting for an Execution Delay to be resolved";
            break;

        case AMDT_THREAD_WAIT_REASON_WRSUSPENDED:
            retVal = "Suspended";
            break;

        case AMDT_THREAD_WAIT_REASON_WRUSERREQUEST:
            retVal = "Waiting for an event pair high";
            break;

        case AMDT_THREAD_WAIT_REASON_WRSPARE0:
            retVal = "Waiting for an event pair low";
            break;

        case AMDT_THREAD_WAIT_REASON_WRQUEUE:
            retVal = "AMDT_THREAD_WAIT_REASON_WRQUEUE";
            break;

        case AMDT_THREAD_WAIT_REASON_WRLPCRECEIVE:
            retVal = "Waiting for an LPC Reply notice";
            break;

#pragma message ("TODO: TP : Get strings for the wait reasons ")

        case AMDT_THREAD_WAIT_REASON_WRLPCREPLY:
            retVal = "AMDT_THREAD_WAIT_REASON_WRLPCREPLY";
            break;

        case AMDT_THREAD_WAIT_REASON_WRVIRTUALMEMORY:
            retVal = "Waiting for a page to be written to disk";
            break;

        case AMDT_THREAD_WAIT_REASON_WRPAGEOUT:
            retVal = "AMDT_THREAD_WAIT_REASON_WRPAGEOUT";
            break;

        case AMDT_THREAD_WAIT_REASON_WRRENDEZVOUS:
            retVal = "AMDT_THREAD_WAIT_REASON_WRRENDEZVOUS";
            break;

        case AMDT_THREAD_WAIT_REASON_WRKEYEDEVENT:
            retVal = "AMDT_THREAD_WAIT_REASON_WRKEYEDEVENT";
            break;

        case AMDT_THREAD_WAIT_REASON_WRTERMINATED:
            retVal = "Terminated";
            break;

        case AMDT_THREAD_WAIT_REASON_WRPROCESSINSWAP:
            retVal = "AMDT_THREAD_WAIT_REASON_WRPROCESSINSWAP";
            break;

        case AMDT_THREAD_WAIT_REASON_WRCPURATECONTROL:
            retVal = "AMDT_THREAD_WAIT_REASON_WRCPURATECONTROL";
            break;

        case AMDT_THREAD_WAIT_REASON_WRCALLOUTSTACK:
            retVal = "AMDT_THREAD_WAIT_REASON_WRCALLOUTSTACK";
            break;

        case AMDT_THREAD_WAIT_REASON_WRKERNEL:
            retVal = "AMDT_THREAD_WAIT_REASON_WRKERNEL";
            break;

        case AMDT_THREAD_WAIT_REASON_WRRESOURCE:
            retVal = "AMDT_THREAD_WAIT_REASON_WRRESOURCE";
            break;

        case AMDT_THREAD_WAIT_REASON_WRPUSHLOCK:
            retVal = "AMDT_THREAD_WAIT_REASON_WRPUSHLOCK";
            break;

        case AMDT_THREAD_WAIT_REASON_WRMUTEX:
            retVal = "AMDT_THREAD_WAIT_REASON_WRMUTEX";
            break;

        case AMDT_THREAD_WAIT_REASON_WRQUANTUMEND:
            retVal = "AMDT_THREAD_WAIT_REASON_WRQUANTUMEND";
            break;

        case AMDT_THREAD_WAIT_REASON_WRDISPATCHINT:
            retVal = "AMDT_THREAD_WAIT_REASON_WRDISPATCHINT";
            break;

        case AMDT_THREAD_WAIT_REASON_WRPREEMPTED:
            retVal = "AMDT_THREAD_WAIT_REASON_WRPREEMPTED";
            break;

        case AMDT_THREAD_WAIT_REASON_WRYIELDEXECUTION:
            retVal = "AMDT_THREAD_WAIT_REASON_WRYIELDEXECUTION";
            break;

        case AMDT_THREAD_WAIT_REASON_WRFASTMUTEX:
            retVal = "AMDT_THREAD_WAIT_REASON_WRFASTMUTEX";
            break;

        case AMDT_THREAD_WAIT_REASON_WRGUARDEDMUTEX:
            retVal = "AMDT_THREAD_WAIT_REASON_WRGUARDEDMUTEX";
            break;

        case AMDT_THREAD_WAIT_REASON_WRRUNDOWN:
            retVal = "AMDT_THREAD_WAIT_REASON_WRRUNDOWN";
            break;

        case AMDT_THREAD_WAIT_REASON_WRALERTBYTHREADID:
            retVal = "AMDT_THREAD_WAIT_REASON_WRALERTBYTHREADID";
            break;

        case AMDT_THREAD_WAIT_REASON_WRDEFERREDPREEMPT:
            retVal = "AMDT_THREAD_WAIT_REASON_WRDEFERREDPREEMPT";
            break;

        default:
            break;
    }

    return retVal;
}
