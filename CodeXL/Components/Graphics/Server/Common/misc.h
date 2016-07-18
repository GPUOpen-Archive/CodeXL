//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Collections of miscellaneous support fuinctions.
//==============================================================================

#ifndef GPS_MISC_H
#define GPS_MISC_H

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//--------------------------------------------------------------------------
/// Used in cases where a function is traced, but has a "void" return type.
//--------------------------------------------------------------------------
static const int FUNCTION_RETURNS_VOID = -1;

#if defined _WIN32
    #include "windows.h"
    #include "Unknwn.h"
#endif // _WIN32
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include "Logger.h"
#include "mymutex.h"
#include "TSingleton.h"
#include "defines.h"
#include "CommonTypes.h"
#include <stdlib.h>
#include "ExportDefinitions.h"

#if defined (_LINUX)
    #include <sys/time.h>
    #include "Linux/SafeCRT.h"
#endif // _LINUX

// Forward declaration for usage in InterceptorBase.
class ModernAPILayerManager;

enum
{
    TIME_OVERRIDE_FREEZE,
    TIME_OVERRIDE_SLOWMOTION,
    TIME_OVERRIDE_NONE,
};

//--------------------------------------------------------------------------
/// A set of flag bits that define which type of trace is to be collected.
/// Declaration is mirrored for Client use in TraceManager.cs
//--------------------------------------------------------------------------
enum eTraceType
{
    kTraceType_None         = 0x0,
    kTraceType_API          = 0x1,
    kTraceType_GPU          = 0x2,
    kTraceType_Linked       = kTraceType_API | kTraceType_GPU,
    kTraceType_AutoCapture  = 0x4
};

/// Helper to format text similarly to the way printf does
/// \param fmt a formatted string
/// \return the formatted string with values properly substituted
gtASCIIString FormatText(const char* fmt, ...);

/// Helper to format text similarly to the way printf does
/// \param fmt a formatted string
/// \return the formatted string with values properly substituted
std::string FormatString(const char* fmt, ...);

//--------------------------------------------------------------------------
/// Build a string of comma-separated pointers surrounded with brackets.
/// \param inCount The number of items in the array to print.
/// \param inArray The array where the values to print live.
/// \param inFormatString The formatter used to print each item in the array.
/// \return A string containing a bracketed comma-separated list of pointers.
//--------------------------------------------------------------------------
template<class T>
std::string PrintArrayWithFormatter(int inCount, T inArray, const char* inFormatString)
{
    std::string out;
    out = "[ ";

    for (int i = 0; i < inCount; i++)
    {
        if (i != 0)
        {
            out += ", ";
        }

        out += FormatText(inFormatString, inArray[i]).asCharArray();
    }

    out += " ]";
    return out;
}

/// Displays a message box with the specified stop message
/// \param s the stop message to display
void MessageBoxStop(gtASCIIString s);

/// Displays a message box with the specified info message
/// \param s the info message to display
void MessageBoxInfo(gtASCIIString s);

/// Displays a message box with the specified warning message
/// \param s the warning message to display
void MessageBoxWarning(gtASCIIString s);

/// Displays a message box with the specified error message
/// \param s the error message to display
void MessageBoxError(gtASCIIString s);

/// Indicates that the specified parameter is not referenced in the function.
/// This should be used to remove warnings due to unused parameters
#if defined (_WIN32)
    #define PS_UNREFERENCED_PARAMETER(p) (p) ///< Definition
#else
    #define PS_UNREFERENCED_PARAMETER(p) (void)(p)
#endif

/// Converts a HRESULT to a string
/// \param hRes the HRESULT to convert
/// \return The error string equivalent
#if defined (_WIN32)
    std::string GetErrorStringFromHRESULT(HRESULT hRes);
#endif

extern void hResultLogAndAssert(INT32 error, const char* strMessage);

/// Decodes a URL encoded string.
/// \param pszDecodedOut The decoded output string
/// \param nBufferSize The size of the output string (must be at least the size of pszEncodedIn)
/// \param pszEncodedIn The encoded input string
/// \param bIgnoreLinefeed Flag to ignore the linefeed command.
extern void URLDecode(char* pszDecodedOut, size_t nBufferSize, const char* pszEncodedIn, bool bIgnoreLinefeed);

