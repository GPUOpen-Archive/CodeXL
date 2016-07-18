//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CluInfo.cpp
/// \brief Implements a class which process IBS record and generate the
///        information regarding cache line utilization
///
//==================================================================================

#include "CluInfo.h"
#include "Cache.h"

// Un-defining MAINTAIN_DATA will not store data accesses with sample data
#define MAINTAIN_DATA


CluInfo::CluInfo(UINT8 l1DcAssoc,
                 UINT8 l1DcLineSize,
                 UINT8 l1DcLinesPerTag,
                 UINT8 l1DcSize)
{
    m_totCacheBytes = 0;
    m_totCacheEvictions = 0;
    m_modIndex = 0;
    m_funcIndex = 0;
    m_pCacheUtilMap = NULL;
    m_pCacheErrors = NULL;
    m_pModInfoMap = NULL;
    m_initialized = false;
    m_l1DcAssoc = l1DcAssoc;
    m_l1DcLineSize = l1DcLineSize;
    m_l1DcLinesPerTag = l1DcLinesPerTag;
    m_l1DcSize = l1DcSize;
    m_pCache = NULL;
}

CluInfo::~CluInfo()
{
    if (NULL != m_pCacheUtilMap)
    {
        delete m_pCacheUtilMap;
    }

    if (NULL != m_pCacheErrors)
    {
        delete m_pCacheErrors;
    }

    if (NULL != m_pModInfoMap)
    {
        delete m_pModInfoMap;
    }

    if (NULL != m_pCache)
    {
        delete m_pCache;
    }

    m_pCacheUtilMap = NULL;
    m_pCacheErrors = NULL;
    m_pModInfoMap = NULL;
    m_pCache = NULL;
    m_initialized = false;
}

void CluInfo::AddToErrorMap(unsigned char err, gtVAddr RIPx, const char* mnemx)
{
    CLUErrorMap::iterator it = m_pCacheErrors->find((err));

    if (it == m_pCacheErrors->end())
    {
        CLUERRData dat0;
        dat0.count = 1;
        CLUripData dat1;
        dat1.count = 1;
        strncpy_s(dat1.mnem, (mnemx), 50);
        dat0.errMap.insert(std::pair < gtVAddr, CLUripData > ((RIPx), dat1));
        m_pCacheErrors->insert(std::pair<unsigned char, CLUERRData > ((err), dat0));
    }
    else
    {
        it->second.count++;
        CLUErrData::iterator it2 = it->second.errMap.find((RIPx));

        if (it2 == it->second.errMap.end())
        {
            CLUripData dat1;
            dat1.count = 0;
            strncpy_s(dat1.mnem, (mnemx), 50);
            it->second.errMap.insert(std::pair< gtVAddr, CLUripData > ((RIPx), dat1));
            it2 = it->second.errMap.find((RIPx));
        }

        it2->second.count++;
    }
}

