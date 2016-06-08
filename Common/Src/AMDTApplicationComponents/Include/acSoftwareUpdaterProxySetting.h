//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSoftwareUpdaterProxySetting.h
///
//==================================================================================

#ifndef ACSOFTWAREUPDATERPROXYSETTING_H
#define ACSOFTWAREUPDATERPROXYSETTING_H

// Qt:
#include <QThread>


class acSoftwareUpdaterProxyThread;

class acSoftwareUpdaterProxySetting : public QObject
{
    Q_OBJECT

public:

    friend class acSoftwareUpdaterProxyThread;
    friend class acSingeltonsDelete;

    // Single Instance:
    static acSoftwareUpdaterProxySetting& Instance();

    // Destructor
    virtual ~acSoftwareUpdaterProxySetting();

    /// On complete of proxy setting check
    void Finish();

    /// Will indicate current status of proxy checking.
    /// \return True if proxy check is going on, else false.
    bool IsChecking();

private:

    // Private constructor (single Instance class):
    acSoftwareUpdaterProxySetting();

    void IsProxyAvailable();

    static acSoftwareUpdaterProxySetting* m_spMySingleInstance;

    bool m_checkingNetworkProxy; /// Indicates current status of network proxy checking process.

    static acSoftwareUpdaterProxyThread* m_proxyCheckingThread;

signals:
    /// Emitted once Proxy checking is completed.
    void Finished();
};

class acSoftwareUpdaterProxyThread : public QThread
{
    Q_OBJECT

public:
    acSoftwareUpdaterProxyThread(acSoftwareUpdaterProxySetting* networkProxyObject): m_CheckNetworkProxy(networkProxyObject)
    {
        setTerminationEnabled(true);
    }

    virtual ~acSoftwareUpdaterProxyThread() {};

    virtual void run()
    {
        if (m_CheckNetworkProxy != nullptr)
        {
            m_CheckNetworkProxy->IsProxyAvailable();
        }

        exec();
    }
private:
    acSoftwareUpdaterProxySetting* m_CheckNetworkProxy;
};

#define theNetworkProxyChecker acSoftwareUpdaterProxySetting::Instance()

#endif //ACSOFTWAREUPDATERPROXYSETTING_H