//
/// This class helps detect reentrance problems
//
class RefTrackerCounter
{
    bool m_IsUsingExternalMutex;            ///< If using an external mutex (by using the constructor passing in a mutex or calling UseExternalMutex())

    mutex* m_pmutex;                        ///< pointer to a mutex to ensure the reference counter is only updated by one thread at a time

    std::map<UINT32, int> m_mapInsideWrapper; ///< map of reference count and thread ID

public:
    RefTrackerCounter();                    ///< default constructor
    ~RefTrackerCounter();                   ///< destructor
    RefTrackerCounter(mutex* pM);           ///< constructor taking a mutex pointer
    void UseExternalMutex(mutex* pM);       ///< use an external mutex rather than once created by this class
    void operator++(int);                   ///< increment operator
    void operator--(int);                   ///< decrement operator
    bool operator==(UINT32 v);              ///< equality operator
    bool operator>(UINT32 v);               ///< greater than operator
    UINT32 GetRef();                        ///< return the current reference count
};

//
/// This class helps detect reentrance problems
//
class RefTracker
{
    /// pointer to an unsigned long that contains the number of references
    RefTrackerCounter* m_dwVal;

public:

    /// increments dwVal on creation
    RefTracker(RefTrackerCounter* dwVal)
    {
        m_dwVal = dwVal;

        PsAssert(m_dwVal != NULL);

        (*m_dwVal)++;
    }

    /// decrements dwVal on destruction
    ~RefTracker()
    {
        //
        // Protect against the app doing something stupid (like releasing a
        // resource more than once)
        if ((*m_dwVal) > 0)
        {
            (*m_dwVal)--;
        }
        else
        {
            Log(logWARNING, "RefTracker destructor called with m_dwVal == 0\n");
        }
    }
};

/// Get the time string
gtASCIIString GetTimeStr();

/// Get the micro time string
gtASCIIString GetMicroTimeStr();

/// Dump hex
/// \param pData Input data
/// \param dwSize Size
/// \param dwLength Length
/// \return String
gtASCIIString DumpHex(unsigned char* pData, unsigned long dwSize, unsigned long dwLength);

/// Definition
#define SAFE_DELETE_ARRAY( p ) { if ( p != NULL ) { delete [] p; p = NULL; } }
/// Definition
#define SAFE_DELETE( p ) { if ( p != NULL ) { delete p; p = NULL; } }
/// Definition
#define SAFE_RELEASE( p ) { if ( p != NULL ) if ( p->Release() == 0 ) p = NULL; }


//====================================================================================
/// \brief Utility method that wraps new in a try catch block
//====================================================================================
template<class T>
bool PsNew(T*& pOutObject)
{
    try
    {
        pOutObject = new T();
    }
    catch (std::bad_alloc)
    {
        Log(logERROR, "Out of memory\n");
        return false;
    }

    return true;
}