unsigned char CluInfo::GetOperationSize(gtVAddr RIP,
                                        BOOL isLoad,
                                        TiModuleInfo* pModInfo,
                                        NameModuleMap* pMMap,
                                        bool& bErr,
                                        UINT32* modIndex)
{
    unsigned char size;
    gtVAddr codeVaddr;
    DWORD codeOffset;
    unsigned char* code;
    UINT32 csiz;
    UINT8 ErrorCode;
    unsigned int memopIndex = 0;
    bErr = false;

    InstructionType instr;
    eAddrMode mode;
    bool found = false;

    CLUModInfoMap::iterator fmit;

    if (!pModInfo->pModulename || (wcslen(pModInfo->pModulename) == 0))
    {
        // Module name empty - log the event and default to 4 bytes
        AddToErrorMap(MODULE_NAME_EMPTY, RIP, "");
        bErr = true;
        return MODULE_NAME_EMPTY;  // Default to 4 bytes
    }

    LibDisassembler dasm;
    NameModuleMap::iterator mit = pMMap->find(pModInfo->pModulename);

    if (mit == pMMap->end())
    {
        // Module not in module map - log the event and default to 4 bytes
        AddToErrorMap(MODULE_NOT_IN_MAP, RIP, "");
        bErr = true;
        return MODULE_NOT_IN_MAP;  // Default to 4 bytes
    }

    fmit = m_pModInfoMap->find(pModInfo->pModulename);

    if (fmit == m_pModInfoMap->end())
    {
        // Added a new module - initialize its values
        CLUModInfoData data;
        data.index = m_modIndex++;
        m_pModInfoMap->insert(std::pair<wchar_t*, CLUModInfoData > (pModInfo->pModulename, data));
        fmit = m_pModInfoMap->find(pModInfo->pModulename);
    }

    *modIndex = fmit->second.index;

    // mit points to the module info struct
    // fmit points to our module info, which has the code byte info
    if ((fmit->second.getCodeBytes() == NULL) ||
        (fmit->second.startRva > static_cast<gtRVAddr>(RIP - mit->second.m_base)) ||
        (fmit->second.endRva < static_cast<gtRVAddr>(RIP - mit->second.m_base)))
    {
        // Open the executable and get code byte and function information
        PeFile pe(pModInfo->pModulename);

        if (fmit->second.getCodeBytes())
        {
            // If there was already code and the VA is not within its range
            // free the old code memory
            free(fmit->second.getCodeBytes());
            fmit->second.setCodeBytes(NULL);
        }

        if (!pe.Open())
        {
            // Try to prepend the session path
            gtString sessionModule;

            if (ExtractFileName(pModInfo->pModulename, sessionModule))
            {
                sessionModule.prepend(L'\\');
                sessionModule.prepend(pModInfo->pSessionDir);
            }

            pe.Reset(sessionModule.asCharArray());

            if (!pe.Open())
            {
                // The module can't be opened - log the event and default to 4 bytes
                AddToErrorMap(CANNOT_OPEN, RIP, "");
                bErr = true;
                return CANNOT_OPEN;  // Default to 4 bytes
            }
        }

        unsigned secIndex = pe.LookupSectionIndex(static_cast<gtRVAddr>(RIP - pModInfo->ModuleStartAddr));

        if (!pe.GetSectionRvaLimits(secIndex, fmit->second.startRva, fmit->second.endRva))
        {
            pe.Close();
            AddToErrorMap(RIP_OVERRUN, RIP, "");
            bErr = true;
            return RIP_OVERRUN;  // Default to 4 bytes
        }

        const unsigned char* tmp = pe.GetSectionBytes(secIndex);

#ifdef ALLOC_FULL_CODE_BUFFER
        csiz = fmit->second.endRva - fmit->second.startRva;
        unsigned char* tmp2 = (unsigned char*) calloc(1, csiz);
        memcpy(tmp2, tmp, csiz);
        fmit->second.setCodeBytes(tmp2);

        fmit->second.codeSize = csiz;
        fmit->second.codeOffset = pe.GetCodeBase();
#else
        // Reduce the amount of memory - more alloc/free, but less memory used
        codeOffset = RIP - pModInfo->ModuleStartAddr - fmit->second.startRva;
        csiz = std::min((DWORD)4096, (fmit->second.endRva - (fmit->second.startRva + codeOffset)));
        fmit->second.startRva = static_cast<gtRVAddr>(RIP - pModInfo->ModuleStartAddr);
        fmit->second.endRva = static_cast<gtRVAddr>(fmit->second.startRva + csiz - 1);
        unsigned char* tmp2 = (unsigned char*) calloc(1, csiz);
        memcpy(tmp2, &tmp[codeOffset], csiz);
        fmit->second.setCodeBytes(tmp2);

        fmit->second.codeSize = csiz;
        fmit->second.codeOffset = pe.GetCodeBase();
#endif

        pe.Close();
    }

    // Find the size of the operation - use the disassembler
    codeVaddr = pModInfo->ModuleStartAddr;
    code = fmit->second.getCodeBytes();
    codeOffset = RIP - pModInfo->ModuleStartAddr - fmit->second.startRva;

    if (codeOffset > fmit->second.codeSize)
    {
        // This should never happen
        AddToErrorMap(OFFSET_OVERFLOW, RIP, "");
        bErr = true;
        return OFFSET_OVERFLOW;  // Default to 4 bytes
    }

    dasm.SetLongMode(!mit->second.m_is32Bit);
    dasm.SetDefaultSegSize(mit->second.m_is32Bit);

    unsigned int strLength = 1000;
    BYTE ASCIICode[1000];
    UIInstInfoType dType;

    if (E_FAIL == dasm.UIDisassemble(&code[codeOffset], &strLength, ASCIICode, &dType, &ErrorCode))
    {
        // The disassembly failed - bad news

        // Toggle LongMode and try again
        dasm.SetLongMode(mit->second.m_is32Bit);
        dasm.SetDefaultSegSize(!mit->second.m_is32Bit);
        strLength = 1000;

        if (E_FAIL == dasm.UIDisassemble(&code[codeOffset], &strLength, ASCIICode, &dType, &ErrorCode))
        {
            char str[50];
            int xx;
            str[0] = '\0';

            for (xx = 0; xx < 16; xx++)
            {
                char str2[10];
                sprintf_s(str2, "%02x ", code[codeOffset + xx]);
                strcat_s(str, str2);
            }

            AddToErrorMap(DISASM_FAILED, RIP, str);
            bErr = true;
            return DISASM_FAILED;  // Default to 4 bytes
        }

        dasm.SetLongMode(!mit->second.m_is32Bit);
        dasm.SetDefaultSegSize(mit->second.m_is32Bit);
    }

    for (int i = 0; i < 3; i++)
    {
        if (!dType.bHasMemOp[i])
        {
            continue;
        }

        if (!found)
        {
            memopIndex = i;
            found = true;
        }
        else
        {
            // Return the size of the correct memory operation in case there are multiple
            memopIndex = isLoad ? i : memopIndex;
        }
    }

    if (!found)
    {

#if 1

        // Here, we try the Etch disassembler to see if it knows the operand types.
        // This is a hack, but the disassembler isn't up-to-date apparently.
        // TODO - HACK
        if (E_FAIL != dasm.EtchDisassemble(&code[codeOffset], &instr, &ErrorCode))
        {
            // The Etch disassembler handled it - let's see if the memop is found
            for (int i = 0; i < instr.NumOperands; i++)
            {
                mode = instr.Operands[i].AddrMode;

                if ((mode == evMemory) || (mode == evPCRelative))
                {
                    switch (instr.Operands[i].OpSize)
                    {
                        case evSizeByte:
                            size = 1;
                            break;

                        case evSizeWord:
                            size = 2;
                            break;

                        case evSizeDWord:
                            size = 4;
                            break;

                        case evSizeFWord:
                            size = 6;
                            break;

                        case evSizeQWord:
                            size = 8;
                            break;

                        case evSizeTByte:
                            size = 10;
                            break;

                        case evSizeDQWord:
                            size = 16;
                            break;

                        case evSizeNone:
                        default:
                            AddToErrorMap(SIZE_ERROR, RIP, (const char*) ASCIICode);
                            return SIZE_ERROR;
                            break;
                    }

                    found = true;
                    dType.bHasMemOp[memopIndex] = true;
                    dType.MemAccessSize[memopIndex] = size;
                    break;
                }
            }
        }

#endif

        if (!found)
        {
            // There are cases with micro-code ops where they will have
            // a memory event but no memory operands.  Tally these, as we
            // may want to use that number later.
            AddToErrorMap(NO_MEMOPS, RIP, (const char*) ASCIICode);
            bErr = true;
            return NO_MEMOPS;  // Default to 4 bytes
        }
    }

    if (0 == dType.MemAccessSize[memopIndex])
    {
        AddToErrorMap(SIZE_ERROR, RIP, (const char*) ASCIICode);
        bErr = true;
        return SIZE_ERROR;  // Default to 4 bytes
    }

    return dType.MemAccessSize[memopIndex];
}

