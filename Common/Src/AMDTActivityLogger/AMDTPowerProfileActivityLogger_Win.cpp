#include <windows.h>
#include <Sddl.h>
#include <stdio.h>
#include <AMDTDriverTypedefs.h>
#include "AMDTActivityLogger.h"

#pragma warning(disable: 4996)

// to be allocated dynamically depending on the number of available cores
#define PWRPROF_SHARED_BUFFER_SIZE 256*4096

// 1K for all core buffer parameter other than buffers
#define PWRPROF_SHARED_METADATA_SIZE 4096

// Maximum per core buffer size
#define PWRPROF_PERCORE_BUFFER_SIZE 16 * 4096

#define PWR_PROF_SHARED_OBJ L"Global\\AMD_PWRPROF_SHARED_OBJ"

/// \def PWR_PROF_SHARED_OBJ_BASE The shared object that various instances of
/// power profilers will use to signal a pause state during sampling
#define PWRPROF_SHARED_OBJ_BASE L"\\BaseNamedObjects\\Global\\AMD_PWRPROF_SHARED_OBJ"
#define PWR_MAX_MARKER_CNT 10
#define PWR_MARKER_BUFFER_SIZE 32

// Maker states
#define PWR_MARKER_DISABLE 0
#define PWR_MARKER_ENABLE 1
#define PWR_MARKER_ENABLE_INITIATED 2
#define PWR_MARKER_DISABLE_INITIATED 3


static HANDLE g_sharedFile = nullptr;
static unsigned char* g_pSharedBuffer = nullptr;
static bool g_isInitialized = false;

// MarkerTag Data
// Start and stop marker tag
typedef struct MarkerTag
{
    uint32     m_markerId;
    uint32     m_pid;
    uint32     m_state;
    uint32     m_fill;
    uint64     m_ts;
    uint8      m_name[PWR_MARKER_BUFFER_SIZE];
    uint8      m_userBuffer[PWR_MARKER_BUFFER_SIZE];
} MarkerTag;


// InitializeSharedBuffer: Create and initialize shared buffer between driver and user space
extern "C"
static int PwrInitializeSharedBuffer(void)
{
    int ret = -1;
    g_sharedFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS,  // read/write access
                                    FALSE,                // do not inherit the name
                                    PWR_PROF_SHARED_OBJ); // name of mapping object

    if (nullptr == g_sharedFile)
    {
        SECURITY_ATTRIBUTES secAttr;
        PSECURITY_DESCRIPTOR pSD;
        PACL pSacl = nullptr;  // not allocated
        BOOL fSaclPresent = FALSE;
        BOOL fSaclDefaulted = FALSE;
        char secDesc[ SECURITY_DESCRIPTOR_MIN_LENGTH ] = "";

        secAttr.nLength = sizeof(secAttr);
        secAttr.bInheritHandle = FALSE;
        secAttr.lpSecurityDescriptor = &secDesc;

        bool bHasSD = false;
        OSVERSIONINFO osVersionInfo;
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (GetVersionEx(&osVersionInfo))
        {
            if (osVersionInfo.dwMajorVersion >= 6)
            {
                // Vista, Longhorn or later;
                bHasSD = true;
            }
        }

        if (bHasSD)
        {
            ConvertStringSecurityDescriptorToSecurityDescriptorW(L"S:(ML;;NW;;;LW)",   // this means "low integrity"
                                                                 SDDL_REVISION_1, &pSD, nullptr);

            GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl,
                                      &fSaclDefaulted);

            SetSecurityDescriptorSacl(secAttr.lpSecurityDescriptor, TRUE,
                                      pSacl, FALSE);
        }

        InitializeSecurityDescriptor(secAttr.lpSecurityDescriptor,
                                     SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0,
                                  FALSE);

        g_sharedFile =
            CreateFileMappingW(INVALID_HANDLE_VALUE,
                               &secAttr,                     // default security
                               PAGE_READWRITE,               // read/write access
                               0,                            // max. object size
                               PWRPROF_SHARED_BUFFER_SIZE,   // buffer size
                               PWR_PROF_SHARED_OBJ);         // name of mapping object
    }

    if (nullptr != g_sharedFile)
    {
        g_pSharedBuffer =
            (unsigned char*) MapViewOfFile(g_sharedFile,                   // handle to mapping object
                                           FILE_MAP_READ | FILE_MAP_WRITE, // read/write permission
                                           0, 0, PWRPROF_SHARED_BUFFER_SIZE);

        if (nullptr != g_pSharedBuffer)
        {
            memset(g_pSharedBuffer, 0, PWRPROF_SHARED_METADATA_SIZE);
            ret = 0;
            g_isInitialized = true;
        }
    }

    return ret;
}

// SetMarkers: Set marker record
int SetMarkers(MarkerTag* pTag)
{
    uint32 prev = 0;
    bool done = false;
    uint32* pMarkerCnt = 0;
    MarkerTag* pBuffer = nullptr;
    int ret = 0;
    uint32 offset = 0;

    if ((nullptr != pTag) && (nullptr != g_pSharedBuffer))
    {
        do
        {
            // Check if buffer is busy. If not, set the busy flag and read data
            prev = InterlockedCompareExchange((uint32*) g_pSharedBuffer, 1, 0);

            // Buffer is not busy, read the content
            if (prev == 0)
            {
                pMarkerCnt = (uint32*)&g_pSharedBuffer[sizeof(uint32)];

                if (*pMarkerCnt < PWR_MAX_MARKER_CNT)
                {
                    offset = 2 * sizeof(uint32) + *pMarkerCnt * (uint32)sizeof(MarkerTag);
                    pBuffer = (MarkerTag*)&g_pSharedBuffer[offset];
                    memcpy(pBuffer, pTag, sizeof(MarkerTag));
                    (*pMarkerCnt)++;
                    ATOMIC_SET((uint32*)&g_pSharedBuffer[0], 0);
                }

                done = true;
            }
        }
        while (!done);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

// amdtPowerBeginMarker: Add begin marker
extern "C"
int AL_API_CALL amdtPowerBeginMarker(const char* szMarkerName, unsigned char* buffer)
{
    int ret = 0;
    MarkerTag marker;

    if (false == g_isInitialized)
    {
        PwrInitializeSharedBuffer();
    }

    if (NULL != szMarkerName)
    {
        memset(&marker, 0, sizeof(MarkerTag));
        marker.m_pid = (uint32)GetCurrentProcessId();
        marker.m_state = PWR_MARKER_ENABLE;
        memcpy(marker.m_name, szMarkerName, PWR_MARKER_BUFFER_SIZE);
        memcpy(marker.m_userBuffer, buffer, PWR_MARKER_BUFFER_SIZE);

        ret = SetMarkers(&marker);
    }

    return ret;
}

// amdtPowerEndMarker: stop the marker
extern "C"
int AL_API_CALL amdtPowerEndMarker(const char* szMarkerName)
{
    int ret = 0;
    MarkerTag marker;

    if (NULL != szMarkerName)
    {
        marker.m_state = PWR_MARKER_DISABLE;
        memcpy(marker.m_name, szMarkerName, PWR_MARKER_BUFFER_SIZE);
        ret = SetMarkers(&marker);
    }

    return ret;
}
