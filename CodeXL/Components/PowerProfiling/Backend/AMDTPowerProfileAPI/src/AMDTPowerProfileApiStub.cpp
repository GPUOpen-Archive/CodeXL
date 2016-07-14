//=============================================================
// (c) 2014 Advanced Micro Devices, Inc.
//
/// \author CodeXL Developer Tools
/// \version $Revision: $
/// \brief Public API implementation for CodeXL Power Profiler
//
//=============================================================
#include <ppCountersStringConstants.h>

AMDTUInt32 g_platformId = PLATFORM_CARRIZO;
#ifdef LINUX
    #pragma GCC diagnostic ignored "-Wwrite-strings"
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    #pragma GCC diagnostic ignored "-Wunused-variable"
    #pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    #pragma GCC diagnostic ignored "-Wunused-value"
#endif

static char g_FilePath[MAX_OUTPUT_FILE_PATH] = { '\0', 0 };

// Macros
#define IS_PROFILE_MODE_OFFLINE (g_profileMode == AMDT_PWR_PROFILE_MODE_OFFLINE)
#define IS_PROFILE_MODE_ONLINE (g_profileMode == AMDT_PWR_PROFILE_MODE_ONLINE)

#define PP_INVALID_COUNTER_ID    0xFFFF

// Clock for stub implementation.
static AMDTUInt64 gs_ppClock = 0;

AMDTResult PwrStartHistogram_stub();

// Kaveri Stub
// #define PP_STUB_SUPPORTED_COUNTERS       27

// Carrizo Stub
#define PP_STUB_SUPPORTED_COUNTERS       51

// active counters
typedef gtMap <AMDTUInt32, bool> PwrCounters;
PwrCounters g_activeCounters;
#define MAX_SAMPLES_CNT 100
#define MAX_SUPPORTED_COUNTERS 51
// Global List to hold counter data
static AMDTPwrSample counterData[MAX_SAMPLES_CNT];
static AMDTUInt32 nbrSupportedCounters = PP_STUB_SUPPORTED_COUNTERS; // cu0, cu1, apu

static AMDTPwrSample aSample[1];


vector <AMDTPwrCounterValue> g_counterValues;
static AMDTPwrCounterValue coreCefValue[1];

static AMDTFloat32 CU0_POWER_BASE = 2.5;
static AMDTFloat32 CU1_POWER_BASE = 2.0;
// static AMDTFloat32 IGPU_POWER_BASE = 15.0;
// static AMDTFloat32 PACKAGE_POWER_BASE = 40.0;

// Base for all the supported counters
static AMDTFloat32 counterBaseValues[MAX_SUPPORTED_COUNTERS] =
{
    (AMDTFloat32)1.0,    // pcie power
    (AMDTFloat32)1.0,    // mem power
    (AMDTFloat32)50.0,    // apu power
    (AMDTFloat32)1.0,    // display power
    (AMDTFloat32)8.0,     // cu0 power
    (AMDTFloat32)46.0,    // cu0 temp
    (AMDTFloat32)20000,  // core-0 process id
    (AMDTFloat32)1400.0, // core-0 frequency
    (AMDTFloat32)4.0,      // core-0 pstate
    (AMDTFloat32)30000,  // core-1 process id
    (AMDTFloat32)1500.0, // core-1 frequency
    (AMDTFloat32)5.0,      // core-1 pstate
    (AMDTFloat32)15.0,     // cu1 power
    (AMDTFloat32)56.0,    // cu1 temp
    (AMDTFloat32)40000,  // core-0 process id
    (AMDTFloat32)1600.0, // core-0 frequency
    (AMDTFloat32)4.0,      // core-0 pstate
    (AMDTFloat32)50000,  // core-1 process id
    (AMDTFloat32)1600.0, // core-1 frequency
    (AMDTFloat32)5.0,      // core-1 pstate
    (AMDTFloat32)15.0,   // igpu power
    (AMDTFloat32)200.0,   // igpu freq
    (AMDTFloat32)46.0,    // igpr temp
    (AMDTFloat32)0.2,     // svi2 core volt
    (AMDTFloat32)200.0,     // svi2 core amp
    (AMDTFloat32)0.2,     // svi2 nb volt
    (AMDTFloat32)200.0,  // svi2 nb current
    (AMDTFloat32)0.0     // dummy
};

