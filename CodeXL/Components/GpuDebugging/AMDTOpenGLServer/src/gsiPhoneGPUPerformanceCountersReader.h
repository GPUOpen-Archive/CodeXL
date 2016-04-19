//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsiPhoneGPUPerformanceCountersReader.h
///
//==================================================================================

//------------------------------ gsiPhoneGPUPerformanceCountersReader.h ------------------------------

#ifndef __GSIPHONEGPUPERFORMANCECOUNTERSREADER_H
#define __GSIPHONEGPUPERFORMANCECOUNTERSREADER_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTAPIClasses/Include/apCounterInfo.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsiPhoneGPUPerformanceCountersReader
// General Description:
//   A performance counters reader that reads the iPhone on-devices GPU and
//   graphic driver performance counters
// Author:               Yaki Tebeka
// Creation Date:        12/1/2010
// ----------------------------------------------------------------------------------
class gsiPhoneGPUPerformanceCountersReader
{
public:
    gsiPhoneGPUPerformanceCountersReader();
    ~gsiPhoneGPUPerformanceCountersReader();

    bool addSupportediPhonePerformanceCounter(int counterIndex, const gtString& counterName);
    bool initialize();
    bool terminate();

    int amountOfCounters() const;
    bool updateCountersValues();
    const double* getCounterValues() const {return _pCounterValues;};

    void onDictionaryPerformanceCounter(const gtString& counterName);
    bool setCounterValue(const gtString& counterName, double counterValue);
    bool isFirstUpdateCounterValuesCall() const {return _isFirstUpdateCounterValuesCall;};

private:
    bool logGraphicAccelerator();
    bool logGPUPerformanceCounters();
    bool updatePerformanceStatisticsDictionary();
    void* getAcceleratorPerformanceStatisticsDictionary(void* acceleratorHandle);

private:
    // Contains true iff this class was initialized:
    bool _isInitialized;

    // The graphic accelerator name:
    gtString _acceleratorName;

    // We only want to print unknown counters once:
    bool _isFirstUpdateCounterValuesCall;

    // The amount of performance counters exposed by the graphic accelerator:
    int _countersAmount;

    // An array of the counters' values:
    double* _pCounterValues;

    // Map for counter names and indices:
    gtMap<gtString, unsigned int> _counterNameToIndexMap;

    // The graphic accelerators handle (cast into a void*):
    void* _acceleratorHandle;

    // A copy of the graphic accelerator's performance counters dictionary (cast into a void*):
    void* _rPerformanceCountersDictionary;
};


#endif  // __GSIPHONEGPUPERFORMANCECOUNTERSREADER_H