void CluInfo::RecordCacheLdSt(IBSOpRecordData* ibsOpRec,
                              TiModuleInfo* pModInfo,
                              NameModuleMap* pMMap,
                              bool isLoad)
{
    unsigned char size;
    bool bErr;
    UINT32 modIndex;

    if (!Initialize())
    {
        return;
    }

    if (!ibsOpRec->m_IbsDcPhyAddrValid)
    {
        // Should never happen
        return;
    }

    if (!ibsOpRec->m_IbsDcLinAddrValid)
    {
        // Ignore these, as they are most likely micro-ops with no effect on caches
        return;
    }

#ifdef DEBUG_CACHE_EVENT
    PrintMemoryUsage("\nMemory Usage: Before GetOperationSize.\n");
#endif

    // Add process and module data for cache utilization processing

    size = GetOperationSize(ibsOpRec->m_RIP, ibsOpRec->m_IbsLdOp, pModInfo, pMMap, bErr, &modIndex);

#ifdef DEBUG_CACHE_EVENT
    PrintMemoryUsage("\nMemory Usage: After GetOperationSize.\n");
#endif

    if (size < CA_DATA_SIZE_MAX)
    {
        CacheEvent(ibsOpRec, size, isLoad, bErr, modIndex, false);

#ifdef DEBUG_CACHE_EVENT
        PrintMemoryUsage("\nMemory Usage: After CacheEvent.\n");
#endif
        return;
    }

    // An error occurred
    switch (size)
    {
        case MODULE_NOT_IN_MAP: // Could not find module name in map
            CacheEvent(ibsOpRec, CLU_DEFAULT_SIZE, isLoad, bErr, modIndex, false);
            break;

        case CANNOT_OPEN:       // Module could not be opened - path error?
            CacheEvent(ibsOpRec, CLU_DEFAULT_SIZE, isLoad, bErr, modIndex, false);
            break;

        case OFFSET_OVERFLOW:   // RIP outside bounds of code segment
            CacheEvent(ibsOpRec, CLU_DEFAULT_SIZE, isLoad, bErr, modIndex, false);
            break;

        case DISASM_FAILED:     // EtchDisassemble returned E_FAIL
            CacheEvent(ibsOpRec, CLU_DEFAULT_SIZE, isLoad, bErr, modIndex, false);
            break;

        case NO_MEMOPS:         // The instruction had no memory operations
            CacheEvent(ibsOpRec, CLU_DEFAULT_SIZE, isLoad, bErr, modIndex, false);
            break;

        case SIZE_ERROR:        // The parameter size was not in case statement
            CacheEvent(ibsOpRec, CLU_DEFAULT_SIZE, isLoad, bErr, modIndex, false);
            break;

        default:
            return;
            break;

    }

#ifdef DEBUG_CACHE_EVENT
    PrintMemoryUsage("\nMemory Usage: After CacheEvent with error.\n");
#endif

    return;
}

