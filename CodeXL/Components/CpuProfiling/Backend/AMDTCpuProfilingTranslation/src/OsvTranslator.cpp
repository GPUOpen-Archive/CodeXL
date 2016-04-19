//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvTranslator.cpp
/// \brief Implementation of the OsvTranslator class.
///
//==================================================================================

#include <QtCore>
#include <QFile>
#include <QDir>

#include <OsvTranslator.h>
#include <AMDTCpuProfilingRawData/inc/OsvWriter.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>

#if SUPPORT_OCL
    #include "OcltTranslator.h"
    #include "AsvReader.h"
    #include "CtxReader.h"
    #include "TrcReader.h"
#endif

#if SUPPORT_CPUUTIL
    #include "CaCURdr.h"
#endif

#include <AMDTBaseTools/Include/gtAssert.h>

OsvTranslator::OsvTranslator()
{
    clear();
}


OsvTranslator::~OsvTranslator()
{
}


void OsvTranslator::clear()
{
    m_pFile = NULL;
    m_path = L"";
}


HRESULT OsvTranslator::generateOsvFile(
    QString sessionDir,
    QString sessionName,
    bool bCLUtil)
{
    (void)(bCLUtil); // unused
    /* Here we need information from
     * 1. TBP/EBP file
     * 2. CUT file
     * 3. ASV file
     * 4. CTX file
     */
    QDir dir(sessionDir);
    QStringList fileList = dir.entryList(QDir::Files);
    QStringList::iterator fit;
    QStringList::iterator fend = fileList.end();

    // Looking for EBP/TBP files
    bool bFoundEbp = false;

    for (fit = fileList.begin(); fit != fend; fit++)
    {
        if (fit->endsWith(".ebp") || fit->endsWith(".tbp"))
        {
            bFoundEbp = true;
            break;
        }
    }

    OsvWriter osvWriter;

    if (bFoundEbp)
    {
        osvWriter.appendTopic(
            generateOsvAppInfoSection(sessionDir + "/" + *fit));

        if (m_targetProcList.size() == 0)
        {
            osvWriter.appendTopic(
                generateOsvSystemWideProfileSection(sessionDir + "/" + *fit));
        }
    }

#if SUPPORT_CPUUTIL

    // Utilization
    for (fit = fileList.begin(); fit != fend; fit++)
    {
        if (fit->endsWith(".cut"))
        {
            bCut = true;
            osvWriter.appendTopic(generateOsvUtilSection(sessionDir + "/" + *fit, bCLUtil));
            break;
        }
    }

    if (bCLUtil && !bCut)
    {
        // Put out cache line utilization if there was no cut file
        osvWriter.appendTopic(generateOsvUtilSection(sessionDir + "/ERROR", bCLUtil));
    }

#endif
#if SUPPORT_OCL
    // OpenCL stuff
    OsvDataItem* pTopic = new OsvDataItem("OpenCL Summary");

    if (!pTopic)
    {

        return E_FAIL;
    }

    QStringList dirList = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    QStringList::iterator dit = dirList.begin();
    QStringList::iterator dend = dirList.end();

    for (; dit != dend; dit++)
    {
        OsvDataItem* pOclPid = generateOsvOclSection(sessionDir + "/" + *dit, bCut);

        if (pOclPid)
        {
            pTopic->appendChild(pOclPid);
        }
    }

    if (pTopic->child)
    {
        osvWriter.appendTopic(pTopic);
    }
    else
    {
        delete pTopic;
    }

#endif

    // Generate session report stuff
    if (m_transReportEntryList.size() != 0)
    {
        osvWriter.appendTopic(generateSessionReport());
    }

    // Writing out the file
    QString osvFilePath = sessionDir + "/" + sessionName;

    if (osvFilePath.endsWith("ebp"))
    {
        osvFilePath += ".osv";
    }
    else
    {
        osvFilePath += ".ebp.osv";
    }

    osvWriter.write(osvFilePath.toStdWString().c_str());

    return S_OK;
}