// Base for all the supported counters: {Base Value, Maximum delta}
static std::pair<AMDTFloat32, AMDTFloat32> counterBaseValuesCz[MAX_SUPPORTED_COUNTERS] =
{
    { (AMDTFloat32)      17.00, (AMDTFloat32)     1.0 },       //0.         total-apu-power
    { (AMDTFloat32)       0.80, (AMDTFloat32)     0.0 },       //1.         vddio-power
    { (AMDTFloat32)       3.50, (AMDTFloat32)     0.2 },       //2.         vddnb-power
    { (AMDTFloat32)       1.40, (AMDTFloat32)     0.1 },       //3.         vddp-power
    { (AMDTFloat32)       0.02, (AMDTFloat32)     0.01 },      //4.         uvd-power
    { (AMDTFloat32)       0.03, (AMDTFloat32)     0.01 },      //5.         vce-power
    { (AMDTFloat32)       0.01, (AMDTFloat32)     0.001 },     //6.         acp-power
    { (AMDTFloat32)       1.00, (AMDTFloat32)     0.01 },      //7.         unb-power
    { (AMDTFloat32)       1.65, (AMDTFloat32)     0.01 },      //8.         smu-power
    { (AMDTFloat32)       0.80, (AMDTFloat32)     0.00 },      //9.         roc-power
    { (AMDTFloat32)       2.50, (AMDTFloat32)     0.50 },      //10.        cpu-cu0-power
    { (AMDTFloat32)      46.00, (AMDTFloat32)     2.00 },      //11.        cpu-cu0-temp
    { (AMDTFloat32)      60.00, (AMDTFloat32)    10.00 },      //12.        cpu-cu0-c0-residency
    { (AMDTFloat32)       0.00, (AMDTFloat32)    10.00 },      //13.        cpu-cu0-c1-residency
    { (AMDTFloat32)       0.00, (AMDTFloat32)    10.00 },      //14.        cpu-cu0-cc6-residency
    { (AMDTFloat32)       0.00, (AMDTFloat32)    10.00 },      //15.        cpu-cu0-pc6-residency
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //16.        cpu-core0-process-id
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //17.        cpu-core0-thread-id
    { (AMDTFloat32)     1900.00, (AMDTFloat32)   500.00 },     //18.        cpu-core0-frequency
    { (AMDTFloat32)        1.00, (AMDTFloat32)     8.00 },     //19.        cpu-core0-p-state
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //20.        cpu-core1-process-id
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //21.        cpu-core1-thread-id
    { (AMDTFloat32)     2300.00, (AMDTFloat32)   500.00 },     //22.        cpu-core1-frequency
    { (AMDTFloat32)        1.00, (AMDTFloat32)     8.00 },     //23.        cpu-core1-p-state
    { (AMDTFloat32)        2.00, (AMDTFloat32)     0.50 },     //24.        cpu-cu1-power
    { (AMDTFloat32)        48.0, (AMDTFloat32)     3.00 },     //25.        cpu-cu1-temp
    { (AMDTFloat32)      60.00, (AMDTFloat32)    10.00 },      //26.        cpu-cu1-c0-residency
    { (AMDTFloat32)       0.00, (AMDTFloat32)    10.00 },      //27.        cpu-cu1-c1-residency
    { (AMDTFloat32)       0.00, (AMDTFloat32)    10.00 },      //28.        cpu-cu1-cc6-residency
    { (AMDTFloat32)       0.00, (AMDTFloat32)    10.00 },      //29.        cpu-cu1-pc6-residency
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //30.         cpu-core2-process-id
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //31.         cpu-core2-thread-id
    { (AMDTFloat32)     2700.00, (AMDTFloat32)   500.00 },     //32.         cpu-core2-frequency
    { (AMDTFloat32)        1.00, (AMDTFloat32)     8.00 },     //33.         cpu-core2-p-state
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //34.         cpu-core3-process-id
    { (AMDTFloat32)    20000.00, (AMDTFloat32)     0.00 },     //35.         cpu-core3-thread-id
    { (AMDTFloat32)      900.00, (AMDTFloat32)  1000.00 },     //36.         cpu-core3-frequency
    { (AMDTFloat32)        1.00, (AMDTFloat32)     8.00 },     //37.         cpu-core3-p-state
    { (AMDTFloat32)        0.75, (AMDTFloat32)     2.50 },     //38.         vddgfx-power
    { (AMDTFloat32)       50.00, (AMDTFloat32)     3.00 },     //39.         vddgfx-temp
    { (AMDTFloat32)      200.00, (AMDTFloat32)   100.00 },     //40.         igpu-frequency
    { (AMDTFloat32)        0.20, (AMDTFloat32)     0.05 },     //41.         svi2 core volt
    { (AMDTFloat32)      200.00, (AMDTFloat32)    10.00 },     //42.         svi2 core amp
    { (AMDTFloat32)        0.20, (AMDTFloat32)     0.05 },     //43.         svi2 nb volt
    { (AMDTFloat32)      200.00, (AMDTFloat32)    10.00 },     //44.         svi2 nb current
    { (AMDTFloat32)       60.00, (AMDTFloat32)    30.00 },     //45.         BN-dgpu-power
    { (AMDTFloat32)      600.00, (AMDTFloat32)   400.00 },     //46.         BN-dgpu-avg-frequency
    { (AMDTFloat32)       70.00, (AMDTFloat32)    10.00 },     //47.         BN-dgpu-temp
    { (AMDTFloat32)       70.00, (AMDTFloat32)    25.00 },     //48.         TN-dgpu-power
    { (AMDTFloat32)      750.00, (AMDTFloat32)   350.00 },     //49.         TN-dgpu-avg-frequency
    { (AMDTFloat32)       75.00, (AMDTFloat32)    15.00 }      //50.         TN-dgpu-temp

};

// The histogram thread specific variables
struct HistogramThreadData
{
    AMDTUInt32                  m_clientID;
    bool                        m_stopped;
    AMDTPwrSample*              m_counterData; // The data Buffer to hold the counters
    AMDTUInt32                  m_totalCount; // The max number of counters we can hold
    AMDTUInt32                  m_currentCount; // How many we have filled in til now.
    osCriticalSection           m_cs;
    HistogramThreadData(AMDTPwrSample* cntData) :
        m_clientID(0), m_stopped(false), m_counterData(cntData), m_totalCount(0), m_currentCount(0), m_cs()
    {
    };
    ~HistogramThreadData()
    {
    }
    HistogramThreadData() = delete;
};

static HistogramThreadData g_threadData(counterData);
static AMDTPwrProfileState g_profileState;

static AMDTUInt32 g_durationCount = 0;
static AMDTUInt32 g_recordId = 0;
static AMDTPwrSystemTime g_profileStartTime;

void PwrGetSystemTime(AMDTPwrSystemTime* systemTime)
{
    // The Windows ticks are in 100 nanoseconds (10^-7).
#define WINDOWS_TICK_PER_SEC 10000000

    // The windows epoch starts 1601-01-01 00:00:00.
    // It's 11644473600 seconds before the UNIX/Linux epoch (1970-01-01 00:00:00).
#define SEC_TO_UNIX_EPOCH 11644473600LL

#if ( defined (_WIN32) || defined (_WIN64) )
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);

    // Convert this file time to our system time
    ULARGE_INTEGER time;
    time.HighPart = fileTime.dwHighDateTime;
    time.LowPart = fileTime.dwLowDateTime;

    systemTime->m_second = static_cast<AMDTUInt32>((time.QuadPart / WINDOWS_TICK_PER_SEC) - SEC_TO_UNIX_EPOCH);
    systemTime->m_microSecond = static_cast<AMDTUInt32>((time.QuadPart - systemTime->m_second) / 10);

#else // LINUX
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    systemTime->m_second = tv.tv_sec;
    systemTime->m_microSecond = tv.tv_usec;
#endif
}


// Helper Function that fills the data into the thread specific array
// We should call AMDTPwrGetData function that will fecth the data from the driver in this  function
void FillData(HistogramThreadData* threadData)
{
    AMDTUInt32 cnt = threadData->m_currentCount++;

    if (cnt > threadData->m_totalCount)
    {
        // Buffer full. Reset
        threadData->m_currentCount = 0;
        cnt = 0;
    }

    // For the stub , we send back 1 sample
    // CU Power for 2 CU's
    // CEF for Core0
    AMDTPwrSample* pwrSample = &threadData->m_counterData[cnt];

    pwrSample->m_numOfValues = 3;
    PwrGetSystemTime(&pwrSample->m_systemTime);

    // Allocate memory for the counter values
    pwrSample->m_counterValues = (AMDTPwrCounterValue*) GetMemoryPoolBuffer(&g_apiMemoryPool,
                                 3 * sizeof(AMDTPwrCounterValue));

    // CU0 Power
    pwrSample->m_counterValues->m_counterID = 0;
    pwrSample->m_counterValues->m_counterValue = CU0_POWER_BASE + (float)((double)rand() / (RAND_MAX));
    pwrSample->m_counterValues++;

    // CU1 Power
    pwrSample->m_counterValues->m_counterID = 1;
    pwrSample->m_counterValues->m_counterValue = CU1_POWER_BASE + (float)((double)rand() / (RAND_MAX));
    pwrSample->m_counterValues++;

    // Core0 CEF
    pwrSample->m_counterValues->m_counterID = 2;
    pwrSample->m_counterValues->m_counterValue = (float)((double)rand() / (RAND_MAX) * 1400);
    pwrSample->m_counterValues++;
}

