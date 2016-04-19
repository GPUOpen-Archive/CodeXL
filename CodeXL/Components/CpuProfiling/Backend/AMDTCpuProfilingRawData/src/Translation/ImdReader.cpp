//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ImdReader.cpp
/// \brief Implementation of the ImdReader class.
///
//==================================================================================

#include "ImdReader.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

ImdReader::ImdReader(CpuProfileInfo* pProfileInfo) : m_imdVersion(0), m_pProfileInfo(pProfileInfo)
{
}


ImdReader::~ImdReader()
{
    close();
}


bool ImdReader::parseModAttributes(const gtString& line, CpuProfileModule& mod)
{
    bool bRet = true;

    if (line.find(L"BASE=") != -1)
    {
        unsigned long long tmp = 0ULL;
        gtString str;

        if (!section(str, line, L"=", 1) || !str.toUnsignedLongLongNumber(tmp))
        {
            bRet = false;
        }
        else
        {
            mod.m_base = static_cast<gtVAddr>(tmp);
        }
    }
    else if (line.find(L"SIZE=") != -1)
    {
        gtString str;

        if (!section(str, line, L"=", 1) || !str.toUnsignedIntNumber(mod.m_size))
        {
            bRet = false;
        }
    }
    else if (line.find(L"SAMPS=") != -1)
    {
        /* TBPVER9 and prior */
        // NOTE: We get the true total number from aggregation
        //      (Suravee)
        //mod.m_total = toULong(section (line, "=", 1));
    }
    else if (line.find(L"ModuleType=") != -1)
    {
        gtString str;

        if (!section(str, line, L"=", 1) || !str.toUnsignedIntNumber(mod.m_modType))
        {
            bRet = false;
        }
    }
    else if (line.find(L"LINECOUNT=") != -1)
    {
        /* TBPVER9 and prior */
        // TODO: Not Currently Used
    }
    else if (line.find(L"TOTAL_SAMPLES=") != -1)
    {
        /* TBPVER10+*/
        // TODO: Not Currently Used
    }
    else if (line.find(L"NUMSUBSECTIONS=") != -1)
    {
        /* TBPVER10+*/
        // TODO: Not Currently Used
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

/* TBP Version 9
[JIT_BEGIN]
JITSYM=call_stub
JITSRC=jit/21091/7f6924022369-0
JITBASE=7f6924022369
JITSIZE=0
LINECOUNT=2
5,0,1,1,[9 0] 1,0x7f69240223e8
5,0,1,1,[0 7] 1,0x7f692402243c
.....
[JIT_END]
*/
bool ImdReader::parseJitAttributes(const gtString& line, CpuProfileFunction& func)
{
    bool bRet = true;

    if (line.find(L"JITSYM=") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        func.setFuncName(str);
    }
    else if (line.find(L"JITSRC=") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        func.setJncFileName(str);
    }
    else if (line.find(L"JITBASE=") != -1)
    {
        gtString str;
        unsigned long long tmp = 0ULL;

        if (!section(str, line, L"=", 1) || !str.toUnsignedLongLongNumber(tmp))
        {
            bRet = false;
        }
        else
        {
            func.setBaseAddr(static_cast<gtVAddr>(tmp));
        }
    }
    else if (line.find(L"JITSIZE=") != -1)
    {
        // TODO: Not Currently Used
    }
    else if (line.find(L"LINECOUNT=") != -1)
    {
        // TODO: Not Currently Used
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

/* In TBPVER 10
[SUB]
BINARYFILE=<jnc file>
BASEADDR=<LoadAddr>
INSTANCE=<addr>:<size>:<symbol>
INSTANCE=<addr>:<size>:<symbol>
.....
LINECOUNT=<num of lines>
line1.....
line2.....
.....
[SUB_END]
*/

/* In TBPVER 12
[SUB]
BINARYFILE=<jnc file>
BASEADDR=<LoadAddr>
INSTANCE=<addr>:<size>:<symbol>
INSTANCE=<addr>:<size>:<symbol>
AGGREGATED=lineAgg1....
AGGREGATED=lineAgg2....
.....
LINECOUNT=<num of lines>
line1.....
line2.....
.....
[SUB_END]
*/

/* Baskar: Now in CodeXL INSTANCE tag is unused !!!
*/

bool ImdReader::parseSubAttributes(const gtString& line, CpuProfileFunction& func)
{
    bool bRet = true;

    if (line.find(L"BINARYFILE=") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        func.setJncFileName(str);
    }
    else if (line.find(L"SRCFILE=") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        func.setSourceFile(str);
    }
    else if (line.find(L"SRCLINE=") != -1)
    {
        gtString str;
        unsigned int tmp = 0ULL;

        if (!section(str, line, L"=", 1) || !str.toUnsignedIntNumber(tmp))
        {
            bRet = false;
        }
        else
        {
            func.setSourceLine(tmp);
        }
    }
    else if (line.find(L"BASEADDR=") != -1)
    {
        gtString str;
        unsigned long long tmp = 0ULL;

        if (!section(str, line, L"=", 1) || !str.toUnsignedLongLongNumber(tmp))
        {
            bRet = false;
        }
        else
        {
            func.setBaseAddr(static_cast<gtVAddr>(tmp));
        }
    }
    else if (line.find(L"TOPADDR=") != -1)
    {
        gtString str;
        unsigned long long tmp = 0ULL;

        if (!section(str, line, L"=", 1) || !str.toUnsignedLongLongNumber(tmp))
        {
            bRet = false;
        }
        else
        {
            func.setTopAddr(static_cast<gtVAddr>(tmp));
        }
    }
    else if (line.find(L"SYMBOL=") != -1)
    {
        gtString str;
        section(str, line, L"=", 1);
        func.setFuncName(str);
    }
    else if (line.find(L"AGGREGATED=") != -1)
    {
        int k;
        AptKey aptKey;
        AggregatedSample aggSamp;
        gtString inst;
        section(inst, line, L"=", 1);

        if (!processInstSampleLine(inst, aptKey, aggSamp, k))
        {
            return false;
        }

        func.addMetadataSample(aptKey, aggSamp);
    }
    else if (line.find(L"LINECOUNT=") != -1)
    {
        // TODO: Not Currently Used
    }
    else
    {
        bRet = false;
    }

    return bRet;
}


bool ImdReader::markSection(const gtString& name, fpos_t* pos)
{
    gtString line;

    if (getPos(name, pos))
    {
        return true;
    }

    while (readLine(line) > 0)
    {
        if (line[0] == L'[')
        {
            // Look for [ModuleName]
            if (line.find(name) != -1)
            {
                markPos(name);
                break;
            }
        }
    } // end of while loop;

    if (!getPos(name, pos))
    {
        return false;
    }

    return true;
}


bool ImdReader::processDataBlock(gtString& line, CpuProfileModule& mod, CpuProfileFunction& func)
{
    do
    {
        int k = 0;
        AptKey aptKey;
        AggregatedSample aggSamp;

        if ((line[0] >= L'0' || line[0] <= L'9') && processInstSampleLine(line, aptKey, aggSamp, k))
        {
            func.insertSample(aptKey, aggSamp);
            continue;
        }
        else
        {
            break;
        }
    }
    while (readLine(line) > 0);

    if (func.getTotal() > 0)
    {
        if (CpuProfileModule::JAVAMODULE == mod.getModType() || CpuProfileModule::MANAGEDPE == mod.getModType())
        {
            if (0 == func.getMetadataMapSize())
            {
                func.computeJavaAggregatedMetadata();
            }
        }

        mod.insertModDetailData(func.getBaseAddr(), func);
        func.clearSample();
    }

    return true;
}


bool ImdReader::processDataBlock_JavaTbp6(gtString& line, CpuProfileModule& mod)
{
    bool ret = false;
    CpuProfileFunction func;
    gtString javaSrcFile, jncFile, javaFuncName;
    gtVAddr jitBaseAddr = 0;

    do
    {
        AptKey aptKey;
        AggregatedSample aggSamp;

        if ((line[0] >= L'0' || line[0] <= L'9')
            && processInstSampleLine(line, aptKey, aggSamp,
                                     javaFuncName, jitBaseAddr,
                                     javaSrcFile, jncFile))
        {
            if (func.getBaseAddr() != jitBaseAddr)
            {
                if (func.getBaseAddr() != 0)
                {
                    // Insert function to Module
                    mod.insertModDetailData(func.getBaseAddr(), func);
                }

                // Clean up old function
                func.clearSample();

                // Setup new function
                func.setFuncName(javaFuncName);
                func.setJncFileName(jncFile);
                func.setSourceFile(javaSrcFile);
                func.setBaseAddr(jitBaseAddr);
            }

            // Aggregate data
            func.insertSample(aptKey, aggSamp);

            continue;
        }
        else
        {
            break;
        }
    }
    while (readLine(line) > 0);

    // Insert last function
    if (func.getBaseAddr() != 0)
    {
        mod.insertModDetailData(func.getBaseAddr(), func);
    }

    return ret;
}


/* TBP Version 9 */
bool ImdReader::processJitBlock(gtString& line, CpuProfileModule& mod)
{
    bool ret = false;
    CpuProfileFunction func;

    while (readLine(line) > 0)
    {
        if (line[0] >= L'0' && line[0] <= L'9')
        {
            // This always reads in an extra line
            processDataBlock(line, mod, func);
        }

        if (line.find(JIT_END) != -1)
        {
            ret = true;
            break;
        }

        if (parseJitAttributes(line, func))
        {
            continue;
        }

        break;
    }

    return ret;
}

/* TBPVER_10 */
bool ImdReader::processSubBlock(gtString& line, CpuProfileModule& mod)
{
    bool ret = false;
    CpuProfileFunction func;

    while (readLine(line) > 0)
    {
        if (line[0] >= L'0' && line[0] <= L'9')
        {
            // This always reads in an extra line
            processDataBlock(line, mod, func);
        }

        if (line.find(SUB_END) != -1)
        {
            ret = true;
            break;
        }

        if (parseSubAttributes(line, func))
        {
            continue;
        }

        break;
    }

    return ret;
}


bool ImdReader::readModSection(CpuProfileModule& mod)
{
    gtString line;
    fpos_t pos;
    bool ret = true;

    if (!isOpen())
    {
        return false;
    }

    gtString modSectName = L'[';
    modSectName += mod.getPath();
    modSectName += L']';

    if (!resetStream())
    {
        return false;
    }

    /* Find and get the beginning of section */
    if (!markSection(modSectName, &pos))
    {
        return false;
    }

    /* Go to the beginning of section */
    getCurrentPosition(&pos);

    CpuProfileFunction func;

    /* Start parsing attribute */
    while (readLine(line) > 0)
    {
        if (parseModAttributes(line, mod))
        {
            /* NOTE: In Version 7, the module baseaddr
             *       is in the ModAttribute. Therefore, we
             *       need to copy this into the Function
             */
            if (0 != mod.m_base && func.getBaseAddr() != mod.m_base)
            {
                func.setBaseAddr(mod.m_base);
            }

            continue;
        }
        else if (line.find(IMD_END) != -1)
        {
            ret = false;
            break;
        }
        else if (line[0] >= L'0' || line[0] <= L'9')     // Version 9
        {
            break;
        }
    }

    if (ret == false)
    {
        return ret;
    }

    /* Check module type */
    if ((m_pProfileInfo->m_tbpVersion <= 7) &&
        (mod.m_modType == CpuProfileModule::JAVAMODULE || mod.m_modType == CpuProfileModule::MANAGEDPE))
    {
        do
        {
            if ((line[0] >= L'0' || line[0] <= L'9'))
            {
                if (processDataBlock_JavaTbp6(line, mod))
                {
                    continue;
                }
            }
            else if (line.isEmpty() || line[0] == L'\0')
            {
                continue;
            }
            else if (line.find(IMD_END) != -1)
            {
                break;
            }
        }
        while (readLine(line) > 0);
    }
    else
    {
        do
        {
            if ((line.find(JIT_BEGIN) != -1)
                &&  processJitBlock(line, mod))   // Version 9
            {
                continue;
            }
            else if ((line.find(SUB_BEGIN) != -1)
                     &&  processSubBlock(line, mod))   // Version 10
            {
                continue;
            }
            else if ((line[0] >= L'0' || line[0] <= L'9')
                     && processDataBlock(line, mod, func))   // Version 9
            {
                continue;
            }
            else if (line.isEmpty() || line[0] == L'\0')
            {
                continue;
            }
            else if (line.find(IMD_END) != -1)
            {
                break;
            }
        }
        while (readLine(line) > 0);
    }

    if (CpuProfileModule::UNMANAGEDPE == mod.m_modType && 0 != mod.getNumSubSection())
    {
        AddrFunctionMultMap::const_iterator itFunc = mod.getEndFunction();
        --itFunc;

        const CpuProfileFunction& lastFunc = itFunc->second;

        if (0 == mod.m_base)
        {
            // NOTE [Suravee]:
            // In case of non-java module, we make assumption that
            // there is one [SUB] section.  Therefore, we can just
            // return the m_baseAddr of the first sub-section.
            // In case of Java module, this is not the case, so it
            // returns 0, and we need to get the m_baseAddr of the specified
            // JAVA function instead.
            mod.m_base = mod.getBeginFunction()->second.getBaseAddr();
        }

        CpuProfileFunction* pUnchartedFunc = mod.getUnchartedFunction();

        if (NULL != pUnchartedFunc)
        {
            gtString imageFile;

            if (mod.extractFileName(imageFile))
            {
                imageFile.append(L'!');
                pUnchartedFunc->setFuncName(imageFile);
            }
        }

        if (0 == mod.m_size)
        {
            if (NULL != pUnchartedFunc)
            {
                if (0 != pUnchartedFunc->getSampleMapSize())
                {
                    AptAggregatedSampleMap::const_iterator itSample = pUnchartedFunc->getEndSample();
                    --itSample;

                    mod.m_size = static_cast<gtUInt32>(itSample->first.m_addr) + 1U;
                }
            }

            if (&lastFunc != pUnchartedFunc)
            {
                gtUInt32 funcSize = lastFunc.getSize();

                if (0 == funcSize)
                {
                    if (0 != lastFunc.getSampleMapSize())
                    {
                        AptAggregatedSampleMap::const_iterator itSample = lastFunc.getEndSample();
                        --itSample;

                        funcSize = static_cast<gtUInt32>(itSample->first.m_addr) + 1U;
                    }
                }

                funcSize += static_cast<gtUInt32>(lastFunc.getBaseAddr() - mod.getBaseAddr());

                if (mod.m_size < funcSize)
                {
                    mod.m_size = funcSize;
                }
            }
        }
    }

    return ret;
}


/*
 * Description: This function process each line within [Each Module] section
 *
 * Format of each line in [Each Module] section of version 9 or higher:
 *   PID,TID,TOTALSAMPLE,#EVENTSET,[CPU INDEX] #SAMPLE,address
 */
bool ImdReader::processInstSampleLine(const gtString& line, AptKey& aptKey, AggregatedSample& aggSamp, int& k)
{
    unsigned int eventOrIndex = 0;
    gtString buffer;

    k = 0;

    if (line.find(L",") == -1)
    {
        return false;
    }

    // Get PID
    if (!parsePid(line, k, aptKey.m_pid))
    {
        return false;
    }

    // Get TID
    if (!parsePid(line, k, aptKey.m_tid))
    {
        return false;
    }

    // Note: We should not depend on this. Instead, we get this
    //       from aggregation. (Suravee)
    // Get TOTALSAMPLE
    //gtUInt32 total =  toULongLong(section(line, ",", k, k+1), 16);
    //func.m_total += total;
    k++;

    // pass the #eventset
    k++;

    // Get [CPU INDEX] #SAMPLE
    for (; section(buffer, line, L",", k, k + 1); k++)
    {
        SampleKey key;
        unsigned long count;

        if (buffer[0] != L'[')
        {
            break;
        }

        swscanf(buffer.asCharArray(), L"[%u %u] %lu", &key.cpu, &eventOrIndex, &count);

        if (eventOrIndex >= m_pProfileInfo->m_eventVec.size())
        {
            return false;
        }

        // This is index, convert it to event;
        key.event = m_pProfileInfo->m_eventVec[eventOrIndex].eventMask;
        aggSamp.insertSamples(key, count);
    }


    // if (aggSamp.getTotal() != total) {
    //     OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: total value is wrong:\nline: %ls", line.asCharArray());
    // }

    // Get address
    if (!parseVaddr(line, k, aptKey.m_addr))
    {
        return false;
    }

    return true;
}

/*
 * Description: This function process each line within [Each Module] section
 *
 * Format of each line in [Each Module] section of version 6 or higher:
 *   PID,TID,TOTALSAMPLE,#EVENTSET,[CPU INDEX] #SAMPLE,address,FUNCNAME,
 *          FUNCBASEADDR,SRCFILE,JNCFILE
 */
bool ImdReader::processInstSampleLine(const gtString& line,
                                      AptKey& aptKey,
                                      AggregatedSample& aggSamp,
                                      gtString& javaFuncName,
                                      gtVAddr& jitBaseAddr,
                                      gtString& javaSrcFile,
                                      gtString& jncFile)
{
    int k = 0;

    if (!processInstSampleLine(line, aptKey, aggSamp, k))
    {
        return false;
    }

    // The rest only for Java TBP version 7
    if (m_pProfileInfo->m_tbpVersion > 7)
    {
        return true;
    }

    // Get Java function name
    if (!parseWstring(line, k, javaFuncName))
    {
        return false;
    }

    // Get Java jit baseaddr
    if (!parseVaddr(line, k, jitBaseAddr))
    {
        return false;
    }

    // Get Java src file
    if (!parseWstring(line, k, javaSrcFile))
    {
        return false;
    }

    // Get Java src file
    if (!parseWstring(line, k, jncFile))
    {
        return false;
    }

    return true;

}