// Assumes the TI file has already been read and the map is already generated
OsvDataItem* OsvTranslator::generateOsvAppInfoSection(QString profFile)
{
    OsvDataItem* pL1 = NULL;
    OsvDataItem* pL2 = NULL;
    OsvDataItem* pL3 = NULL;

    pL1 = new OsvDataItem("Target Application Info");

    if (!pL1)
    {

        return NULL;
    }

    QFileInfo profFileInfo(profFile);
    gtString path(profFile.toStdWString().c_str());
    QDir sessDir = QFileInfo(profFile).absoluteDir();

    CpuProfileReader profRdr;

    // Initialize EBP reader stuff
    if (!profRdr.open(path))
    {
        return NULL;
    }

    // Look for CSS files (Done once)
    QStringList cssFilter;
    cssFilter << "*.css";
    QStringList cssList = sessDir.entryList(cssFilter, QDir::NoDotAndDotDot | QDir::Files);

    // Show each target PID
    OsvTargetProcList::iterator pit = m_targetProcList.begin();
    OsvTargetProcList::iterator pend = m_targetProcList.end();

    for (; pit != pend; pit++)
    {
        // Fill in information for each target processes
        wchar_t procName[1024] = {L'\0'};

        if (0 == (*pit).pid)
        {
            continue;
        }


        ////////////////////////////////////////////////////////////////
        // Get number of modules for this process

        NameModuleMap* modMap = profRdr.getModuleMap();

        if (!modMap)
        {
            return NULL;
        }

        unsigned int numMod = 0;
        NameModuleMap::iterator mit = modMap->begin();
        NameModuleMap::iterator mend = modMap->end();

        for (; mit != mend; mit++)
        {
            if (mit->second.findSampleForPid((*pit).pid) != mit->second.getEndSample())
            {
                numMod++;
            }
        }

        if (0 == numMod)
        {
            continue;
        }

        ///////////////////////////////////////////////////////////////////
        // Create target process
        pL2 = new OsvDataItem(QString("PID : ") +
                              QString::number((*pit).pid),
                              ((*pit).procName.isEmpty()) ? QString("N/A") : (*pit).procName);

        if (!pL2)
        {

            break;
        }

        pL1->appendChild(pL2);

        ////////////////////////////////////////////////////////////////
        // Get threads for this process
        for (OsvThreadInfoList::iterator tit = (*pit).threadList.begin() ;
             tit != (*pit).threadList.end();
             tit++)
        {
            pL3 = new OsvDataItem(
                QString("TID : ") +
                QString::number((*tit).tid),
                (((*tit).sec == 0) ? QString("") : QString::number((*tit).sec) + " sec"));

            if (!pL3)
            {

                break;
            }

            pL2->appendChild(pL3);
        }

        ////////////////////////////////////////////////////////////////
        // Add number of modules for this process

        QString fileName = profFileInfo.fileName();
        pL3 = new OsvDataItem("Number of modules with profile data",
                              QString::number(numMod) +
                              " [View modules for this process]",
                              "proc:" + fileName +
                              "|" + QString::fromWCharArray(procName) +
                              "|" + QString::number((*pit).pid));

        if (!pL3)
        {

            return NULL;
        }

        pL2->appendChild(pL3);

        ////////////////////////////////////////////////////////////////
        // Add CSS for this process
        if (cssList.isEmpty())
        {
            continue;
        }

        // Search CSS file for this process
        QString cssFile = QString::number((*pit).pid) + ".css";

        if (-1 != cssList.indexOf(cssFile))
        {
            pL3 = new OsvDataItem("Call-chain Information is available",
                                  "[View call chain for this process]",
                                  QString("css:") + cssFile);

            if (!pL3)
            {

                return NULL;
            }

            pL2->appendChild(pL3);
        }
    } // for loop

    // If no PID information, return NULL
    if (!pL1->child)
    {
        delete pL1;
        pL1 = NULL;
    }

    return pL1;
}


