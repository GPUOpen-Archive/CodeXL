
#ifndef __AFOPENCLDEVICEINFORMATIONCOLLECTOR_H
#define __AFOPENCLDEVICEINFORMATIONCOLLECTOR_H

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTimer.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>

/// \class afOpenCLDeviceInformationCollector is a thread used to run afSystemInformationCommand's CollectAllOpenCLDevicesInformation which might take a while
class afOpenCLDeviceInformationCollector : protected osThread
{

public:
    afOpenCLDeviceInformationCollector();
    virtual ~afOpenCLDeviceInformationCollector();

    bool StartCollectingInfo();
    bool& IsActive();
    void StopCollectingInfo();
    bool GetOpenCLDeviceInformation(gtList< gtList <gtString> >& openCLDevicesInfoData);

protected:
    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination() {};

private:
    bool m_isActive;
    bool m_dataCollectedOk;
    gtList< gtList <gtString> > m_openCLDevicesInfoData;
};

#endif

