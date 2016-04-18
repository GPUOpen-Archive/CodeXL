//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileReader.h
/// \brief Interface for the CpuProfileReader class.
///
//==================================================================================

#ifndef _CPUPROFILEREADER_H_
#define _CPUPROFILEREADER_H_

#include "CpuProfileInputStream.h"
#include "CpuProfileInfo.h"
#include "CpuProfileModule.h"
#include "CpuProfileProcess.h"

/**********************************
 * class CpuProfileReader
 *
 * Description:
 * This class implements TBP/EBP reader which is the
 * main interface for accessing profile data
 */
class CP_RAWDATA_API CpuProfileReader : public CpuProfileInputStream
{
public:
    CpuProfileReader();
    ~CpuProfileReader();


    /* Open profile file and read profile info.
     * On success, the CpuProfileInfo should be
     * populated.
     */
    virtual bool open(const gtString& path) override;
    virtual void close() override;


    /* Return the pointer to CpuProfileInfo on success. */
    CpuProfileInfo* getProfileInfo();

    /* Return the pointer to populated PidProcessMap on success.
     * If the map has not yet been populated, the reader will read the
     * [PROCESSDATA] section in the profile file.
     */
    PidProcessMap* getProcessMap();

    /* Return the pointer to populated NameModuleMap on success.
     * If the map has not yet been populated, the reader will read the
     * [MODDATA] section in the profile file.
     */
    NameModuleMap* getModuleMap();

    /* Return the pointer to populated CoreTopologyMap on success. */
    CoreTopologyMap* getTopologyMap();

    /* Return the pointer to CpuProfileModule stored in the NameModuleMap
     * on success.  If the CpuProfileModule entry has not yet been filled with
     * module detail from the IMD file, it will read the appropriated
     * IMD file.
     */
    CpuProfileModule* getModuleDetail(const gtString& modName);

    const gtString& getPath() const { return m_path; }

private:
    bool checkVersion();

    bool readRunInfoSection();

    void processRunInfoLine(const gtString& line);

    bool readEnvSection();

    void processEnvLine(const gtString& line);

    bool getImd(CpuProfileModule& mod);

    void processProcLine(PidProcessMap& procMap, const gtString& line);

    void processModLine(NameModuleMap& modMap, const gtString& line);

private:
    CpuProfileInfo  m_profileInfo;
    PidProcessMap   m_procMap;
    NameModuleMap   m_modMap;
    CoreTopologyMap m_topMap;
    bool m_isProcessDataRead;
    bool m_isModDataRead;
};

#endif // _CPUPROFILEREADER_H_