OsvDataItem* OsvTranslator::generateOsvSystemWideProfileSection(QString profFile)
{
    QFileInfo profFileInfo(profFile);
    gtString path(profFile.toStdWString().c_str());
    QDir sessDir = QFileInfo(profFile).absoluteDir();
    QString fileName = profFileInfo.fileName();

    // Initialize EBP reader stuff
    CpuProfileReader profRdr;

    if (!profRdr.open(path))
    {
        return NULL;
    }

    CpuProfileInfo* profInfo = profRdr.getProfileInfo();

    if (!profInfo)
    {
        return NULL;
    }

    PidProcessMap* procMap = profRdr.getProcessMap();

    if (!procMap)
    {
        return NULL;
    }

    OsvDataItem* pItem = NULL;
    OsvDataItem* pTopic = new OsvDataItem("System-wide Profile");

    if (!pTopic)
    {

        return NULL;
    }

    pItem = pTopic->addChild("Number of Processes",
                             QString::number(procMap->size()) +
                             "  [View all processes]",
                             "proc:" + fileName);
    pItem = pItem->addSibling("Number of Modules",
                              QString::number(profInfo->m_numModules) +
                              "  [View all modules]",
                              "sys:" + fileName);

    return pTopic;
}


OsvDataItem* OsvTranslator::generateOsvUtilSection(QString cutPath, bool bCLU)
{
#if SUPPORT_CPUUTIL
    // Initialize CutReader
    CCaCURdr cutReader;
    bool bShowCUT = true;
    CpuUtilHeader cutHdr;
    float sysCpuData = 0;
    float sysMemData = 0;
    ProcessUtiMap pMapData;
    QVector<float> coreData;
    unsigned int cutCnt = 0;

    if (cutReader.OpenCpuUtiFile(cutPath.toStdWString().c_str()) != CCaCURdr::evCACURDR_OK)
    {
        if (!bCLU)
        {
            return NULL;
        }

        bShowCUT = false;
    }

    if (bShowCUT)
    {
        cutReader.GetCpuUtiHeader(&cutHdr);

        coreData.resize(cutHdr.num_cores);

        UtilRecords cRec;

        while (CCaCURdr::evCACURDR_OK == cutReader.GetRecordsInNextTimeFrame(cRec))
        {
            sysCpuData += cRec.sysCPUUtil;
            sysMemData += cRec.sysMemUsage;

            cutCnt++;
            CoreUtilMap::iterator cit = cRec.corUtilMap.begin();
            CoreUtilMap::iterator cend = cRec.corUtilMap.end();

            for (; cit != cend; cit++)
            {
                coreData[cit->first] += cit->second;
            }

            ProcessUtiMap::iterator pit = cRec.procUtiMap.begin();
            ProcessUtiMap::iterator pend = cRec.procUtiMap.end();

            for (; pit != pend; pit++)
            {
                pMapData[pit->first].cpuUtilization += pit->second.cpuUtilization;
                pMapData[pit->first].memoryConsumption += pit->second.memoryConsumption;
            }
        }
    }

    OsvDataItem* pL1 = NULL;
    OsvDataItem* pL2 = NULL;
    OsvDataItem* pTopic = new OsvDataItem("Utilization");

    if (!pTopic)
    {

        return NULL;
    }

    if (bShowCUT)
    {
        pL1 = pTopic->addChild("Avg System CPU Utilization", QString::number(sysCpuData / cutCnt) + " %");

        for (unsigned int i = 0; i < cutHdr.num_cores; i++)
        {
            OsvDataItem* pCore = new OsvDataItem(
                QString("Avg Core ") + QString::number(i) + " Utilization",
                QString::number((double)coreData[i] / cutCnt) + " %");

            if (!pCore)
            {

                break;
            }

            pL1->appendChild(pCore);
        }

        pL1 = pL1->addSibling("Avg System Memory Utilization", QString::number((double)sysMemData / cutCnt) + " %");
        pL2 = pL1->addChild("Virtual Memory (MB)", QString::number((double)cutHdr.vir_memroy / 1024000));
        pL2 = pL2->addSibling("Physical Memory (MB)", QString::number((double)cutHdr.phy_memory / 1024000));

        ProcessUtiMap::iterator pit = pMapData.begin();
        ProcessUtiMap::iterator pend = pMapData.end();

        for (; pit != pend; pit++)
        {
            pL1 = pL1->addSibling(QString("Avg Process Utilization (PID : ")
                                  + QString::number(pit->first) + ")",
                                  QString("cpu ") + QString::number((double)pit->second.cpuUtilization / cutCnt)
                                  + "% : mem " + QString::number((double)pit->second.memoryConsumption / cutCnt) + "%",
                                  "tlv:" + QString::number(pit->first));
        }
    }

    if ((NULL != m_pCLU) && bCLU)
    {
        if (bShowCUT)
        {
            pL1 = pL1->addSibling(QString("Avg Cache Line Utilization"),
                                  QString::number(m_pCLU->getAvgUtil()) + "%",
                                  "clu:");
        }
        else
        {
            pL1 = pTopic->addChild(QString("Avg Cache Line Utilization"),
                                   QString::number(m_pCLU->getAvgUtil()) + "%",
                                   "clu:");
        }
    }

    return pTopic;
#else
    (void)(cutPath); // unused
    (void)(bCLU); // unused
#endif
    return NULL;
}