// Fetch the data from the driver; Process the raw data and store in the m_counterData array.
void GetCounters(HistogramThreadData* threadData)
{
    osCriticalSectionLocker locker(threadData->m_cs);
    FillData(threadData);
}

// AMDTPwrProfileInitialize: This function loads and initializes the AMDT Power Profile drivers.
// This function should be the first one to be called.
AMDTResult AMDTPwrProfileInitialize(AMDTPwrProfileMode profileMode)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if ((profileMode != AMDT_PWR_PROFILE_MODE_ONLINE)
        && (profileMode != AMDT_PWR_PROFILE_MODE_OFFLINE))
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        g_profileMode = profileMode;
        g_profileState = AMDT_PWR_PROFILE_STATE_IDLE;

        g_activeCounters.clear();
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (nullptr != g_apiMemoryPool.m_pBase)
        {
            ReleaseMemoryPool(&g_apiMemoryPool);
        }

        // Create memory pool for API layer
        ret = CreateMemoryPool(&g_apiMemoryPool, API_POOL_SIZE);
    }

    return ret;
}

// AMDTPwrGetSystemTopology: This function provides device tree which represents
// the current system topology Here node as well as components inside the nodes
// are also considered as devices. Each device in the tree has none or more next
// devices in the same hierarchy and none or more sub devices.
enum StubDeviceIds
{
    STUB_DEVICE_ID_SYSTEM,          // = 0
    STUB_DEVICE_ID_PACKAGE,         // = 1
    STUB_DEVICE_ID_CPU_CU0,         // = 2
    STUB_DEVICE_ID_CPU_CORE0,       // = 3
    STUB_DEVICE_ID_CPU_CORE1,       // = 4
    STUB_DEVICE_ID_CPU_CU1,         // = 5
    STUB_DEVICE_ID_CPU_CORE2,       // = 6
    STUB_DEVICE_ID_CPU_CORE3,       // = 7
    STUB_DEVICE_ID_IGPU,            // = 8
    STUB_DEVICE_ID_SVI2,            // = 9
    STUB_DEVICE_ID_DGPU1,           // = 10
    STUB_DEVICE_ID_DGPU2            // = 11
};