//====================================================================================
/// \brief Utility method that wraps array new in a try catch block
//====================================================================================
template<class T>
bool PsNewArray(T*& pOutObject, unsigned int nSize = 0)
{
    if (nSize < 1)
    {
        Log(logERROR, "Array size must be larger than 0\n");
        return false;
    }

    try
    {
        pOutObject = new T [nSize];
    }
    catch (std::bad_alloc)
    {
        Log(logERROR, "Out of memory\n");
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Helper struct to extract layouts from command data
//-----------------------------------------------------------------------------
struct HUDLayout
{
    int m_nIndex;     ///< the index of the hud element if it is part of an array
    bool m_bShow;     ///< indicates whether or not the element is visible
    int m_nTop;       ///< the y coordinate corresponding to the top of the HUD element
    int m_nLeft;      ///< the x coordinate corresponding to the left of the HUD element
    int m_nWidth;     ///< the width of the element
    int m_nHeight;    ///< the height of the element

    /// Constructor
    HUDLayout():
        m_nIndex(0),
        m_bShow(false),
        m_nTop(0),
        m_nLeft(0),
        m_nWidth(0),
        m_nHeight(0)
    {
    }

    //-----------------------------------------------------------------------------
    /// Extract the layouts from an input string and return them.
    /// \param str The input string with the layout dscriptions.
    /// \param layouts Output vector of layout data.
    //-----------------------------------------------------------------------------
    static void ExtractLayouts(const char* str, std::vector<HUDLayout>& layouts)
    {
        char* pNextLayout = NULL;
        char* pToken;
        pToken = strtok_s((char*)str, ";", &pNextLayout);

        while (pToken != NULL)
        {
            // Get next token:
            if (pToken != NULL)
            {
                HUDLayout layout;

                char* pInnerToken;
                char* pNextDataItem = NULL;

                // Get the Index
                pInnerToken = strtok_s(pToken, ",", &pNextDataItem);
                layout.m_nIndex = atoi(pInnerToken);

                // Get the value that decides if the element is shown on the HUD.
                pInnerToken = strtok_s(NULL, ",", &pNextDataItem);
                layout.m_bShow = false;

                if (strcmp(pInnerToken, "False") == 0)
                {
                    layout.m_bShow = false;
                }
                else if (strcmp(pInnerToken, "True") == 0)
                {
                    layout.m_bShow = true;
                }

                // Get the Index
                pInnerToken = strtok_s(NULL, ",", &pNextDataItem);
                layout.m_nTop = atoi(pInnerToken);

                // Get the Left
                pInnerToken = strtok_s(NULL, ",", &pNextDataItem);
                layout.m_nLeft = atoi(pInnerToken);

                // Get the Width
                pInnerToken = strtok_s(NULL, ",", &pNextDataItem);
                layout.m_nWidth = atoi(pInnerToken);

                // Get the Height
                pInnerToken = strtok_s(NULL, ",", &pNextDataItem);
                layout.m_nHeight = atoi(pInnerToken);

                layouts.push_back(layout);

                // Get the next layout data set.
                pToken = strtok_s(NULL, ";", &pNextLayout);
            }
        }
    }
};

//----------------------------------------------------------------
// URL encode an string, replace illegal chars with %hex
//----------------------------------------------------------------
std::string UrlEncode(std::string in);

//----------------------------------------------------------------
/// Return a text version of S_OK and S_FALSE
/// \param eVal The HRESULT value
/// \return String version of the HRESULT
//----------------------------------------------------------------
extern const char* GetErrorString(unsigned int eVal);

///////////////////////////////////////////////////////////////////////////////////////////
/// This class is an easy to use logging object that logs the time difference between
/// when the object was created and LogTimeFromStart() is called.
///////////////////////////////////////////////////////////////////////////////////////////
#if 0  // LogTime currently unused
class LogTime
{

public:

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Constructor
    ///////////////////////////////////////////////////////////////////////////////////////////
    LogTime()
    {
#if defined (_WIN32)
        QueryPerformanceFrequency((LARGE_INTEGER*)&m_nPerfCountFreq);
        QueryPerformanceCounter(&m_nStartCount);
#elif defined (_LINUX)
        struct timeval tv;
        gettimeofday(&tv, NULL);
        m_nStartCount = (GPS_TIMESTAMP)tv.tv_sec * 1000000 + (GPS_TIMESTAMP)tv.tv_usec;
#endif
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// logs the time difference between when the object was created and LogTimeFromStart() is called.
    /// \param str A user defined string associated with the log message.
    ///////////////////////////////////////////////////////////////////////////////////////////
    void LogTimeFromStart(char* str)
    {
#if defined (_WIN32)
        PsAssert(str != NULL);
        GPS_TIMESTAMP nEndCount;
        QueryPerformanceCounter(&nEndCount);
        GPS_TIMESTAMP timeDiff;
        timeDiff.QuadPart = nEndCount.QuadPart - m_nStartCount.QuadPart;
        //Log(logMESSAGE, "%s %lf\n", str, (double)timeDiff.QuadPart / (double)m_nPerfCountFreq);
        //Log(logMESSAGE, "%s %lf\n", str, (double)timeDiff.QuadPart);
        //LONGLONG
        Log(logMESSAGE, "%s %ld\n", str, timeDiff.QuadPart);
#elif defined (_LINUX)
        struct timeval tv;
        GPS_TIMESTAMP nEndCount;
        GPS_TIMESTAMP timeDiff;

        gettimeofday(&tv, NULL);
        nEndCount = (GPS_TIMESTAMP)tv.tv_sec * 1000000 + (GPS_TIMESTAMP)tv.tv_usec;

        timeDiff = nEndCount - m_nStartCount;

        Log(logMESSAGE, "%s %ld\n", str, timeDiff);
#else
#warning IMPLEMENT ME!
#endif
    }

private :

    /// record the sample frequency
    UINT64 m_nPerfCountFreq ;
    /// Time when the object is created
    GPS_TIMESTAMP m_nStartCount;

};
#endif

// uncomment to use global object tracking in VTable patching
//#define USE_GLOBAL_TRACKER

#ifdef USE_GLOBAL_TRACKER
///////////////////////////////////////////////////////////////////////////////////////////
/// A dubugging class that tracks the VTable patching of objects that come from the API
///////////////////////////////////////////////////////////////////////////////////////////
class GlobalObjectTracker : public TSingleton< GlobalObjectTracker >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<GlobalObjectTracker>;

    /// map of a vtable to a map of objects which use that vtable
    typedef std::map< IUnknown*, int > ObjectTrackerMap;

public:

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Returns true if the object pointer is currently in the map (false if not)
    /// \param pObj The input object pointer
    /// \return true or false.
    ///////////////////////////////////////////////////////////////////////////////////////////
    bool IsBeingTracked(IUnknown* pObj)
    {
        ObjectTrackerMap::iterator iter = m_objectTrackerMap.find(pObj);

        if (iter != m_objectTrackerMap.end() && iter->second > 0)
        {
            /// We did  find the object pointer
            return true;
        }
        else
        {
            /// We did not find the object pointer
            return false;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Adds an object pointer or a ref to the map
    /// \param pObj The object pointer to add or ref
    ///////////////////////////////////////////////////////////////////////////////////////////
    void AddObject(IUnknown* pObj)
    {
        ObjectTrackerMap::iterator iter = m_objectTrackerMap.find(pObj);

        if (iter == m_objectTrackerMap.end())
        {
            // Object not found in map so add it.
            m_objectTrackerMap[pObj] = 1;
#ifdef USE_STREAMLOG
            StreamLog::Ref() << "Object Tracker: Adding object" << pObj << "\n";
#endif
        }
        else
        {
            iter->second++;

            // Object found in the map so increment the ref
#ifdef USE_STREAMLOG
            StreamLog::Ref() << "Object Tracker: Adding REF to object" << pObj << "RefCount: " << iter->second << "\n";
#endif
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Remove a pointer ref from the map
    /// \param pObj The object pointer to remove
    ///////////////////////////////////////////////////////////////////////////////////////////
    void RemoveObjectRef(IUnknown* pObj)
    {
        ObjectTrackerMap::iterator iter = m_objectTrackerMap.find(pObj);

        if (iter == m_objectTrackerMap.end())
        {
            // Not found
#ifdef USE_STREAMLOG
            StreamLog::Ref() << "GlobalObjectTracker::RemoveObject: Trying to remove an object not in tracker:" << pObj << "\n";
#endif
        }
        else
        {
            // Decrement the count
            int nCount = iter->second;

            if (nCount  < 0)
            {
#ifdef USE_STREAMLOG
                StreamLog::Ref() << "GlobalObjectTracker::RemoveObject: Trying to remove an object too many times:" << pObj << "ref:" << nCount << "\n";
#endif
                iter->second--;
            }
            else
            {
                iter->second--;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Cleanup the map so it can be re-used.
    ///////////////////////////////////////////////////////////////////////////////////////////
    void Clear()
    {
        m_objectTrackerMap.clear();
    }

private:

    /// The map used to store the object pointers and their ref counts
    ObjectTrackerMap m_objectTrackerMap ;

};
#endif // USE_GLOBAL_TRACKER

#ifdef _WIN32
//--------------------------------------------------------------------------
/// A small class to manage linking a Real_ and Mine_ function together when hooked.
//--------------------------------------------------------------------------
template <typename HookType>
class RealAndMineHook
{
public:
    //--------------------------------------------------------------------------
    /// Default constructor that initializes the internal hooks to NULL.
    //--------------------------------------------------------------------------
    RealAndMineHook() : mRealHook(NULL), mMineHook(NULL) {}

    //--------------------------------------------------------------------------
    /// Initialize the hooks pair within this instance of a RealAndMineHook.
    /// \param inRealHook A function pointer to the real implementation of the function.
    /// \param inMineHook A function pointer to the Mine_ implementation of the function.
    //--------------------------------------------------------------------------
    void SetHooks(HookType inRealHook, HookType inMineHook)
    {
        mRealHook = inRealHook;
        mMineHook = inMineHook;
    }

    //--------------------------------------------------------------------------
    /// This call allows us to attach our hooked function.
    /// \returns True if the function was hooked successfully, false if otherwise.
    //--------------------------------------------------------------------------
    bool Attach()
    {
        bool bResult = (AMDT::HookAPICall((PVOID*)&mRealHook, mMineHook) == NO_ERROR);

        if (bResult != true)
        {
            Log(logERROR, "HookAPICall FAILED. Hook not attached successfully.\n");
        }

        return bResult;
    }

    //--------------------------------------------------------------------------
    /// This call allows us to detach our hooked function.
    /// \returns True if the function was unhooked successfully.
    //--------------------------------------------------------------------------
    bool Detach()
    {
        bool bResult = (AMDT::UnhookAPICall((PVOID*)&mRealHook, mMineHook) == NO_ERROR);

        if (bResult != true)
        {
            Log(logERROR, "Hook not detached successfully.\n");
        }

        return bResult;
    }

    /// A function pointer to the real function implementation.
    HookType mRealHook;

    /// A function pointer to the GPS-instrumented function implementation.
    HookType mMineHook;
};
#endif // def WIN32

// A macro used within switch statements to stringify and return the selected case.
#define PRINTENUMCASE(inEnum, outString) case inEnum: outString = #inEnum; break;

//--------------------------------------------------------------------------
/// A small baseclass to provide a common interface for enable/disable of
/// API/GPU Tracing with a MultithreadedTraceAnalyzerLayer.
//--------------------------------------------------------------------------
class InterceptorBase
{
public:

    //-----------------------------------------------------------------------------
    /// Interceptor destructor.
    //-----------------------------------------------------------------------------
    virtual ~InterceptorBase()
    {
    }

    //-----------------------------------------------------------------------------
    /// Flag to control API trace generation
    /// \param inbCollectTrace for ON, false for OFF
    //-----------------------------------------------------------------------------
    virtual void SetCollectTrace(bool inbCollectTrace) = 0;

    //-----------------------------------------------------------------------------
    /// Flag to control GPU trace generation
    /// \param inbProfilingEnabled for ON, false for OFF
    //-----------------------------------------------------------------------------
    virtual void SetProfilingEnabled(bool inbProfilingEnabled) = 0;

    //-----------------------------------------------------------------------------
    /// Gets the parent layer manager
    /// \return Pointer to the layer manager
    //-----------------------------------------------------------------------------
    virtual ModernAPILayerManager* GetParentLayerManager() = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////
/// Simple map class to record system failure events. It is used in Frame capture where our copies of some API calls
/// can cause resource failures. We need to detect these events and protect the server from them.
///////////////////////////////////////////////////////////////////////////////////////////
class SystemFailure : public TSingleton< SystemFailure >
{
    friend class TSingleton< SystemFailure >;

    /// Structure to relate the failure event to a function name
    struct StringPair
    {
        std::string m_strFunction;   ///< The name of the function where the failure happened
        std::string m_strEvent;      ///< String containing details of the faliure event
    };

    /// map of a vtable to a map of objects which use that vtable
    std::vector< StringPair* > m_Events;

public:

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Add a failure event to the map usings strings
    /// \param strFunction The function where the event happened.
    /// \param strEvent Details of the event.
    ///////////////////////////////////////////////////////////////////////////////////////////
    void AddFailureEvent(std::string strFunction, std::string strEvent)
    {
        StringPair* pEventPair = new StringPair();
        pEventPair->m_strFunction = strFunction;
        pEventPair->m_strEvent = strEvent;

        m_Events.push_back(pEventPair);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Add a failure event to the map usings char*'s
    /// \param pFunction The function where the event happened.
    /// \param pEvent Details of the event.
    ///////////////////////////////////////////////////////////////////////////////////////////
    void AddFailureEvent(char* pFunction, char* pEvent)
    {
        //std::string str1(pFunction);
        //std::string str2(pEvent);
        AddFailureEvent(std::string(pFunction), std::string(pEvent));
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Use this to detect if a failure event happened.
    /// \return True if a system item failed.
    ///////////////////////////////////////////////////////////////////////////////////////////
    bool SystemFailed()
    {
        if (m_Events.empty() == false)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Clear the events map
    ///////////////////////////////////////////////////////////////////////////////////////////
    void Clear()
    {
        m_Events.clear();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Get the failure string from a specific failure event.
    ///////////////////////////////////////////////////////////////////////////////////////////
    std::string GetFailureString(UINT32 nIndex)
    {
        PsAssert(nIndex < m_Events.size());

        if ((m_Events.empty() == false) && (nIndex < m_Events.size()))
        {
            StringPair* pEventPair = m_Events[nIndex];
            return (pEventPair->m_strFunction + " " + pEventPair->m_strEvent);
        }
        else
        {
            return std::string("SystemFailure::GetFailure index out of range.");
        }
    }

};



//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Structure which stores the process information that is important to PerfStudio.
/// variables names here are taken from the PROCESSENTRY32 structure
/// simply to make sure that it is easy to switch between one and the other
//////////////////////////////////////////////////////////////////////////////////////////////////////
struct ProcessInfo
{
    unsigned long th32ProcessID;     ///< Process ID
    char szExeFile[ PS_MAX_PATH ];   ///< Path and filename of the exe
    char szPath[ PS_MAX_PATH ];      ///< Path to the exe
};

#if defined (_WIN32)

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Enable token privilege
/// \param htok Handle
/// \param szPrivilege Privilige
/// \param tpOld Token priviliges
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool enable_token_privilege(HANDLE htok, LPCTSTR szPrivilege, TOKEN_PRIVILEGES* tpOld);

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Enable privilege
/// \param szPrivilege Privilige
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool enable_privilege(LPCTSTR szPrivilege);

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Return a string with the thread ID
/// \return an std::string
//////////////////////////////////////////////////////////////////////////////////////////////////////
std::string GetThreadString();

DWORD GetThreadsID();

/// Help function - show reminder message to disable in game overlay
enum RMD_MSGS
{
    RMD_MSG_OL_NONE,
    RMD_MSG_OL_STEAM,
    RMD_MSG_OL_ORIGIN,
    RMD_MSG_OL_UPLAY
};

void ShowLauncherReminder(const char* szAppName);
void ShowLauncherReminder(const wchar_t* szAppName);
void LauncherReminderMessage(int iRmdMsg);

#endif // _WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// A hacky function to detect the end of an ASCI  string within a binary file
/// \param buf The input data
/// \param len Length of the input data
/// \param start The index into the main string to start searching from
/// \param str Records the ASCI section in a reference that can be read outside of this function
//////////////////////////////////////////////////////////////////////////////////////////////////////
extern long find_non_ascii(unsigned char* buf, long len, long start, std::string& str);

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Designed to work on binary data wherte strstr function cannot be used
/// Searches for a substring within the buf
/// \param buf String to search within
/// \param len Length of the input string
/// \param s The substring to search for
//////////////////////////////////////////////////////////////////////////////////////////////////////
extern long find_string_in_buf(unsigned char* buf, size_t len, const char* s);

extern const char* GetPerfStudioDirName();

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Converts data from a half format to a floating point value
/// \param half The bits of the half data.
/// \return A floating point equivalent to the half value.
//////////////////////////////////////////////////////////////////////////////////////////////////////
float HalfToFloat(unsigned short half);

//////////////////////////////////////////////////////////////////////////////////////////////////////
/// Obtain location of GPS server
/// \param  out    String containing PerfServer location
/// \return True   If successful.
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool GetModuleDirectory(gtASCIIString& out);

#endif // GPS_MISC_H