double OsvTranslator::GetOcltDuration(const QString& ocltDir) const
{
#if SUPPORT_OCL
    gtString path(ocltDir.toStdWString().c_str());
    path.append(L"/");
    QString pid = ocltDir.section("/", -1);
    path.append(pid.toStdWString().c_str());
    path.append(L".trc");

    TrcReader trcRdr;

    if (trcRdr.open(path) != 0)
    {
        return 0.0;
    }

    const TrcHeader* trcHdr = trcRdr.getTrcHeader();

    if (!trcHdr)
    {
        return 0.0;
    }

    const TrcRecord* pRec = NULL;

    const TrcExtRecord* pExt = NULL;

    if (trcRdr.getNextRecord(&pRec, &pExt) != 0)
    {
        return 0.0;
    }

    gtUInt64 firstTs = pRec->proTs;
    double profDurationSec = (double)(trcHdr->lastTs - firstTs) / trcHdr->highResFreq;
    return profDurationSec;
#else
    (void)(ocltDir); // unused
    return 0.0;
#endif
}

OsvDataItem* OsvTranslator::generateOsvOclSection(QString ocltDir, bool bCut)
{
#if SUPPORT_OCL
    gtString path(ocltDir.toStdWString().c_str());
    path.append(L"/");
    QString pid = ocltDir.section("/", -1);
    path.append(pid.toStdWString().c_str());

    // Initialize ASV reader stuff
    gtString asvPath  = path + L".asv";
    AsvReader asvRdr;

    if (asvRdr.open(asvPath) != 0)
    {
        return NULL;
    }

    const AsvHeader* asvHdr = asvRdr.getAsvHeader();

    if (!asvHdr)
    {
        return NULL;
    }

    const CtxCqTidTypeOclApiMap* pApiMap = asvRdr.getApiMap();

    if (!pApiMap)
    {
        return NULL;
    }

    // Initialize CTX reader stuff
    gtString ctxPath  = path + L".ctx";
    CtxReader ctxRdr;

    if (ctxRdr.open(ctxPath) != 0)
    {
        return NULL;
    }

    const CtxHeader* ctxHdr = ctxRdr.getCtxHeader();

    if (!ctxHdr)
    {
        return NULL;
    }

    const DeviceList* pDevList = ctxRdr.getDevList();

    if (!pDevList)
    {
        return NULL;
    }

    const ProgramList* pProgList = ctxRdr.getProgList();

    if (!pProgList)
    {
        return NULL;
    }

    OsvDataItem* pL1 = NULL;
    OsvDataItem* pL2 = NULL;
    OsvDataItem* pOclPid = NULL;
    pOclPid = new OsvDataItem("OpenCL Process : " + pid,
                              "[View OpenCL Timeline]",
                              "tlv:" + pid);

    if (!pOclPid)
    {

        return NULL;
    }

    double profDurationSec = GetOcltDuration(ocltDir);

    if (0.0 != profDurationSec)
        pL1 = pOclPid->addChild("OpenCL Profile Duration",
                                QString::number((double)profDurationSec) + " sec");
    else
    {
        pL1 = pOclPid->addChild("OpenCL Profile Duration", "Unknown");
    }

    // Build up Tid List
    gtList<unsigned int> tidList;
    CtxCqTidTypeOclApiMap::const_iterator ait = pApiMap->begin();
    CtxCqTidTypeOclApiMap::const_iterator aend = pApiMap->end();

    for (; ait != aend; ait++)
    {
        gtList<unsigned int>::iterator tit = tidList.begin();
        gtList<unsigned int>::iterator tend = tidList.end();

        for (; tit != tend; tit++)
        {
            if (ait->first.tid == *tit)
            {
                break;
            }
        }

        if (tit == tend)
        {
            tidList.push_back(ait->first.tid);
        }
    }

    pL1 = pL1->addSibling("Number of OpenCL Threads",
                          QString::number(tidList.size()));

    pL1 = pL1->addSibling("Number of OpenCL Devices",
                          QString::number(pDevList->size()));

    pL1 = pL1->addSibling("Number of OpenCL Programs",
                          QString::number(pProgList->size()));

    pL1 = pL1->addSibling("Number of OpenCL Contexts",
                          QString::number(ctxHdr->numOfCtx));

    pL1 = pL1->addSibling("Number of Command Queues",
                          QString::number(ctxHdr->numOfCq));

    //-------------------
    pL1 = pL1->addSibling("OpenCL API Summary",
                          "[View API Summary]",
                          "asv:" + pid);
    pL2 = pL1->addChild("Number of OpenCL API calls",
                        QString::number(asvHdr->numOfApis));
    pL2 = pL2->addSibling("Number of OpenCL API instances",
                          QString::number(asvHdr->numOfInsts));

    //-------------------
    pL1 = pL1->addSibling("OpenCL Kernel Execution Summary",
                          "[View Kernel Execution Summary]",
                          "kxv:" + pid);
    pL2 = pL1->addChild("Number of Kernels",
                        QString::number(ctxHdr->numOfKernel));
    pL2 = pL2->addSibling("Number of kernel execution instances",
                          QString::number(asvHdr->numOfInsts_kx));

    //-------------------
    pL1 = pL1->addSibling("OpenCL Data Transfer Summary",
                          "[View Data Transfer Summary]",
                          "dxv:" + pid);
    pL2 = pL1->addChild("Number of data transfer instances",
                        QString::number(asvHdr->numOfInsts_dx));

    //  pL2 = pL1->addChild("Number of Host-to-Device transfers");
    //  pL2 = pL2->addSibling("Host-to-Device transfer rate");
    //  pL2 = pL1->addSibling("Number of Device-to-Host transfers");
    //  pL2 = pL2->addSibling("Device-to-Host transfer rate");
    //  pL2 = pL2->addSibling("Number of copying");
    //  pL2 = pL2->addSibling("Copying transfer rate");
    //  pL2 = pL2->addSibling("Number of Mapping");
    //  pL2 = pL2->addSibling("Mapping transfer rate");

    // TODO: Add more info here

    return pOclPid;
#else
    (void)(ocltDir); // unused
    (void)(bCut); // unused
    return NULL;
#endif
}

OsvDataItem* OsvTranslator::generateSessionReport()
{
    OsvDataItem* pL1 = NULL;
    OsvDataItem* pL2 = NULL;
    OsvDataItem* pL3 = NULL;

    pL1 = new OsvDataItem("Profile Session Report");

    if (!pL1)
    {

        return NULL;
    }

    // Generate data translation report
    pL2 = new OsvDataItem("Data Translation Report");

    if (!pL2)
    {

        return NULL;
    }

    pL1->appendChild(pL2);

    gtList<QString>::iterator it = m_transReportEntryList.begin(), itEnd = m_transReportEntryList.end();

    for (int i = 1; it != itEnd; ++it, ++i)
    {
        pL3 = new OsvDataItem("Issue " + QString::number(i), *it);

        if (!pL3)
        {

            break;
        }

        pL2->appendChild(pL3);
    } // for loop

    return pL1;
}
