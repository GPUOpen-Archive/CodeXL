//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CluInfo.h
/// \brief Implements a class which process IBS record and generate the
///        information regarding cache line utilization
///
//==================================================================================

#ifndef _CLUINFO_H_
#define _CLUINFO_H_

#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <QDir>
#include <map>
#include <list>

#include <AMDTCpuProfilingRawData/inc/Windows/PRDReader.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileDataTranslationInfo.h>
#include <AMDTCpuProfilingRawData/inc/CpuProfileModule.h>
#include <AMDTExecutableFormat/inc/PeFile.h>
#include <AMDTDisassembler/inc/LibDisassembler.h>
#include <AMDTCpuProfilingTranslation/inc/Windows/TaskInfoInterface.h>

using namespace std;


#define MAX_UINT64 0xffffffffffffffffui64
#define MAX_UINT32 0xffffffffU
#define MAX_UINT8  0xffU


struct CLUModInfoData
{
    CLUModInfoData()
    {
        index = MAX_UINT32;
        codeBytes = NULL;
        startRva = GT_INVALID_RVADDR;
        endRva = 0;
        codeSize = 0;
        codeOffset = 0;
    };

    ~CLUModInfoData()
    {
        if (codeBytes)
        {
            free(codeBytes);
        }

        codeBytes = NULL;
        startRva = GT_INVALID_RVADDR;
        endRva = 0;
        codeSize = 0;
        codeOffset = 0;
    }

    void setCodeBytes(unsigned char* b) { codeBytes = b; };
    unsigned char* getCodeBytes() { return codeBytes; } ;

    UINT32   index;         // Index of this entry used by CLU data to refer to a module
    UINT8*   codeBytes;
    gtRVAddr startRva;      // Starting RVA of codeBytes
    gtRVAddr endRva;        // Ending RVA of codeBytes
    UINT32   codeSize;
    UINT32   codeOffset;
};

typedef map<wstring, CLUModInfoData > CLUModInfoMap;


typedef pair<UINT64, UINT8 > TimeStampType; // Timestamp with bool isRemote
typedef list<TimeStampType > AddrData;  // Timestamps
typedef pair<gtVAddr, UINT64 > AddrMapKey;  // VA and PA
typedef map<AddrMapKey, AddrData > AddrMap;


struct PidRIPData
{
    // data maintained for each instruction between evictions

    PidRIPData()
    {
        rw_bytes = 0;
        access_bitmap = 0;
        rw_count = 0;
        modIndex = MAX_UINT32;
        SpanCount = 0;
        bSizeUnknown = 0;
        dataAddresses.clear();
    }

    UINT64  access_bitmap;  // bitmap of bytes accessed by this instruction
    UINT64  rw_bytes;       // Number of bytes read / written
    UINT32  rw_count;       // Number of reads / writes
    UINT32  modIndex;       // The index of the module for which this RIP belongs
    UINT32  SpanCount;      // Number of times an access crossed cache line boundaries
    bool    bSizeUnknown;   // Size of operation can't be determined by disassembler
    AddrMap dataAddresses;  // All VA's of the data accesses and timestamps
};

struct PidRIPKey
{
    PidRIPKey(UINT32 pid, UINT32 tid, gtVAddr rip, UINT8 _core)
    {
        PID = pid;
        TID = tid;
        RIP = rip;
        core = _core;
    };

    bool operator< (const PidRIPKey& other) const
    {
        if (other.PID != this->PID)
        {
            return (other.PID > this->PID);
        }

        if (other.core != this->core)
        {
            return (other.core > this->core);
        }

        if (other.TID != this->TID)
        {
            return (other.TID > this->TID);
        }

        if (other.RIP > this->RIP)
        {
            return true;
        }

        return false;
    };

    UINT32  PID;
    UINT32  TID;
    gtVAddr RIP;
    UINT8   core;
};

typedef map< PidRIPKey, PidRIPData > PidRIPMap; // Key is PID/RIP


struct CacheDataStuff   // "For each cache line" data
{
    CacheDataStuff()
    {
        tag = 0;
        access_bitmap = 0;
        tot_rw_bytes = 0;
        tot_rw = 0;
        tot_evictions = 0;
        min_bytes = MAX_UINT8;
        max_bytes = 0;
    };