void CluInfo::CacheEvent(IBSOpRecordData* ibsOpRec,
                         unsigned char size,
                         bool isLoad,
                         bool bSizeUnknown,
                         UINT32 modIndex,
                         bool bSpansLines)
{
    gtVAddr IbsDcPhysAd = ibsOpRec->m_IbsDcPhysAd;
    unsigned char ProcessorID = ibsOpRec->m_ProcessorID;
    unsigned index = m_pCache->GetIndex(IbsDcPhysAd);
    unsigned offset = m_pCache->GetOffset(IbsDcPhysAd);
    CacheDataStuff* cacheDataStuff = nullptr;

    // Check for access spanning 2 cache lines
    if (m_pCache->SpansLines(offset, size))
    {
        unsigned char off = m_pCache->GetBytesPerLine() - offset; // remaining bytes in 1st cache line
        CacheEvent(ibsOpRec, off, isLoad, bSizeUnknown, modIndex, false);
        // Adjust the address
        ibsOpRec->m_IbsDcPhysAd += off;
        ibsOpRec->m_IbsDcLinAd += off;
        CacheEvent(ibsOpRec, size - off, isLoad, bSizeUnknown, modIndex, true);

        // Re-adjust addresses
        ibsOpRec->m_IbsDcPhysAd -= off;
        ibsOpRec->m_IbsDcLinAd -= off;

        return;
    }

    if (CACHE_EVICTED == m_pCache->CacheAccess(IbsDcPhysAd, &cacheDataStuff, ProcessorID))
    {
        CacheLineEviction(*cacheDataStuff, index, ProcessorID);
    }

    IncrCacheByteCount(*cacheDataStuff, ibsOpRec, size, offset, isLoad, bSizeUnknown, modIndex, bSpansLines);
}

