//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvTranslator.h
/// \brief Definition of the OsvTranslator
///
//==================================================================================

#ifndef _OSVTRANSLATOR_H_
#define _OSVTRANSLATOR_H_

#include <QString>
#include <QStringList>

#include "CpuProfilingTranslationDLLBuild.h"
#include <AMDTCpuProfilingRawData/inc/OsvData.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileDataTranslationInfo.h>

#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


//////////////////////////////////////////////////////
class CP_TRANS_API OsvThreadInfo
{
public:
    OsvThreadInfo(ThreadIdType t = 0, double s = 0.0) : tid(t), sec(s) {}

    ThreadIdType tid;
    double sec;
};

typedef gtList<OsvThreadInfo> OsvThreadInfoList;


//////////////////////////////////////////////////////
class CP_TRANS_API OsvTargetProcess
{
public:
    OsvTargetProcess(QString n = "", ProcessIdType i = 0) : procName(n), pid(i) {}

    QString procName;
    QString args;
    ProcessIdType pid;
    OsvThreadInfoList threadList;
};

typedef gtList<OsvTargetProcess> OsvTargetProcList;


//////////////////////////////////////////////////////
class CP_TRANS_API OsvTranslator
{
public:
    OsvTranslator();
    ~OsvTranslator();

    void clear();
    void setTargetProcessList(OsvTargetProcList list) { m_targetProcList = list; }
    void addSessionReportEntry(QString str) { m_transReportEntryList.push_back(str); }
    HRESULT generateOsvFile(QString sessionDir, QString sessionName, bool bCLUtil = false);

protected:
    OsvDataItem* generateOsvAppInfoSection(QString profFile);
    OsvDataItem* generateOsvSystemWideProfileSection(QString profFile);
    OsvDataItem* generateOsvUtilSection(QString cutPath, bool bCLU = false);
    OsvDataItem* generateOsvOclSection(QString ocltDir, bool bCut = false);
    OsvDataItem* generateSessionReport();

private:
    double GetOcltDuration(const QString& ocltDir) const;

    gtString m_path;
    FILE* m_pFile;
    OsvTargetProcList m_targetProcList;
    gtList<QString> m_transReportEntryList;

};

#endif // _OSVTRANSLATOR_H_
