//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsiPhoneGPUPerformanceCountersReader.cpp
///
//==================================================================================

//------------------------------ gsiPhoneGPUPerformanceCountersReader.cpp ------------------------------

// Mac OS:
#include <pthread.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFNumber.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osIOKitForiPhoneDevice.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apGPUInfo.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsiPhoneGPUPerformanceCountersReader.h>


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::gsiPhoneGPUPerformanceCountersReader
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
gsiPhoneGPUPerformanceCountersReader::gsiPhoneGPUPerformanceCountersReader()
    : _isInitialized(false), _isFirstUpdateCounterValuesCall(true), _countersAmount(0), _pCounterValues(NULL), _acceleratorHandle(NULL), _rPerformanceCountersDictionary(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::~gsiPhoneGPUPerformanceCountersReader
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
gsiPhoneGPUPerformanceCountersReader::~gsiPhoneGPUPerformanceCountersReader()
{
    // Delete counter values snapshot vector
    if (_pCounterValues != NULL)
    {
        delete[] _pCounterValues;
        _pCounterValues = NULL;
    }

    if (_isInitialized)
    {
        terminate();
    }
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::addSupportediPhonePerformanceCounter
// Description: Adds support for a given iPhone performance counter.
// Arguments:   counterIndex - The counter's index in pValuesArray of gaGetiPhonePerformanceCountersValues.
//              counterName - The counter's name.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::addSupportediPhonePerformanceCounter(int counterIndex, const gtString& counterName)
{
    // Increase the counters amount:
    _countersAmount++;

    // Insert the counter name to the map:
    _counterNameToIndexMap[counterName] = counterIndex;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::initialize
// Description: Initialize the Mac OS GPU performance counters support.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::initialize()
{
    bool retVal = false;

    // If this class was not initialized yet:
    if (!_isInitialized)
    {
        // Log the graphic accelerator:
        bool rc1 = logGraphicAccelerator();
        GT_IF_WITH_ASSERT(rc1)
        {
            // Log the available performance counters:
            bool rc2 = logGPUPerformanceCounters();
            GT_ASSERT(rc2);

            // Allocate counter values pointer:
            _pCounterValues = new double[_countersAmount];

            ::memset(_pCounterValues, 0, _countersAmount * sizeof(double));
            retVal = true;

            // Mark that we were initialized successfully:
            _isInitialized = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::terminate
// Description: Terminate the Mac OS GPU performance counters support.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::terminate()
{
    bool retVal = true;

    // If this class was initialized and not terminated yet:
    if (_isInitialized)
    {
        // Mark that this class was terminated:
        _isInitialized = false;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::amountOfCounters
// Description: Returns the amount of supported performance counters.
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
int gsiPhoneGPUPerformanceCountersReader::amountOfCounters() const
{
    return _isInitialized ? _countersAmount : 0;
}


// ---------------------------------------------------------------------------
// Name:        pcGetPerformanceCounterValueCallbackFunc
// Description: Is called on each performance counter exported by all graphic
//              accelerators.
// Arguments: key - The performance counter name.
//            value - The performance counter value.
//            context - Pointer to the gsiPhoneGPUPerformanceCountersReader class instance casted into (void*)
// Author:      Yaki Tebeka
// Date:        26/2/2009
// ---------------------------------------------------------------------------
void pcGetPerformanceCounterValueCallbackFunc(const void* key, const void* value, void* context)
{
    // Get the performance counter name:
    CFTypeRef* pCounterName = (CFTypeRef*)key;
    GT_IF_WITH_ASSERT(pCounterName != NULL)
    {
        // Verify that it is a string:
        GT_IF_WITH_ASSERT(CFGetTypeID(pCounterName) == CFStringGetTypeID())
        {
            // Down cast the counter name into a string:
            CFStringRef rCounterNameStr = (CFStringRef)pCounterName;
            const char* pCounterNameStr = CFStringGetCStringPtr(rCounterNameStr, kCFStringEncodingMacRoman);
            GT_IF_WITH_ASSERT(pCounterNameStr != NULL)
            {
                // Get the performance counter value:
                CFTypeRef* pCounterValue = (CFTypeRef*)value;
                GT_IF_WITH_ASSERT(pCounterValue != NULL)
                {
                    // Verify that it is a number:
                    GT_IF_WITH_ASSERT(CFGetTypeID(pCounterValue) == CFNumberGetTypeID())
                    {
                        // Down cast counter value to a number:
                        CFNumberRef rCounterValueAsNumber = (CFNumberRef)pCounterValue;

                        // Cast the number into a double:
                        double counterValue = 0;
                        Boolean rc1 = CFNumberGetValue(rCounterValueAsNumber, kCFNumberDoubleType, &counterValue);
                        GT_IF_WITH_ASSERT(rc1)
                        {
                            // Get the performance counters reader:
                            gsiPhoneGPUPerformanceCountersReader* pCountersReader = (gsiPhoneGPUPerformanceCountersReader*)context;
                            GT_IF_WITH_ASSERT(pCountersReader != NULL)
                            {
                                // Add the counter to the supported counters list:
                                gtString counterName(pCounterNameStr);
                                bool rc = pCountersReader->setCounterValue(counterName, counterValue);

                                if (pCountersReader->isFirstUpdateCounterValuesCall() && (!rc))
                                {
                                    // Log the performance counters' values set failure:
                                    gtString debugMessage;
                                    debugMessage.appendFormattedString(GS_STR_FAILED_TO_UPDATE_PERF_COUNTER, counterName.asCharArray());
                                    OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::updateCountersValues
// Description: Updates the counter values from the Mac OS kernel.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::updateCountersValues()
{
    bool retVal = true;

    // Get an updated version of the SGX Driver performance counters dictionary:
    bool gotPerfDictionary = updatePerformanceStatisticsDictionary();

    // Verify that we have a performance counters dictionary:
    GT_IF_WITH_ASSERT(gotPerfDictionary && (_rPerformanceCountersDictionary != NULL))
    {
        // Iterate the dictionary values ( = exported performance counters) and add them to the _supportedOSCounters vector:
        CFDictionaryApplyFunction((CFDictionaryRef)_rPerformanceCountersDictionary, &pcGetPerformanceCounterValueCallbackFunc, (void*)this);
    }

    return retVal;
}


/*
// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::getLocalMachineGPUInfo
// Description: Retrieves the information of a local machine GPU.
// Arguments: GPUIndex - The queried GPU index.
//            GPUInfo - Will get the queried GPU information.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/3/2009
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::getLocalMachineGPUInfo(int GPUIndex, apGPUInfo& GPUInfo) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= GPUIndex) && (GPUIndex < (int)(_pAcceleratorInfo.size())))
    {
        // Get the queried accelerator info:
        const pcMacAcceleratorInfo* pAcceleratorInfo = _pAcceleratorInfo[GPUIndex];
        GT_IF_WITH_ASSERT(pAcceleratorInfo != NULL)
        {
            // Translate the accelerator information into a GPU information:
            GPUInfo._name = pAcceleratorInfo->_shortName;
            GPUInfo._driverName = pAcceleratorInfo->_longName;

            retVal = true;
        }
    }

    return retVal;
}
*/


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::logGraphicAccelerator
// Description: Logs the information of the local machine's graphic accelerators.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::logGraphicAccelerator()
{
    bool retVal = false;

    // Get the Mach kernel port used to initiate communication with the I/O Kit:
    mach_port_t IOKitMasterPort;
    kern_return_t rc1 = IOMasterPort(MACH_PORT_NULL, &IOKitMasterPort);
    GT_IF_WITH_ASSERT(rc1 == kIOReturnSuccess)
    {
        // Create a "matching dictionary" that will "match" graphic accelerators ("SGXDriver" derived objects):
        CFMutableDictionaryRef rGraphicAcceleratorMatchingDictionary = IOServiceMatching("SGXDriver");
        GT_IF_WITH_ASSERT(rGraphicAcceleratorMatchingDictionary != NULL)
        {
            // Create an iterator over all IOService derived objects matching our IOAccelerator dictionary:
            io_iterator_t servicesIter;
            kern_return_t rc2 = IOServiceGetMatchingServices(IOKitMasterPort, rGraphicAcceleratorMatchingDictionary, &servicesIter);
            GT_IF_WITH_ASSERT(rc2 == KERN_SUCCESS)
            {
                // Iterate the matching objects (graphic accelerators):
                int acceleratorIndex = 0;
                io_object_t acceleratorHandle;

                while ((acceleratorHandle = IOIteratorNext(servicesIter)))
                {
                    // Get the accelerator's performance statistics dictionary:
                    void* pAcceleratorPerformanceStatisticsDic = getAcceleratorPerformanceStatisticsDictionary((void*)acceleratorHandle);
                    GT_IF_WITH_ASSERT(pAcceleratorPerformanceStatisticsDic != NULL)
                    {
                        // Log the accelerator handle and it's performance counters dictionary:
                        _acceleratorHandle = (void*)acceleratorHandle;
                        _rPerformanceCountersDictionary = pAcceleratorPerformanceStatisticsDic;

                        // We found a graphic accelerator that supports performance counters:
                        retVal = true;
                        break;
                    }

                    /*
                        // Get the graphic accelerators long name:
                        bool rc3 = getAcceleratorName((void*)acceleratorHandle, pCurrAcceleratorInfo->_longName);
                        GT_ASSERT(rc3);

                        // If we failed to get the accelerator long name, use its short name:
                        if (!rc3)
                        {
                            pCurrAcceleratorInfo->_longName = pCurrAcceleratorInfo->_shortName;
                        }
                    }
                    */

                    acceleratorIndex++;
                }

                // Release the services iterator:
                IOObjectRelease(servicesIter);
            }
        }

        // IOServiceGetMatchingServices consumes a reference to the dictionary, so we don't need to release the dictionary ref.
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pcLogPerformanceCounterCallbackFunc
// Description: Is called on each performance counter exported by all graphic
//              accelerators.
// Arguments: key - The performance counter name.
//            value - The performance counter value.
//            context - Pointer to the gsiPhoneGPUPerformanceCountersReader class instance casted into (void*)
// Author:      Yaki Tebeka
// Date:        26/2/2009
// ---------------------------------------------------------------------------
void pcLogPerformanceCounterCallbackFunc(const void* key, const void* value, void* context)
{
    // Get the performance counter name:
    CFTypeRef* pCounterName = (CFTypeRef*)key;
    GT_IF_WITH_ASSERT(pCounterName != NULL)
    {
        // Verify that it is a string:
        GT_IF_WITH_ASSERT(CFGetTypeID(pCounterName) == CFStringGetTypeID())
        {
            // Down cast the counter name into a string:
            CFStringRef rCounterNameStr = (CFStringRef)pCounterName;
            const char* pCounterNameStr = CFStringGetCStringPtr(rCounterNameStr, kCFStringEncodingMacRoman);
            GT_IF_WITH_ASSERT(pCounterNameStr != NULL)
            {
                // Add the performance counter to the reader:
                gsiPhoneGPUPerformanceCountersReader* pCountersReader = (gsiPhoneGPUPerformanceCountersReader*)context;
                GT_IF_WITH_ASSERT(pCountersReader != NULL)
                {
                    // Add the counter to the supported counters list:
                    gtString counterName(pCounterNameStr);
                    pCountersReader->onDictionaryPerformanceCounter(counterName);
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::logGPUPerformanceCounters
// Description: Logs the available GPU performance counters.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::logGPUPerformanceCounters()
{
    bool retVal = false;

    // Verify that we have a performance counters dictionary:
    GT_IF_WITH_ASSERT(_rPerformanceCountersDictionary != NULL)
    {
        // Iterate the dictionary values ( = exported performance counters) and add them to the _supportedOSCounters vector:
        CFDictionaryApplyFunction((CFDictionaryRef)_rPerformanceCountersDictionary, &pcLogPerformanceCounterCallbackFunc, (void*)this);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::updatePerformanceStatisticsDictionary
// Description: Updated our copy of the SGX Driver performance counters dictionary.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2010
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::updatePerformanceStatisticsDictionary()
{
    bool retVal = false;

    // Release the accelerator's old performance statistics dictionary:
    if (_rPerformanceCountersDictionary != NULL)
    {
        CFRelease((CFDictionaryRef)(_rPerformanceCountersDictionary));
        _rPerformanceCountersDictionary = NULL;
    }

    // Get an updated performance statistics dictionary:
    void* pUpdatedPerformanceStatisticsDic = getAcceleratorPerformanceStatisticsDictionary(_acceleratorHandle);
    GT_IF_WITH_ASSERT(pUpdatedPerformanceStatisticsDic != NULL)
    {
        _rPerformanceCountersDictionary = pUpdatedPerformanceStatisticsDic;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::getAcceleratorPerformanceStatisticsDictionary
// Description: Retrieves a dictionary, containing the queried accelerator's performance counters.
// Arguments: acceleratorHandle - The queried accelerator handle (cast into void*)
// Return Val: void* - Will get the performance counters dictionary (cast into void*), or NULL in
//                     case of failure.
// Author:      Yaki Tebeka
// Date:        2/3/2009
// ---------------------------------------------------------------------------
void* gsiPhoneGPUPerformanceCountersReader::getAcceleratorPerformanceStatisticsDictionary(void* acceleratorHandle)
{
    void* retVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(acceleratorHandle != NULL)
    {
        // Get the accelerator's performance statistics property:
        CFTypeRef rPerformanceStatisticsProp;
        rPerformanceStatisticsProp = IORegistryEntryCreateCFProperty((io_object_t)acceleratorHandle, CFSTR("PerformanceStatistics"), kCFAllocatorDefault, 0);
        GT_IF_WITH_ASSERT(rPerformanceStatisticsProp != NULL)
        {
            // Verify that this property is a dictionary:
            GT_IF_WITH_ASSERT(CFGetTypeID(rPerformanceStatisticsProp) == CFDictionaryGetTypeID())
            {
                // Return the dictionary cast into void*:
                CFDictionaryRef rPerformanceStatisticsDic = (CFDictionaryRef)rPerformanceStatisticsProp;
                retVal = (void*)rPerformanceStatisticsDic;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::onDictionaryPerformanceCounter
// Description: Is called for every performance counters residing in the
//              performance counters dictionary.
// Arguments: counterName - The performance counter name
// Author:      Sigal Algranaty
// Date:        13/1/2010
// ---------------------------------------------------------------------------
void gsiPhoneGPUPerformanceCountersReader::onDictionaryPerformanceCounter(const gtString& counterName)
{
    // Get the counter's hard coded index (place within the counter values vector, set by the CodeXL client -
    // see addSupportediPhonePerformanceCounter)
    gtMap<gtString, unsigned int>::iterator findIter = _counterNameToIndexMap.find(counterName);

    if (findIter == _counterNameToIndexMap.end())
    {
        // Ooopss - we found a counter, supported by the iPhone driver, but not by the CodeXL client!
        gtString errMgs = GS_STR_UNKNOWN_COUNTER_PART1;
        errMgs += counterName;
        errMgs += GS_STR_UNKNOWN_COUNTER_PART2;
        OS_OUTPUT_DEBUG_LOG(errMgs.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

}


// ---------------------------------------------------------------------------
// Name:        gsiPhoneGPUPerformanceCountersReader::setCounterValue
// Description: Set a performance counters value
// Arguments: counterName - The counter's name.
//            counterValue - The counter's value.
// Author:      Sigal Algranaty
// Date:        13/1/2010
// ---------------------------------------------------------------------------
bool gsiPhoneGPUPerformanceCountersReader::setCounterValue(const gtString& counterName, double counterValue)
{
    bool retVal = false;

    // Get the counter's index (place within the counter values vector):
    gtMap<gtString, unsigned int>::iterator findIter = _counterNameToIndexMap.find(counterName);

    if (findIter != _counterNameToIndexMap.end())
    {
        // Get the counter index:
        unsigned int counterIndex = (*findIter).second;
        GT_IF_WITH_ASSERT((int)counterIndex < _countersAmount)
        {
            GT_IF_WITH_ASSERT(_pCounterValues != NULL)
            {
                // Set the counter value:
                _pCounterValues[counterIndex] = counterValue;
                retVal = true;
            }
        }
    }
    else // findIter == _counterNameToIndexMap.end()
    {
        GT_ASSERT((findIter != _counterNameToIndexMap.end()) || (!_isFirstUpdateCounterValuesCall));
    }

    return retVal;
}