void CluInfo::CacheLineCleanup()
{
    unsigned char core;
    unsigned int index;
    CacheDataStuff* data = NULL;

    if (!Initialize())
    {
        return;
    }

    data = m_pCache->GetNextValidLine(core, index);

    for (; NULL != data; data = m_pCache->GetNextValidLine(core, index))
    {
        data->tot_evictions--;
        CacheLineEviction(*data, index, core);
    }
}

void CluInfo::CacheLineEviction(CacheDataStuff& cData, unsigned int index, unsigned char core)
{
    cData.tot_evictions++;

    // When an eviction occurs on a line, be sure to put out all RIPs that accessed the line

    PidRIPMap::iterator pidrip_it;

    // For each instruction that has accessed this line since the last eviction
    for (pidrip_it = cData.pidripmap.begin(); pidrip_it != cData.pidripmap.end(); ++pidrip_it)
    {
        PidRIPKey prKey = pidrip_it->first; // <PID, TID, RIP>
        CLUKey cluKey(core, prKey.PID, prKey.TID, prKey.RIP, index);
        CLUData cluData;    // Constructor initializes this
        UINT64 bitmap = pidrip_it->second.access_bitmap;    // bitmap of accessed bytes for this instruction
        cData.access_bitmap |= bitmap;  // Not sure if this is valuable.  All bytes accessed by all instructions since last eviction

        bitmap = m_pCache->CountBits(bitmap);

        // See if this PID/TID/RIP previously accessed this cache line on this core
        CLUMap::iterator cluMap_it = m_pCacheUtilMap->find(cluKey);

        if (cluMap_it == m_pCacheUtilMap->end())
        {
            cluMap_it = m_pCacheUtilMap->insert(std::pair <CLUKey, CLUData > (cluKey, cluData)).first;
        }

        // Update data for the instruction
        cluMap_it->second.byteMask |= pidrip_it->second.access_bitmap;
        cluMap_it->second.tot_evictions++;  // Increment # evictions
        cluMap_it->second.min_bytes = std::min(bitmap, (UINT64)cluMap_it->second.min_bytes);
        cluMap_it->second.max_bytes = std::max(bitmap, (UINT64)cluMap_it->second.max_bytes);
        cluMap_it->second.bSizeUnknown = pidrip_it->second.bSizeUnknown;
        cluMap_it->second.tot_rw += pidrip_it->second.rw_bytes;
        cluMap_it->second.num_rw += pidrip_it->second.rw_count;
        cluMap_it->second.modIndex = pidrip_it->second.modIndex;
        cluMap_it->second.SpanCount += pidrip_it->second.SpanCount;
        cluMap_it->second.sumMax += bitmap;

#ifdef MAINTAIN_DATA
        AddrMap::iterator amit = pidrip_it->second.dataAddresses.begin();

        for (; amit != pidrip_it->second.dataAddresses.end(); ++amit)
        {
            AddrMap::iterator amit2 = cluMap_it->second.dataAddresses.find(amit->first);

            if (amit2 != cluMap_it->second.dataAddresses.end())
            {
                amit2->second.splice(amit2->second.end(), amit->second);
            }
            else
            {
                cluMap_it->second.dataAddresses.insert(AddrMap::value_type(amit->first, amit->second));
            }
        }

        pidrip_it->second.dataAddresses.clear();
#endif

        // Update data for the line
        cData.max_bytes = std::max(bitmap, (UINT64)cData.max_bytes);
        cData.min_bytes = std::min(bitmap, (UINT64)cData.min_bytes);
        cData.tot_rw += pidrip_it->second.rw_count;
        cData.tot_rw_bytes += pidrip_it->second.rw_bytes;
    }

    cData.pidripmap.clear();
}