    UINT64      tag;            // Cache line tag
    UINT64      access_bitmap;  // bitmap of bytes accessed in the line, full run
    UINT64      tot_rw_bytes;   // # bytes read / written - total for this line, full run
    UINT64      timestamp;      // Used for LRU determination
    PidRIPMap   pidripmap;      // Instructions accessing this line
    UINT32      tot_rw;         // # reads / writes - total for this line, full run
    UINT32      tot_evictions;  // # times this line was evicted - total, full run
    UINT8       min_bytes;      // Minimum access size in bytes
    UINT8       max_bytes;      // Maximum access size in bytes
};

typedef multimap<unsigned int, CacheDataStuff > CacheMap;   // Key is the cache index (phy addr[14:6]), data is cache lines

typedef map<unsigned char, CacheMap > CoreCacheMap;     // Key is processor core



struct CLUKey
{
    CLUKey(UINT32 _core, ProcessIdType PID, ThreadIdType TID, gtVAddr _RIP, UINT32 line)
    {
        RIP = _RIP;
        core = _core;
        cache_line = line;
        ThreadID = TID;
        ProcessID = PID;
    };

    bool operator< (const CLUKey& other) const
    {
        if (other.ProcessID != this->ProcessID)
        {
            return (other.ProcessID > this->ProcessID);
        }

        if (other.core != this->core)
        {
            return (other.core > this->core);
        }

        if (other.cache_line != this->cache_line)
        {
            return (other.cache_line > this->cache_line);
        }

        if (other.ThreadID != this->ThreadID)
        {
            return (other.ThreadID > this->ThreadID);
        }

        if (other.RIP > this->RIP)
        {
            return true;
        }

        return false;
    };

    gtVAddr         RIP;
    ProcessIdType   ProcessID;
    ThreadIdType    ThreadID;
    UINT32          cache_line;
    UINT32          core;
};

struct CLUData
{
    CLUData()
    {
        tot_rw = 0;
        num_rw = 0;
        byteMask = 0;
        min_bytes = MAX_UINT8;
        max_bytes = 0;
        tot_evictions = 0;
        bSizeUnknown = 0;
        modIndex = MAX_UINT32;
        SpanCount = 0;
        sumMax = 0;
    };

    void clear(void)
    {
        tot_rw = 0;
        num_rw = 0;
        byteMask = 0;
        min_bytes = MAX_UINT8;
        max_bytes = 0;
        tot_evictions = 0;
        bSizeUnknown = 0;
        modIndex = MAX_UINT32;
        SpanCount = 0;
        sumMax = 0;
        dataAddresses.clear();
    };

    // addition operator overloading
    CLUData& operator+= (const CLUData& other)
    {
        this->tot_rw += other.tot_rw;
        this->tot_evictions += other.tot_evictions;
        this->byteMask |= other.byteMask;
        this->num_rw += other.num_rw;
        this->modIndex = other.modIndex;
        this->SpanCount += other.SpanCount;
        this->min_bytes = min(this->min_bytes, other.min_bytes);
        this->max_bytes = max(this->max_bytes, other.max_bytes);
        this->sumMax += other.max_bytes;
        this->bSizeUnknown = this->bSizeUnknown || other.bSizeUnknown;

        AddrMap::const_iterator otherIt = other.dataAddresses.cbegin();

        for (; otherIt != other.dataAddresses.cend(); otherIt++)
        {
            AddrMap::iterator thisIt;
            thisIt = this->dataAddresses.find(otherIt->first);

            if (thisIt == this->dataAddresses.end())
            {
                this->dataAddresses.insert(AddrMap::value_type(otherIt->first, otherIt->second));
            }
            else
            {
                AddrData tmpData;
                tmpData = otherIt->second;
                thisIt->second.splice(thisIt->second.end(), tmpData);
            }
        }

        return *this;
    };