static AMDTPwrDevice system0 =
{
    AMDT_PWR_DEVICE_SYSTEM,           //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_SYSTEM,            //   m_deviceID       - Device id provided by backend
    "System",                         //   m_pName          - Name of the device
    "Root Device",                    //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice package0 =
{
    AMDT_PWR_DEVICE_PACKAGE,          //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_PACKAGE,           //   m_deviceID       - Device id provided by backend
    "Node-0",                         //   m_pName          - Name of the device
    "First node",                     //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice cu0 =
{
    AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT, //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_CPU_CU0,           //   m_deviceID       - Device id provided by backend
    "CU-0",                           //   m_pName          - Name of the device
    "Compute Unit 0",                 //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice core0 =
{
    AMDT_PWR_DEVICE_CPU_CORE,         //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_CPU_CORE0,         //   m_deviceID       - Device id provided by backend
    "Core-0",                         //   m_pName          - Name of the device
    "Core-0",                         //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice core1 =
{
    AMDT_PWR_DEVICE_CPU_CORE,         //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_CPU_CORE1,         //   m_deviceID       - Device id provided by backend
    "Core-1",                         //   m_pName          - Name of the device
    "Core-1",                         //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice cu1 =
{
    AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT, //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_CPU_CU1,           //   m_deviceID       - Device id provided by backend
    "CU-1",                           //   m_pName          - Name of the device
    "Compute Unit 1",                 //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice core2 =
{
    AMDT_PWR_DEVICE_CPU_CORE,         //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_CPU_CORE2,         //   m_deviceID       - Device id provided by backend
    "Core-2",                         //   m_pName          - Name of the device
    "Core-2",                         //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice core3 =
{
    AMDT_PWR_DEVICE_CPU_CORE,         //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_CPU_CORE3,         //   m_deviceID       - Device id provided by backend
    "Core-3",                         //   m_pName          - Name of the device
    "Core-3",                         //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice igpu0 =
{
    AMDT_PWR_DEVICE_INTERNAL_GPU,     //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_IGPU,              //   m_deviceID       - Device id provided by backend
    "iGPU",                           //   m_pName          - Name of the device
    "Integrated GPU",                 //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice svi2 =
{
    AMDT_PWR_DEVICE_SVI2,             //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_SVI2,              //   m_deviceID       - Device id provided by backend
    "svi2 telemetry",                 //   m_pName          - Name of the device
    "Serial Voltage Interface",       //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice dgpu1 =
{
    AMDT_PWR_DEVICE_EXTERNAL_GPU,     //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_DGPU1,             //   m_deviceID       - Device id provided by backend
    "dGPU1",                          //   m_pName          - Name of the device
    "First dGPU",                     //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

static AMDTPwrDevice dgpu2 =
{
    AMDT_PWR_DEVICE_EXTERNAL_GPU,     //   m_type           - Device type- compute unit/Core/ package/ dGPU
    STUB_DEVICE_ID_DGPU2,             //   m_deviceID       - Device id provided by backend
    "dGPU2",                          //   m_pName          - Name of the device
    "Second dGPU",                     //   m_pDescription   - Family and model name will be embedded
    nullptr,                             //   m_pFirstChild    - Points to the sub-devices of this device
    nullptr,                             //   m_pNextDevice    - Points to the next device at the same hierarchy
};

AMDTResult AMDTPwrGetSystemTopology(AMDTPwrDevice** ppTopology)
{
    /*
    Returns a static tree with the following devices: 2 CUs, 4 cores, PCIe, iGPU, DDR, and Package.
    SYSTEM (Id = 0)
        +--PACKAGE (Id = 1)
            +--CPU_COMPUTE_UNIT (Id = 2)
                + --CPU_CORE (Id = 3)
                + --CPU_CORE (Id = 4)
            +--CPU_COMPUTE_UNIT (Id = 5)
                + --CPU_CORE (Id = 6)
                + --CPU_CORE (Id = 7)
            +--INTEGRATED_GPU (Id = 8)
            +--SVI2 (Id = 9)
        +--DGPU1 (Id = 10)
        +--DGPU2 (Id = 11)
    */
    AMDTResult ret = AMDT_STATUS_OK;

    system0.m_pFirstChild = &package0;

    package0.m_pFirstChild = &cu0;

    // CU0 2 children as cores and next device CU1;
    cu0.m_pFirstChild = &core0;
    cu0.m_pNextDevice = &cu1;

    // CU1 2 children and next device IGPU0
    cu1.m_pFirstChild = &core2;
    cu1.m_pNextDevice = &igpu0;

    // igpu's next device is SVI2
    igpu0.m_pNextDevice = &svi2;

    // CORE0 next device CORE1
    core0.m_pNextDevice = &core1;

    // CORE 2 Next device CORE 3
    core2.m_pNextDevice = &core3;

    // package0's next device is dGPU1
    package0.m_pNextDevice = &dgpu1;

    // dGPU1's next device is dGPU2
    dgpu1.m_pNextDevice = &dgpu2;

    *ppTopology = &system0;

    return ret;
}


static AMDTPwrCounterDesc g_desc_stub[] =
{
    // Counters for device 1 - Package
    {
        0,                              // m_counterID Counter index
        1,                              // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_PCIECtrl,  // m_name Name of the counter
        "Average PCIe-Controller Power, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,        // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,          // m_aggregation Single/Histogram/Cumulative
        1.0,                            // m_minValue Minimum possible counter value
        65.0,                           // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,        // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        1,                          // m_counterID Counter index
        1,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_MemCtrl,                // m_name Name of the counter
        "Average Memory-Controller Power, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        2,                          // m_counterID Counter index
        1,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_TotalAPU,          // m_name Name of the counter
        "Average APU Power, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        3,                          // m_counterID Counter index
        1,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_DisplayCtrl,                // m_name Name of the counter
        "Average Display-Controller Power, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 2 - CU0
    {
        4,                          // m_counterID Counter index
        2,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_CU0,                // m_name Name of the counter
        "Average CPU Compute Unit Power for the sampling period, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        5,                          // m_counterID Counter index
        2,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Temp_CU0,                // m_name Name of the counter
        "CPU Compute Unit Temperature, measured in Celsius",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        50.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 3 - Core0
    {
        6,                          // m_counterID Counter index
        3,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C0, // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65535.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        7,                          // m_counterID Counter index
        3,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core0,                // m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        8,                          // m_counterID Counter index
        3,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core0,                // m_name Name of the counter
        "CPU Core P-State status",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        0,                        // m_minValue Minimum possible counter value
        7,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 4 - Core1
    {
        9,                          // m_counterID Counter index
        4,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C1, // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65535.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        10,                          // m_counterID Counter index
        4,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core1,                // m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        11,                          // m_counterID Counter index
        4,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core1,                // m_name Name of the counter
        "CPU Core P-State status",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        0,                        // m_minValue Minimum possible counter value
        7,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 5 - CU1
    {
        12,                          // m_counterID Counter index
        5,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_CU1,                // m_name Name of the counter
        "Average CPU Compute Unit Power for the sampling period, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        13,                          // m_counterID Counter index
        5,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Temp_CU1,                // m_name Name of the counter
        "CPU Compute Unit Temperature, measured in Celsius",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        50.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 6 - Core2
    {
        14,                          // m_counterID Counter index
        6,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C2,                // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65535.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        15,                          // m_counterID Counter index
        6,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core2,                // m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        16,                          // m_counterID Counter index
        6,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core2,                // m_name Name of the counter
        "CPU Core P-State status",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        0,                        // m_minValue Minimum possible counter value
        7,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 7 - Core3
    {
        17,                          // m_counterID Counter index
        7,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C3,                // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65535.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        18,                          // m_counterID Counter index
        7,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core3,                // m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        19,                          // m_counterID Counter index
        7,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core3,                // m_name Name of the counter
        "CPU Core P-State status",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        0,                        // m_minValue Minimum possible counter value
        7,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 8 - iGPU
    {
        20,                          // m_counterID Counter index
        8,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_IGPU,                // m_name Name of the counter
        "Average Integrated-GPU Power, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        21,                          // m_counterID Counter index
        8,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Temp_IGPU,                // m_name Name of the counter
        "Average Integrated-GPU Temperature for the sampling period, measured in Celsius",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        50.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        22,                          // m_counterID Counter index
        8,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_IGPU,                // m_name Name of the counter
        "Integrated-GPU Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        1000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    // Counters for device 9 - SVI2
    {
        23,                          // m_counterID Counter index
        9,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Voltage_CPUCore,                // m_name Name of the counter
        "CPU Core Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_VOLTAGE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        5.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_VOLT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        24,                          // m_counterID Counter index
        9,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Current_CPUCore,                // m_name Name of the counter
        "CPU Core Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_CURRENT,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        500.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MILLI_AMPERE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        25,                          // m_counterID Counter index
        9,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Voltage_NB,                // m_name Name of the counter
        "North-Bridge Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_VOLTAGE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        5.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_VOLT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        26,                          // m_counterID Counter index
        9,                          // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Current_NB,                // m_name Name of the counter
        "North-Bridge Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_CURRENT,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        500.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MILLI_AMPERE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

#if 0
    {
        27,                                  // m_counterID Counter index
        4,                              // m_deviceId Device id for which counter belongs to Core0
        "Core 0 Frequency Histogram",   // m_name Name of the counter
        "Core 0 Frequency Histogram",   // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_HISTOGRAM,       // m_aggregation Single/Histogram/Cumulative
        0.0,                            // m_minValue Minimum possible counter value
        1400.0,                         // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,  // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    }
#endif
};

// Counter list for Carrizo
static AMDTPwrCounterDesc g_descStubCz[MAX_SUPPORTED_COUNTERS] =
{
    //0 total-apu-power
    {
        0,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_TotalAPU,  // m_name Name of the counter
        "Average APU Power for the sampling period, reported in Watts. This is an estimated consumption value which is calculated based on APU activity levels.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //1.vddio-power
    {
        1,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_IOCtrl,// m_name Name of the counter
        "Average VddIO Power for the sampling period, reported in Watts.",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //2.vddnb-power
    {
        2,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_NB,    // m_name Name of the counter
        "Average North Bridge Vdd Power for the sampling period, reported in Watts.",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //3.vddp-power
    {
        3,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_GFXCtrl, // m_name Name of the counter
        "Average Vddp Power for the sampling period, reported in Watts.",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //4.uvd-power
    {
        4,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_UVD,   // m_name Name of the counter
        "Power consumed by high performance H.264 Universal Video Decoder, measured in Watt.",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //5.vce-power
    {
        5,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_VCE,   // m_name Name of the counter
        "Power consumed by Video Compression Engine, measured in Watt",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //6.acp-power
    {
        6,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_ACP,   // m_name Name of the counter
        "Power consumed by Audio Co-processor, measured in Watt",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //7.unb-power
    {
        7,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_UNB,   // m_name Name of the counter
        "Power consumed by Unified North Bridge, measured in Watt",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //8.smu-power
    {
        8,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_SMU,   // m_name Name of the counter
        "Power consumed by SMU micro controller, measured in Watt",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //9.roc-power
    {
        9,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,     // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_RoC,   // m_name Name of the counter
        "Power for rest of the chip. Ideally this includes VDDA power",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //10.cpu-cu0-power
    {
        10,                          // m_counterID Counter index
        STUB_DEVICE_ID_PACKAGE,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_CU0,    // m_name Name of the counter
        "Measured CPU Compute Unit Average Temperature, reported in Celsius. The reported value is normalized and scaled, relative to the specific processor's maximum operating temperature. This value can be used to indicate rise and decline of temperature.",
        AMDT_PWR_CATEGORY_POWER,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //11.cpu-cu0-temp
    {
        11,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU0,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Temp_CU0,     // m_name Name of the counter
        "CPU Compute Unit Temperature, measured in Celsius",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        50.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //12. cpu-cu0-c0-residency
    {
        12,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU0,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_C0Residency_CU0,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in C0 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //13. cpu-cu0-c1-residency
    {
        13,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU0,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_C1Residency_CU0,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in C1 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //14. cpu-cu0-cc6-residency
    {
        14,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU0,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_CC6Residency_CU0,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in CC6 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //15. cpu-cu0-pc6-residency
    {
        15,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU0,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PC6Residency_CU0,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in PC6 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //16.cpu-core0-process-id
    {
        16,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE0,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C0,             // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        65535.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //17.cpu-core0-thread-id
    {
        17,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE0,    // m_deviceId Device id for which counter belongs to
        "Thread Id-C0",              // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65535.0,                     // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //18.cpu-core0-frequency
    {
        18,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE0,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core0,// m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY, // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //19.cpu-core0-p-state
    {
        19,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE0,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core0, // m_name Name of the counter
        "CPU Core P-State status",   // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,      // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        0,                           // m_minValue Minimum possible counter value
        7,                           // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //20.cpu-core1-process-id
    {
        20,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE1,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C1,             // m_name Name of the counter
        "Default C-attr",            // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65535.0,                     // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //21.cpu-core1-thread-id
    {
        21,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE1,    // m_deviceId Device id for which counter belongs to
        "Thread Id-C1",              // m_name Name of the counter
        "Default C-attr",            // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65535.0,                     // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //22.cpu-core1-frequency
    {
        22,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE1,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core1,// m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY, // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //23.cpu-core1-p-state
    {
        23,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE1,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core1, // m_name Name of the counter
        "CPU Core P-State status",   // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,      // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        0,                           // m_minValue Minimum possible counter value
        7,                           // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //24.cpu-cu1-power
    {
        24,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU1,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_CU1,    // m_name Name of the counter
        "Average CPU Compute Unit Power for the sampling period, measured in Watt",
        AMDT_PWR_CATEGORY_POWER,     // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,     // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //25.cpu-cu1-temp
    {
        25,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU1,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Temp_CU1,     // m_name Name of the counter
        "CPU Compute Unit Temperature, measured in Celsius",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        50.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //26. cpu-cu1-c0-residency
    {
        26,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU1,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_C0Residency_CU1,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in C0 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //27. cpu-cu1-c1-residency
    {
        27,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU1,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_C1Residency_CU1,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in C1 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //28. cpu-cu1-cc6-residency
    {
        28,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU1,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_CC6Residency_CU1,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in CC6 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //29. cpu-cu1-pc6-residency
    {
        29,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CU1,      // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PC6Residency_CU1,     // m_name Name of the counter
        "Percentage of the sample interval time during which the CPU Compute Unit was in PC6 state.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        100.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_PERCENT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //30.cpu-core2-process-id
    {
        30,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE2,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C2,             // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65535.0,                     // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //31.cpu-core2-thread-id
    {
        31,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE2,    // m_deviceId Device id for which counter belongs to
        "Thread Id-C2",              // m_name Name of the counter
        "Default C-attr",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65535.0,                     // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //32.cpu-core2-frequency
    {
        32,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE2,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core2,// m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",
        AMDT_PWR_CATEGORY_FREQUENCY, // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,// m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //33.cpu-core2-p-state
    {
        33,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE2,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core2,                // m_name Name of the counter
        "CPU Core P-State status",   // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,      // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        0,                           // m_minValue Minimum possible counter value
        7,                           // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //34.cpu-core3-process-id
    {
        34,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE3,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Process_Id_C3,             // m_name Name of the counter
        "Default C-attr",            // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65535.0,                     // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    //35.cpu-core3-thread-id
    {
        35,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE3,    // m_deviceId Device id for which counter belongs to
        "Thread Id-C3",              // m_name Name of the counter
        "Default C-attr",            // m_description Description of the counter
        AMDT_PWR_CATEGORY_PROCESS,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65535.0,                     // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    //36.cpu-core3-frequency
    {
        36,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE3,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_Core3,// m_name Name of the counter
        "CPU Core Effective Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY, // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    //37.cpu-core3-p-state
    {
        37,                          // m_counterID Counter index
        STUB_DEVICE_ID_CPU_CORE3,    // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_PState_Core3, // m_name Name of the counter
        "CPU Core P-State status",   // m_description Description of the counter
        AMDT_PWR_CATEGORY_DVFS,      // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        0,                           // m_minValue Minimum possible counter value
        7,                           // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_COUNT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //38.vddgfx-power
    {
        38,                          // m_counterID Counter index
        STUB_DEVICE_ID_IGPU,         // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Power_GFX,    // m_name Name of the counter
        "Average vddgfx Power, measured in Watt",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,     // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        65.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,     // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //39.vddgfx-temp
    {
        39,                          // m_counterID Counter index
        STUB_DEVICE_ID_IGPU,         // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Temp_GFX,     // m_name Name of the counter
        "Average Integrated-GPU Temperature for the sampling period, measured in Celsius",
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        50.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //40.igpu-frequency
    {
        40,                          // m_counterID Counter index
        STUB_DEVICE_ID_IGPU,         // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_AvgFreq_GFX,  // m_name Name of the counter
        "Integrated-GPU Frequency, measured in GHz",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY, // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        1000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    // Counters for device 9 - SVI2
    {
        41,                          // m_counterID Counter index
        STUB_DEVICE_ID_SVI2,         // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Voltage_CPUCore,                // m_name Name of the counter
        "CPU Core Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_VOLTAGE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,      // m_aggregation Single/Histogram/Cumulative
        1.0,                        // m_minValue Minimum possible counter value
        5.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_VOLT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        42,                          // m_counterID Counter index
        STUB_DEVICE_ID_SVI2,         // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Current_CPUCore,                // m_name Name of the counter
        "CPU Core Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_CURRENT,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        500.0,                       // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MILLI_AMPERE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        43,                          // m_counterID Counter index
        STUB_DEVICE_ID_SVI2,         // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Voltage_NB,   // m_name Name of the counter
        "North-Bridge Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_VOLTAGE,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        5.0,                         // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_VOLT,     // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    {
        44,                          // m_counterID Counter index
        STUB_DEVICE_ID_SVI2,         // m_deviceId Device id for which counter belongs to
        PP_STR_Counter_Current_NB,   // m_name Name of the counter
        "North-Bridge Voltage measured in Volts by the SVI2 (Serial Voltage identification Interface 2.0) Telemetry",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_CURRENT,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        500.0,                       // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MILLI_AMPERE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    //45.dgpu1-power
    {
        45,                          // m_counterID Counter index
        STUB_DEVICE_ID_DGPU1,         // m_deviceId Device id for which counter belongs to
        "BN " PP_STR_Counter_Power_DGPU,   // m_name Name of the counter
        "Average Discrete-GPU Power for the sampling period, reported in Watts. This is an estimated consumption value which is calculated based on dGPU activity levels.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        500.0,                       // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    //46.dgpu1-frequency
    {
        46,                          // m_counterID Counter index
        STUB_DEVICE_ID_DGPU1,    // m_deviceId Device id for which counter belongs to
        "BN " PP_STR_Counter_AvgFreq_DGPU,// m_name Name of the counter
        "Average Discrete-GPU Frequency for the sampling period, reported in MHz.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY, // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //47.dgpu-temperature
    {
        47,                          // m_counterID Counter index
        STUB_DEVICE_ID_DGPU1,         // m_deviceId Device id for which counter belongs to
        "BN " PP_STR_Counter_Temp_DGPU,     // m_name Name of the counter
        "Measured Discrete-GPU Average Temperature, reported in Celsius. The reported value is normalized and scaled, relative to the specific processor's maximum operating temperature. This value can be used to indicate rise and decline of temperature.",
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        50.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    //48.dgpu2-power
    {
        48,                          // m_counterID Counter index
        STUB_DEVICE_ID_DGPU2,         // m_deviceId Device id for which counter belongs to
        "TN " PP_STR_Counter_Power_DGPU,   // m_name Name of the counter
        "Average Discrete-GPU Power for the sampling period, reported in Watts. This is an estimated consumption value which is calculated based on dGPU activity levels.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_POWER,   // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        500.0,                       // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_WATT,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },

    //49.dgpu2-frequency
    {
        49,                          // m_counterID Counter index
        STUB_DEVICE_ID_DGPU2,    // m_deviceId Device id for which counter belongs to
        "TN " PP_STR_Counter_AvgFreq_DGPU,// m_name Name of the counter
        "Average Discrete-GPU Frequency for the sampling period, reported in MHz.",     // m_description Description of the counter
        AMDT_PWR_CATEGORY_FREQUENCY, // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        3000.0,                      // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_MEGA_HERTZ,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    },
    //50.dgpu-temperature
    {
        50,                          // m_counterID Counter index
        STUB_DEVICE_ID_DGPU2,         // m_deviceId Device id for which counter belongs to
        "TN " PP_STR_Counter_Temp_DGPU,     // m_name Name of the counter
        "Measured Discrete-GPU Average Temperature, reported in Celsius. The reported value is normalized and scaled, relative to the specific processor's maximum operating temperature. This value can be used to indicate rise and decline of temperature.",
        AMDT_PWR_CATEGORY_TEMPERATURE,    // m_category Power/Freq/Temperature
        AMDT_PWR_VALUE_SINGLE,       // m_aggregation Single/Histogram/Cumulative
        1.0,                         // m_minValue Minimum possible counter value
        50.0,                        // m_maxValue Maximum possible counter value
        AMDT_PWR_UNIT_TYPE_CENTIGRADE,    // m_units Seconds/MHz/Joules/Watts/Volt/Ampere
    }
    // NOTE:
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Don't forget to update the definition of "PP_STUB_SUPPORTED_COUNTERS" when
    // adding more counters!
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

};

AMDTResult AMDTPwrGetDeviceCounters(AMDTPwrDeviceId deviceID,
                                    AMDTUInt32* pNumCounters,
                                    AMDTPwrCounterDesc** ppCounterDescs)
{
    AMDTResult ret = AMDT_STATUS_OK;
    (void)deviceID;
    //For the Stub
    // returns a static list of counters regardless of the device  CU Power
    *pNumCounters = nbrSupportedCounters;

    if (PLATFORM_CARRIZO == g_platformId)
    {
        *ppCounterDescs = g_descStubCz;
    }
    else if (PLATFORM_KAVERI == g_platformId)
    {
        *ppCounterDescs = g_desc_stub;
    }

    return ret;
}

// AMDTPwrGetCounterDesc: This API provides the description of the counter requested
// by the client with counter index
AMDTResult AMDTPwrGetCounterDesc(AMDTUInt32 counterID,
                                 AMDTPwrCounterDesc* pCounterDesc)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (counterID >= nbrSupportedCounters)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (PLATFORM_CARRIZO == g_platformId)
        {
            *pCounterDesc = g_descStubCz[counterID];
        }
        else if (PLATFORM_KAVERI == g_platformId)
        {
            *pCounterDesc = g_desc_stub[counterID];
        }
    }

    return ret;
}

// AMDTPwrEnableCounter: This API will enable the counter to be sampled. This
// API can be used even after the profile run is started.
AMDTResult AMDTPwrEnableCounter(AMDTUInt32 counterID)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (counterID >= nbrSupportedCounters)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        PwrCounters::iterator Iter = g_activeCounters.find(counterID);

        if (Iter == g_activeCounters.end())
        {
            g_activeCounters.insert(PwrCounters::value_type(counterID, true));
        }
        else
        {
            // counter already enables
            ret = AMDT_ERROR_COUNTER_ALREADY_ENABLED;
        }
    }

    return ret;
}

// AMDTPwrEnableAllCounters: This API will enable all the available counters.
// This API cannot be used once a profile run is started.
AMDTResult AMDTPwrEnableAllCounters()
{
    AMDTResult ret = AMDT_STATUS_OK;

    AMDTUInt32 i;

    for (i = 0; i < nbrSupportedCounters; i++)
    {
        PwrCounters::iterator Iter = g_activeCounters.find(i);

        if (Iter == g_activeCounters.end())
        {
            g_activeCounters.insert(PwrCounters::value_type(i, true));
        }
    }

    return ret;
}

// AMDTPwrGetMinimalTimerSamplingPeriod: This API provide the minimum sampling
// time which can be set by the client.
AMDTResult AMDTPwrGetMinimalTimerSamplingPeriod(AMDTUInt32* pIntervalMilliSec)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pIntervalMilliSec)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pIntervalMilliSec = MINIMAL_SAMPLING_PERIOD;
    }

    return ret;
}

// AMDTPwrSetTimerSamplingPeriod: This API will set the driver to periodically
// sample the counter values and store them in a buffer. This cannot be called
// once the profile run is started.
AMDTResult AMDTPwrSetTimerSamplingPeriod(AMDTUInt32 interval)
{

    AMDTResult ret = AMDT_STATUS_OK;

    if (interval < MINIMAL_SAMPLING_PERIOD)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        g_samplingPeriod = interval;
    }

    return ret;
}

// AMDTPwrSetProfileDataFile: This API specifies the path to a file in which
// the raw profile data records will be stored. This API should be called
// before starting a profile run. This is an OFFLINE mode ONLY API. This
// cannot be called once the profile run in started.
AMDTResult AMDTPwrSetProfileDataFile(const char* pFilePath, AMDTUInt32 len)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (len > MAX_OUTPUT_FILE_PATH)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    strncpy(g_FilePath, pFilePath, len);
    return ret;
}

// AMDTPwrStartProfiling: If the profiler is not running, this will start the
// profiler.
AMDTResult AMDTPwrStartProfiling()
{
    AMDTResult ret = AMDT_STATUS_OK;
    g_profileState = AMDT_PWR_PROFILE_STATE_RUNNING;

    PwrGetSystemTime(&g_profileStartTime);
    g_durationCount = 0;
    return ret;
}

// AMDTPwrStopProfiling: If the profiler is running, this will stop the profile
AMDTResult AMDTPwrStopProfiling()
{
    AMDTResult ret = AMDT_STATUS_OK;

    g_profileState = AMDT_PWR_PROFILE_STATE_IDLE;

    //Reset all the variables
    g_durationCount = 0;
    g_profileStartTime.m_microSecond = 0;
    g_profileStartTime.m_second = 0;
    gs_ppClock = 0;

    return ret;
}

// AMDTPwrPauseProfiling: This API will pause the profiling. The driver and the
// backend will retain the profile configuration details provided by the client.
AMDTResult AMDTPwrPauseProfiling()
{
    AMDTResult ret = AMDT_STATUS_OK;
    g_profileState = AMDT_PWR_PROFILE_STATE_PAUSED;
    return ret;
}

// AMDTPwrResumeProfiling This API will resume the profiling which is in paused
// state.
AMDTResult AMDTPwrResumeProfiling()
{
    AMDTResult ret = AMDT_STATUS_OK;
    g_profileState = AMDT_PWR_PROFILE_STATE_RUNNING;
    return ret;
}

// AMDTPwrGetProfilingState: This API provides the current state of the profile.
AMDTResult  AMDTPwrGetProfilingState(AMDTPwrProfileState* pState)
{
    AMDTResult ret = AMDT_STATUS_OK;
    *pState = g_profileState;
    return ret;
}

// AMDTPwrProfileClose** This API will close the power profiler and unregister
// driver and cleanup all memory allocated during AMDTPwrProfileInitialize.
AMDTResult AMDTPwrProfileClose()
{
    AMDTResult ret = AMDT_STATUS_OK;
    g_profileState = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;

    if (AMDT_STATUS_OK == ret)
    {
        if (nullptr == g_apiMemoryPool.m_pBase)
        {
            ReleaseMemoryPool(&g_apiMemoryPool);
        }
    }

    g_activeCounters.clear();
    return ret;
}

// AMDTPwrSetSampleValueOption:  API to set the sample value options to be
// returned by the AMDTPwrReadAlllEnabledCounters().
AMDTResult AMDTPwrSetSampleValueOption(AMDTSampleValueOption opt)
{
    AMDTResult ret = AMDT_STATUS_OK;
    AMDTPwrProfileState state;

    ret = AMDTPwrGetProfilingState(&state);

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }

        if (AMDT_PWR_PROFILE_STATE_RUNNING == state)
        {
            ret = AMDT_ERROR_PROFILE_ALREADY_STARTED;
        }
    }

    g_outputOption = opt;

    return ret;
}

// AMDTPwrGetSampleValueOption: API to get the sample value option current set for
// the profiler
AMDTResult AMDTPwrGetSampleValueOption(AMDTSampleValueOption* pOpt)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pOpt)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }

        *pOpt = g_outputOption;
    }

    return ret;
}

// AMDTPwrReadAllEnabledCounters: API to read all the counters that are enabled.
// This will NOT read the histogram counters. This can return an array of
// {CounterID, Float-Value}. If there are no new samples, this API will
// return AMDTResult NO_NEW_DATA and pNumOfSamples will point to value of zero. If there
// are new samples, this API will return AMDTResult SUCCESS and pNumOfSamples will point
// to value greater than zero.
AMDTResult AMDTPwrReadAllEnabledCounters(AMDTUInt32* pNumOfSamples,
                                         AMDTPwrSample** ppData)
{
    //returns static list of a single sample with static list of counters  all CU Power.
    //The time tag of the sample will increase by 100ms with each API call.
    AMDTResult ret = AMDT_STATUS_OK;

    // Clock tick.
    gs_ppClock += g_samplingPeriod;

    // Send back 1 sample of each of all counters
    *pNumOfSamples = 1;

    aSample[0].m_numOfValues = g_activeCounters.size();
    aSample[0].m_systemTime = g_profileStartTime;
    g_durationCount++;
    aSample[0].m_elapsedTimeMs = (g_durationCount * g_samplingPeriod);
    aSample[0].m_recordId = g_recordId++;
    AMDTPwrCounterValue counter;

    for (auto iter : g_activeCounters)
    {
        counter.m_counterID = iter.first;

        AMDTPwrCounterDesc desc;
        AMDTPwrGetCounterDesc(iter.first, &desc);

        AMDTFloat32 randomFraction = ((float)((double)rand() / (RAND_MAX)));

        if (PLATFORM_CARRIZO == g_platformId)
        {
            AMDTFloat32 baseValue = counterBaseValuesCz[iter.first].first;
            AMDTFloat32 maxDelta = counterBaseValuesCz[iter.first].second;
            counter.m_counterValue = baseValue + (maxDelta * randomFraction);

            if (AMDT_PWR_CATEGORY_DVFS == desc.m_category)
            {
                counter.m_counterValue = AMDTFloat32(int(counter.m_counterValue) % int(maxDelta));
            }
        }
        else if (PLATFORM_KAVERI == g_platformId)
        {
            if (2 == counter.m_counterID || 5 == counter.m_counterID || 13 == counter.m_counterID || 22 == counter.m_counterID) // APU Power & all temperatures
            {
                counter.m_counterValue = counterBaseValues[counter.m_counterID] + randomFraction;
            }
            else
            {
                counter.m_counterValue = counterBaseValues[counter.m_counterID] * randomFraction;
            }
        }

        g_counterValues.push_back(counter);
    }

    aSample[0].m_counterValues = &g_counterValues[0];
    aSample[0].m_elapsedTimeMs = gs_ppClock;

    *ppData = aSample;


    return ret;
}

// AMDTPwrReadCounterHistogram: API to read ome of the derived counters generate histograms
// from the raw counter values. Since the histogram may contain multiple entries and according
// to the counter values, a derived histogram counter type specific will be used to provide
// the output data.
AMDTResult AMDTPwrReadCounterHistogram(AMDTUInt32 counterID,
                                       AMDTUInt32* pNumEntries,
                                       AMDTPwrHistogram** ppData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    (void)counterID;
    (void)ppData;
    (void)pNumEntries;
    return ret;
}

// AMDTPwrReadCumulativeCounter: API to read one of the derived accumulated counters
// values from the raw counter values.
AMDTResult AMDTPwrReadCumulativeCounter(AMDTUInt32 counterId,
                                        AMDTUInt32* pNumEntries,
                                        AMDTFloat32** ppData)
{
    AMDTResult ret = AMDT_STATUS_OK;
    (void)ppData;
    (void)pNumEntries;
    (void)counterId;
    return ret;
}

//Helper functions

// AMDTPwrGetTimerSamplingPeriod: This API will get the timer sampling period at which
// the samples are collected by the driver.
AMDTResult AMDTPwrGetTimerSamplingPeriod(AMDTUInt32* pIntervalMilliSec)
{
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pIntervalMilliSec)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        *pIntervalMilliSec = g_samplingPeriod;
    }

    return ret;
}

//AMDTPwrIsCounterEnabled: This API is query API to check whether a counter is enabled
AMDTResult AMDTPwrIsCounterEnabled(AMDTUInt32 counterID)
{
    AMDTResult ret = AMDT_ERROR_COUNTER_NOT_ENABLED;

    if (counterID >= nbrSupportedCounters)
    {
        ret = AMDT_ERROR_INVALID_COUNTERID;
        PwrTrace(" Invalid counter id");
    }
    else
    {
        PwrCounters::iterator Iter = g_activeCounters.find(counterID);

        if (Iter != g_activeCounters.end())
        {
            ret = AMDT_STATUS_OK;
        }

        if (AMDT_STATUS_OK != ret)
        {
            PwrTrace("Counter id not found");
        }
    }

    return ret;
}

// AMDTPwrGetNumEnabledCounters: This API is query API to check number of counters
// which are enabled
AMDTResult AMDTPwrGetNumEnabledCounters(AMDTUInt32* pCount)
{
    AMDTPwrProfileState state = AMDT_PWR_PROFILE_STATE_UNINITIALIZED;
    AMDTResult ret = AMDT_STATUS_OK;

    if (nullptr == pCount)
    {
        ret = AMDT_ERROR_INVALIDARG;
    }

    if (AMDT_STATUS_OK == ret)
    {
        ret = AMDTPwrGetProfilingState(&state);
    }

    if (AMDT_STATUS_OK == ret)
    {
        if (AMDT_PWR_PROFILE_STATE_UNINITIALIZED == state)
        {
            ret = AMDT_ERROR_DRIVER_UNINITIALIZED;
        }

        // For the STUB, only 3 ( 2 normal + 1 histogram) counters are enabled.
        *pCount = g_activeCounters.size();
    }

    return ret;
}

AMDTResult AMDTPwrDisableCounter(AMDTUInt32 counterId)
{
    // (void)counterId;
    AMDTResult ret = AMDT_ERROR_COUNTER_NOT_ENABLED;

    PwrCounters::iterator Iter = g_activeCounters.find(counterId);

    if (Iter != g_activeCounters.end())
    {
        g_activeCounters.erase(Iter);
    }

    return ret;
}

void CollectData()
{
    // Collect Data every 100 ms
    AMDTUInt32 samplingPeriod;

    AMDTPwrGetTimerSamplingPeriod(&samplingPeriod);

    while (!g_threadData.m_stopped)
    {
        GetCounters(&g_threadData);
#ifdef LINUX
        usleep(samplingPeriod * 1000);
#else
        Sleep(samplingPeriod);
#endif
    }
}

AMDTResult AMDTPwrGetApuPstateInfo(AMDTPwrApuPstateList* pList)
{
    AMDTResult ret = AMDT_STATUS_OK;
    pList = nullptr;
    return ret;
}

AMDTResult AMDTPwrGetCounterHierarchy(AMDTUInt32 id, AMDTPwrCounterHierarchy* pInfo)
{
    id = 0;
    (void*)pInfo;
    return AMDT_STATUS_OK;
}


AMDTResult GetNumOfBins(AMDTUInt32 counterId, AMDTUInt32* n)
{
    (void)counterId;
    *n = 1;

    return AMDT_STATUS_OK;
}

AMDTResult AMDTPwrGetNodeTemperature(AMDTFloat32* pNodeTemp)
{
    *pNodeTemp = 1;
    return AMDT_STATUS_OK;
}

AMDTResult AMDTEnableProcessProfiling(void)
{
    return AMDT_STATUS_OK;
}

AMDTResult AMDTGetProcessProfileData(AMDTUInt32* pPIDCount,
                                     AMDTPwrProcessInfo** ppData,
                                     AMDTUInt32 pidVal,
                                     bool reset)
{

    (void)pPIDCount;
    (void)ppData;
    (void)pidVal;
    (void)reset;
    return AMDT_STATUS_OK;
}

AMDTResult AMDTPwrGetModuleProfileData(AMDTPwrModuleData** ppData, AMDTUInt32* pModuleCount, AMDTFloat32* pPower)
{
    (void)ppData;
    (void)pModuleCount;
    (void)pPower;
    return AMDT_ERROR_NOTSUPPORTED;
}