void CluInfo::IncrCacheByteCount(CacheDataStuff& cacheData,
                                 IBSOpRecordData* ibsOpRec,
                                 unsigned char size,
                                 unsigned int offset,
                                 bool isLoad,
                                 bool bSizeUnknown,
                                 UINT32 modIndex,
                                 bool bSpansLines)
{
    GT_UNREFERENCED_PARAMETER(isLoad);

    UINT32 PID = ibsOpRec->m_PID;
    gtVAddr RIP = ibsOpRec->m_RIP;
    UINT32 TID = ibsOpRec->m_ThreadHandle;
    UINT8 core = ibsOpRec->m_ProcessorID;
    UINT64 bitmap = (1 << size) - 1;    // Number of bits in low-order bits
    bitmap <<= 64 - offset - size;

    PidRIPMap::iterator it = cacheData.pidripmap.find(PidRIPKey(PID, TID, RIP, core));

    if (it == cacheData.pidripmap.end())
    {
        PidRIPData pidrip_data;
        pidrip_data.access_bitmap = 0;
        pidrip_data.bSizeUnknown = bSizeUnknown;
        pidrip_data.modIndex = modIndex;
        pidrip_data.rw_count = 0;
        pidrip_data.rw_bytes = 0;
        pidrip_data.SpanCount = 0;

        PidRIPKey key(PID, TID, RIP, core);

        it = cacheData.pidripmap.insert(std::pair < PidRIPKey, PidRIPData > (key, pidrip_data)).first;
    }

    it->second.rw_bytes += size;
    it->second.access_bitmap |= bitmap;

    // bSpansLines is true for the second half of a spanning access.
    // The first half incremented the number of R/W accesses and added
    // the VADDR of the access.

    if (!bSpansLines)
    {
        UINT32 isRemote;

        isRemote = ibsOpRec->m_NbIbsReqSrc ? ibsOpRec->m_NbIbsReqDstProc : 0;

        it->second.rw_count++;

        AddrMapKey key(ibsOpRec->m_IbsDcLinAd, ibsOpRec->m_IbsDcPhysAd);

#ifdef MAINTAIN_DATA
        AddrMap::iterator prit = it->second.dataAddresses.find(key);

        if (prit == it->second.dataAddresses.end())
        {
            AddrData tsVec;
            tsVec.push_back(TimeStampType(ibsOpRec->m_DeltaTick, isRemote));
            it->second.dataAddresses.insert(AddrMap::value_type(key, tsVec));
        }
        else
        {
            prit->second.push_back(TimeStampType(ibsOpRec->m_DeltaTick, isRemote));
        }

#endif
    }
    else
    {
        it->second.SpanCount++;
    }
}

bool CluInfo::Initialize()
{
#define CLU_RETURN_IF_NULL(x) if(NULL == (x)) return m_initialized

    if (m_initialized)
    {
        return true;
    }

    CLU_RETURN_IF_NULL(m_pCacheUtilMap = new CLUMap);
    CLU_RETURN_IF_NULL(m_pCacheErrors = new CLUErrorMap);
    CLU_RETURN_IF_NULL(m_pModInfoMap = new CLUModInfoMap);
    CLU_RETURN_IF_NULL(m_pCache = new Cache);

    m_initialized = m_pCache->Init(m_l1DcAssoc, m_l1DcLineSize, m_l1DcLinesPerTag, m_l1DcSize);

    return m_initialized;
}
