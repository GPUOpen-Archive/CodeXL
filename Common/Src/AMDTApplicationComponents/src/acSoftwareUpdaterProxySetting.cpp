//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSoftwareUpdaterProxySetting.cpp
///
//==================================================================================

// Qt:
#include <QtNetwork/QNetworkProxy>

// Local:
#include <AMDTApplicationComponents/Include/acSoftwareUpdaterProxySetting.h>

acSoftwareUpdaterProxySetting* acSoftwareUpdaterProxySetting::m_spMySingleInstance = nullptr;
acSoftwareUpdaterProxyThread* acSoftwareUpdaterProxySetting::m_proxyCheckingThread = nullptr;

void acSoftwareUpdaterProxySetting::IsProxyAvailable()
{
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery();

    if (listOfProxies.count() != 0)
    {
        // Required only if your network is behind proxy.
        QNetworkProxyFactory::setUseSystemConfiguration(true) ;
    }

    Finish();
}

acSoftwareUpdaterProxySetting::acSoftwareUpdaterProxySetting() :
    m_checkingNetworkProxy(true)
{
}

acSoftwareUpdaterProxySetting& acSoftwareUpdaterProxySetting::Instance()
{
    if (m_spMySingleInstance == nullptr)
    {
        m_spMySingleInstance = new(std::nothrow) acSoftwareUpdaterProxySetting;

        m_proxyCheckingThread = new(std::nothrow) acSoftwareUpdaterProxyThread(m_spMySingleInstance);

        if (m_proxyCheckingThread != nullptr)
        {
            m_proxyCheckingThread->start();
        }
    }

    return *m_spMySingleInstance;
}

acSoftwareUpdaterProxySetting::~acSoftwareUpdaterProxySetting()
{
    m_proxyCheckingThread->terminate();
    m_proxyCheckingThread->wait();
    delete m_proxyCheckingThread;
    m_proxyCheckingThread = nullptr;
}

void acSoftwareUpdaterProxySetting::Finish()
{
    m_checkingNetworkProxy = false;
    Finished();
}

bool acSoftwareUpdaterProxySetting::IsChecking()
{
    return m_checkingNetworkProxy;
}
