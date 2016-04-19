#include <AMDTAccessPmcData.h>
#include <AMDTHelpers.h>

uint16 EventId[] = {0x76, 0xC0};

#define LEGACY_COREPERFCONTROL_BASE 0xC0010000U
#define LEGACY_COREPERFCONTROL_OFFSET 4U
#define LEGACY_COREPMC_STRIDE 1U
#define EXT_COREPERFCONTROL_BASE 0xC0010200U
#define EXT_COREPERFCONTROL_OFFSET 1U
#define EXT_COREPMC_STRIDE 2U
// Attempt to get the HAL Arbitration for the counter availability
static bool g_halAcquired = false;

// ResetPMCCounters: Reset PCM counter values
bool ResetPMCCounters(PmcCounters* pPmc)
{
    bool retVal = true;
    uint32 index = 0;

    for (index = 0; index < PMC_EVENT_MAX_CNT; index++)
    {
        // Initialize the counter value to 0;
        pPmc[index].m_data.isReadAccess = false;
        pPmc[index].m_data.data = 0ULL;
        HelpAccessMSRAddress(&pPmc[index].m_data);
    }

    return retVal;
}

// EncodePMCEvent: Encode PMC event as per bit field
uint32 EncodePMCEvent(uint16 eventSelect, uint8 unitMask)
{
    typedef union
    {
        struct
        {
            uint8 ucEventSelect : 8;
            uint8 ucUnitMask : 8;
            uint8 bitUsrEvents : 1;
            uint8 bitOsEvents : 1;
            uint8 bitEdgeEvents : 1;
            uint8 bitPinControl : 1;
            uint8 bitSampleEvents : 1;
            uint8 bitReserved : 1;
            uint8 bitEnabled : 1;
            uint8 bitInvert : 1;
            uint8 ucCounterMask : 8;
            uint8 ucEventSelectHigh : 4;
            uint8 Reserved1 : 4;
            uint8 guestOnly : 1;
            uint8 hostOnly : 1;
            uint32 Reserved : 22;
        };
        uint32 perf_ctl;
    } PERF_CTL;

    PERF_CTL perfControl;
    perfControl.perf_ctl = 0;

    perfControl.ucEventSelect = eventSelect & 0xFFU;
    perfControl.ucEventSelectHigh = (eventSelect >> 8) & 0xFU;
    perfControl.ucUnitMask = (uint8)unitMask;
    perfControl.bitOsEvents = 1U;
    perfControl.bitUsrEvents = 1U;
    perfControl.bitSampleEvents = 0U; // Count Mode
    perfControl.bitEnabled = 1U; // Enable the Performance event counter

    return perfControl.perf_ctl;
}

// InitializePMCCounters
bool InitializePMCCounters(PmcCounters* pPmc)
{
    bool retVal = false;
    uint32 msrBaseAddress = 0;
    uint32 counterOffset = 0;
    uint32 stride = 0;
    uint32 index = 0;
    bool isPmcAvailable = HelpIsPMCCounterAvailable();

    if (true == isPmcAvailable)
    {
        if (!g_halAcquired)
        {
            g_halAcquired = AcquirePCMCountersLock();

            if (!g_halAcquired)
            {
                DRVPRINT("Power HAL Arbitration failed!");
            }
        }

        if (g_halAcquired)
        {
            msrBaseAddress = isPmcAvailable ? EXT_COREPERFCONTROL_BASE : LEGACY_COREPERFCONTROL_BASE;
            counterOffset = isPmcAvailable ? EXT_COREPERFCONTROL_OFFSET : LEGACY_COREPERFCONTROL_OFFSET;
            stride = isPmcAvailable ? EXT_COREPMC_STRIDE : LEGACY_COREPMC_STRIDE;
            index = 0;

            for (index = 0; index < PMC_EVENT_MAX_CNT; index++)
            {
                // initialize the MSR for CPU Cycles Unhalted
                // Initialize the MSR for Retired Instructions
                pPmc[index].m_control.regId = msrBaseAddress + (index * stride);
                pPmc[index].m_data.regId = msrBaseAddress + (index * stride) + counterOffset;

                pPmc[index].m_control.data = EncodePMCEvent(EventId[index], 0);
                pPmc[index].m_control.isReadAccess = false;
                HelpAccessMSRAddress(&pPmc[index].m_control);
            }

            // Reset PMC Counter values
            ResetPMCCounters(pPmc);
        }

        retVal = true;
    }

    return retVal;
}

// ReadPmcCounterData: Read PMC counter values
uint32 ReadPmcCounterData(PmcCounters* pPmc, uint32* pData)
{
    uint32 index = 0;
    uint32 retSize = 0;

    if (g_halAcquired)
    {
        for (index = 0; index < PMC_EVENT_MAX_CNT; index++)
        {
            pPmc[index].m_data.isReadAccess = true;
            HelpAccessMSRAddress(&pPmc[index].m_data);
            pData[index] = pPmc[index].m_data.data;
        }
    }
    else
    {
        DRVPRINT("PMC counters are not accessible");

        for (index = 0; index < PMC_EVENT_MAX_CNT; index++)
        {
            pData[index] = 1;
        }
    }

    retSize = PMC_EVENT_MAX_CNT * sizeof(uint32);

    return retSize;
}

// ResetPMCControl: Reset PMC counter control data
bool ResetPMCControl(PmcCounters* pPmc)
{
    bool retVal = true;
    uint32 index = 0;

    if (g_halAcquired)
    {
        for (index = 0; index < PMC_EVENT_MAX_CNT; index++)
        {
            // Restet the control value to 0;
            pPmc[index].m_control.isReadAccess = false;
            pPmc[index].m_control.data = 0ULL;
            HelpAccessMSRAddress(&pPmc[index].m_control);
        }

        // Free the HAL Arbitration
        ReleasePCMCountersLock();
        g_halAcquired = false;
    }

    return retVal;
}