    UINT64  tot_rw;         // Total number of bytes read / written
    // Utilization is calculated as bytes_accessed / 64 / tot_evictions
    // (a/64) = util. for 1 eviction
    // (a/64) + (b/64) = (a+b)/64; therefore ((a+b)/64)/2) = util. for 2 evictions
    // (a+b+...) = bytes_accessed
    // Total util. = ((bytes_accessed)/64)/tot_evictions
    UINT64  tot_evictions;  // Total number of evictions
    UINT64  byteMask;       // bitmap of bytes accessed in this line
    UINT32  sumMax;         // Used for aggregation
    UINT32  num_rw;         // Total number of reads / writes from this line
    UINT32  modIndex;       // The index of the module for which this RIP belongs
    UINT32  SpanCount;      // Number of times an access crossed cache line boundaries
    AddrMap dataAddresses;  // All VA's of the data accesses and timestamps
    SourceLineInfo lineInfo;        // Line number info for this RIP
    UINT8   min_bytes;      // minimum # bytes accessed between evictions
    UINT8   max_bytes;      // maximum # bytes accesses between evictions
    UINT8   bSizeUnknown;   // Couldn't disassemble and determine size
};

typedef map<CLUKey, CLUData > CLUMap;


struct CLUripData
{
    UINT32      count;
    char        mnem[50];
};

typedef std::map<gtVAddr, CLUripData > CLUErrData;

struct CLUERRData
{
    UINT64      count;
    CLUErrData  errMap;
};

typedef std::map<unsigned char, CLUERRData > CLUErrorMap;


// Error codes from CLU processing
#define MODULE_NOT_IN_MAP       0xff
#define CANNOT_OPEN             0xfe
#define OFFSET_OVERFLOW         0xfd
#define DISASM_FAILED           0xfc
#define NO_MEMOPS               0xfb
#define SIZE_ERROR              0xfa
#define MODULE_NAME_EMPTY       0xf9
#define RIP_OVERRUN             0x80

// if return value is < CA_DATA_SIZE_MAX then size is valid
#define CA_DATA_SIZE_MAX        0x40

// On error, assume the access was this size
#define CLU_DEFAULT_SIZE        4

class Cache;

class CluInfo
{
public:
    CluInfo(UINT8 l1DcAssoc,
            UINT8 l1DcLineSize,
            UINT8 l1DcLinesPerTag,
            UINT8 l1DcSize);

    ~CluInfo();

    CLUMap* GetCLUData()
    {
        return m_pCacheUtilMap;
    }

    void RecordCacheLdSt(IBSOpRecordData* ibsOpRec,
                         TiModuleInfo* pModInfo,
                         NameModuleMap* pMMap,
                         bool isLoad);

    void CacheLineCleanup();

    double GetAvgUtil()
    {
        if (!m_initialized)
        {
            return 0.0;
        }

        return (double) m_totCacheBytes / (double) m_totCacheEvictions / 64.0 * 100.0;
    }

private:
    void IncrCacheByteCount(CacheDataStuff& cData,
                            IBSOpRecordData* ibsOpRec,
                            unsigned char size,
                            unsigned int offset,
                            bool isLoad,
                            bool isUnaligned,
                            UINT32 modIndex,
                            bool bSpansLines);

    unsigned char GetOperationSize(gtVAddr RIP,
                                   BOOL isLoad,
                                   TiModuleInfo* pModInfo,
                                   NameModuleMap* pMMap,
                                   bool& bErr,
                                   UINT32* modIndex);

    void CacheEvent(IBSOpRecordData* ibsOpRec,
                    unsigned char size,
                    bool isLoad,
                    bool isUnaligned,
                    UINT32 modIndex,
                    bool bSpansLines);

    void CacheLineEviction(CacheDataStuff& cData, unsigned int index, unsigned char core);

    void AddToErrorMap(unsigned char err, gtVAddr RIPx, const char* mnemx);

    bool Initialize();

    Cache*          m_pCache;
    CLUMap*         m_pCacheUtilMap;
    CLUErrorMap*    m_pCacheErrors;
    CLUModInfoMap*  m_pModInfoMap;
    UINT64          m_totCacheBytes;
    UINT64          m_totCacheEvictions;
    UINT32          m_modIndex;
    UINT32          m_funcIndex;
    bool            m_initialized;
    UINT8           m_l1DcAssoc;
    UINT8           m_l1DcLineSize;
    UINT8           m_l1DcLinesPerTag;
    UINT8           m_l1DcSize;
};

#endif // _CLUINFO_H_
