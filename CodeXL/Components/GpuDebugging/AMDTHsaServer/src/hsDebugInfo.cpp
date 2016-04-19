//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsDebugInfo.cpp
///
//==================================================================================


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// HW Debugger + HwDbgFacilities:
#include <AMDGPUDebug.h>
#include <DbgInfoDwarfParser.h>
#include <FacilitiesInterface.h>

// Local:
#include <src/hsDebuggingManager.h>
#include <src/hsDebugInfo.h>
#include <src/hsStringConstants.h>

hsDebugInfo::hsDebugInfo(const void* pBinary, size_t binarySize, const void* kernelArgs)
    : m_hDbgInfo(nullptr), m_pKernelArgs(kernelArgs)
{
    HwDbgInfo_err err = HWDBGINFO_E_SUCCESS;
    HwDbgInfo_debug hDbgInfo = hwdbginfo_init_with_hsa_1_0_binary((void*)pBinary, binarySize, &err);
    GT_ASSERT(HWDBGINFO_E_SUCCESS == err);
    GT_IF_WITH_ASSERT(nullptr != hDbgInfo)
    {
        const char* pSrc = nullptr;
        size_t srcLen = 0;
        err = hwdbginfo_get_hsail_text(hDbgInfo, &pSrc, &srcLen);
        GT_IF_WITH_ASSERT((HWDBGINFO_E_SUCCESS == err) && (nullptr != pSrc) && (0 < srcLen))
        {
            determineSourceFilePath(pSrc, srcLen);

            osFile srcFile;
            bool rcFl = srcFile.open(m_sourceFilePath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
            GT_IF_WITH_ASSERT(rcFl)
            {
                rcFl = srcFile.write((gtByte*)pSrc, srcLen);
                GT_ASSERT(rcFl);
                srcFile.close();
            }
        }

        m_hDbgInfo = (void*)hDbgInfo;
    }

#ifdef HS_DUMP_BINARY
    {
        osFilePath binPath(osFilePath::OS_TEMP_DIRECTORY);
        binPath.setFileName(L"temp_kernel").setFileExtension(L"hsabin");
        osFile binFile;
        bool rcBn = binFile.open(binPath, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        GT_IF_WITH_ASSERT(rcBn)
        {
            rcBn = binFile.write((gtByte*)pBinary, binarySize);
            GT_ASSERT(rcBn);
            binFile.close();
        }
    }
#endif // HS_DUMP_BINARY
}

hsDebugInfo::~hsDebugInfo()
{
    osFile srcFile(m_sourceFilePath);
    srcFile.deleteFile();
}

bool hsDebugInfo::LineToAddrs(gtUInt64 line, gtVector<gtUInt64>& o_addrs) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_hDbgInfo)
    {
        HwDbgInfo_debug hDbgInfo = (HwDbgInfo_debug)m_hDbgInfo;
        HwDbgInfo_code_location locs[2] = {nullptr, nullptr};
        locs[0] = hwdbginfo_make_code_location(nullptr, (HwDbgInfo_linenum)line);
        HwDbgInfo_err err = hwdbginfo_nearest_mapped_line(hDbgInfo, locs[0], &(locs[1]));

        if (HWDBGINFO_E_SUCCESS == err)
        {
            size_t addrCount = 0;
            err = hwdbginfo_line_to_addrs(m_hDbgInfo, locs[1], 0, nullptr, &addrCount);

            if ((HWDBGINFO_E_SUCCESS == err) && (0 < addrCount))
            {
                gtVector<HwDbgInfo_addr> getAddrs(addrCount);
                err = hwdbginfo_line_to_addrs(m_hDbgInfo, locs[1], addrCount, &(getAddrs[0]), nullptr);

                if (HWDBGINFO_E_SUCCESS == err)
                {
                    o_addrs.resize(addrCount);

                    for (size_t i = 0; addrCount > i; ++i)
                    {
                        o_addrs[i] = getAddrs[i];
                    }

                    retVal = true;
                }
            }
        }

        hwdbginfo_release_code_locations(locs, 2);
    }

    return retVal;
}

bool hsDebugInfo::AddrToLine(gtUInt64 addr, gtUInt64& o_line) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_hDbgInfo)
    {
        HwDbgInfo_debug hDbgInfo = (HwDbgInfo_debug)m_hDbgInfo;
        HwDbgInfo_addr resolved = 0;
        HwDbgInfo_err err = hwdbginfo_nearest_mapped_addr(hDbgInfo, (HwDbgInfo_addr)addr, &resolved);

        if (HWDBGINFO_E_SUCCESS == err)
        {
            HwDbgInfo_code_location loc = nullptr;
            err = hwdbginfo_addr_to_line(hDbgInfo, resolved, &loc);

            if (HWDBGINFO_E_SUCCESS == err)
            {
                HwDbgInfo_linenum line = 0;
                err = hwdbginfo_code_location_details(loc, &line, 0, nullptr, nullptr);

                if (HWDBGINFO_E_SUCCESS == err)
                {
                    retVal = true;
                    o_line = (gtUInt64)line;
                }
            }

            hwdbginfo_release_code_locations(&loc, 1);
        }
    }

    return retVal;
}

bool hsDebugInfo::EvaluateVariable(const gtString& varName, gtString& o_varValue, gtString* o_pVarValueHex, gtString* o_pVarType) const
{
    bool retVal = false;

    osDebugLogSeverity debugLogSev = osDebugLog::instance().loggedSeverity();

    GT_IF_WITH_ASSERT(nullptr != m_hDbgInfo)
    {
        bool hexValueNeeded = (nullptr != o_pVarValueHex);
        HwDbgInfo_debug hDbgInfo = (HwDbgInfo_debug)m_hDbgInfo;
        HwDbgInfo_addr addr = (HwDbgInfo_addr)hsDebuggingManager::instance().GetCurrentAddress();
        HwDbgInfo_err err = HWDBGINFO_E_SUCCESS;
        HwDbgInfo_variable var = nullptr;

        if (!varName.isEmpty())
        {
            if ('$' == varName[0])
            {
                var = hwdbginfo_low_level_variable(hDbgInfo, addr, false, varName.asASCIICharArray(), &err);
            }
            else // '$' != varName[0]
            {
                var = hwdbginfo_variable(hDbgInfo, addr, false, varName.asASCIICharArray(), &err);
            }
        }

        if ((HWDBGINFO_E_SUCCESS == err) && (nullptr != var))
        {
            size_t varSize = 0;
            HwDbgInfo_encoding varEnc = HWDBGINFO_VENC_NONE;
            bool isConst = false;

            // Type information is handled after the rest of the function evaluation:
            size_t varTypeLen = 0;

            err = hwdbginfo_variable_data(var, 0, nullptr, nullptr, 0, nullptr, &varTypeLen, &varSize, &varEnc, &isConst, nullptr);

            if (HWDBGINFO_E_SUCCESS == err)
            {
                // No string format we will print is longer than 8 bytes:
                gtUByte varStorage[8] = { 0 };
                void* varValBuffer = (void*)varStorage;
                size_t usedSize = ((varSize > 8) || (varSize < 1)) ? 8 : varSize;
                bool isActiveWI = true;

                if (isConst)
                {
                    if (8 < varSize)
                    {
                        gtVector<gtUByte> val(varSize);
                        err = hwdbginfo_variable_const_value(var, varSize, &(val[0]));

                        if (HWDBGINFO_E_SUCCESS == err)
                        {
                            retVal = true;
                            ::memcpy(varValBuffer, &(val[0]), 8);
                        }
                    }
                    else // 8 >= varSize
                    {
                        err = hwdbginfo_variable_const_value(var, varSize, varValBuffer);

                        if (HWDBGINFO_E_SUCCESS == err)
                        {
                            retVal = true;
                        }
                    }
                }
                else // !isConst
                {
                    HwDbgInfo_locreg regType = 0;
                    unsigned int regNum = 0;
                    bool derefValue = false;
                    unsigned int offset = 0;
                    unsigned int resource = 0;
                    unsigned int isaMemoryRegion = 0;
                    unsigned int pieceOffset = 0;
                    unsigned int pieceSize = 0;
                    int constAdd = 0;

                    err = hwdbginfo_variable_location(var, &regType, &regNum, &derefValue, &offset, &resource, &isaMemoryRegion, &pieceOffset, &pieceSize, &constAdd);

                    if (HWDBGINFO_E_SUCCESS == err)
                    {
                        // 1. Get the base location:
                        static const size_t zero = 0;
                        const void* loc = NULL;
                        //  size_t locStride = 0;

                        /* offset for step 2 */
                        size_t totalOffset = 0;
                        retVal = true;

                        switch (regType)
                        {
                            case HwDbg::DwarfVariableLocation::LOC_REG_REGISTER:
                                // Not currently supported:
                                retVal = false;
                                break;

                            case HwDbg::DwarfVariableLocation::LOC_REG_STACK:
                                // Not currently supported:
                                retVal = false;
                                break;

                            case HwDbg::DwarfVariableLocation::LOC_REG_NONE:
                                loc = &zero;
                                break;

                            case HwDbg::DwarfVariableLocation::LOC_REG_UNINIT:
                                retVal = false;
                                break;

                            default:
                                GT_ASSERT(false);
                                break;
                        }

                        /* 2. Dereference and apply offset as needed: */
                        /* currently ignoring array offset */
                        totalOffset = offset;

                        if (derefValue && retVal)
                        {
                            retVal = false;
                            // Note: This assumes for dereferenced types that the values of the base pointer are all the same.
                            // A more "correct" method would be to iterate all the active work items, get the value for each,
                            // then copy that into a buffer.
                            // Cast to int since offset values may be signed (e.g. stack memory):
                            size_t realLocation = *((size_t*)loc) + (int)totalOffset + (int)pieceOffset;

                            // Since we applied the piece offset here (to get the correct value), we can reset the piece offset we will use to parse to 0:
                            pieceOffset = 0;

                            switch (isaMemoryRegion)
                            {
                                case 0: // = IMR_Global
                                    // Global Memory:
                                    ::memcpy(varValBuffer, ((void*)realLocation), usedSize);
                                    // valueStride = (unsigned int)var_size;
                                    retVal = true;
                                    break;

                                case 1: // = IMR_Scratch
                                {
                                    // Private memory:
                                    gtUInt32 workDims[9] = { 0 };
                                    hsDebuggingManager& theDebuggingMgr = hsDebuggingManager::instance();
                                    bool rcWI = theDebuggingMgr.GetActiveWorkItem(&(workDims[0]), &(workDims[3]), &(workDims[6]), isActiveWI);
                                    GT_ASSERT(rcWI);

                                    if (isActiveWI)
                                    {
                                        HwDbgDim3 focusWg = { workDims[6], workDims[7], workDims[8] };
                                        HwDbgDim3 focusWi = { workDims[3], workDims[4], workDims[5] };

                                        HwDbgContextHandle hDbg = (HwDbgContextHandle)theDebuggingMgr.GetDebugContextHandle();
                                        size_t outSize = 0;
                                        HwDbgStatus rcMem = HwDbgReadMemory(hDbg, isaMemoryRegion, focusWg, focusWi, (size_t)realLocation, usedSize, varValBuffer, &outSize);
                                        GT_ASSERT(usedSize == outSize);

                                        if (HWDBG_STATUS_SUCCESS == rcMem)
                                        {
                                            retVal = true;
                                        }
                                        else if (OS_DEBUG_LOG_DEBUG <= debugLogSev)
                                        {
                                            gtString errMsg;
                                            errMsg.appendFormattedString(HS_STR_debugLogCouldNotGetPrivateMemory, rcMem);
                                            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                                        }
                                    }
                                }
                                break;

                                case 2: // = IMR_Group
                                    // Local memory:
                                    // Not currently supported
                                    retVal = false;
                                    break;

                                case 3: // = IMR_ExtUserData
                                    // Uri, 21/05/13 - As a workaround to getting an ExtUserData (32-bit aligned OpenCL arguments buffer) pointer
                                    // And having to read from the AQL (64-bit aligned HSA arguments buffer), we need to double the offset here,
                                    // As this works properly for most parameters.
                                    // Should be revised (as parameters larger than 32-bit will not work) or removed when the compiler moves to AQL offsets.
                                    realLocation *= 2;

                                case 4: // = IMR_AQL
                                case 5: // = IMR_FuncArg

                                    // assume kernel argument is not NULL but value is 0?
                                    // Kernel arguments:
                                    // Add the offset to the kernel args base pointer:
                                    if (0 > (intptr_t)realLocation)
                                    {
                                        GT_ASSERT(0 <= (intptr_t)realLocation);
                                        realLocation = 0;
                                    }

                                    realLocation += (size_t)m_pKernelArgs;
                                    ::memcpy(varValBuffer, ((void*)realLocation), usedSize);
                                    retVal = true;
                                    break;

                                default:
                                    GT_ASSERT(false);
                                    break;
                            }
                        }
                        else
                        {
                            retVal = true;
                            void* variableValues = (void*)((size_t)loc + totalOffset + pieceOffset);

                            ::memcpy(varValBuffer, variableValues, usedSize);
                        }
                    }
                }

                if (retVal && isActiveWI)
                {
                    for (size_t i = usedSize; 8 > i; ++i)
                    {
                        varStorage[i] = (gtUByte)0;
                    }

                    o_varValue.makeEmpty();

                    switch (varEnc)
                    {
                        case HWDBGINFO_VENC_POINTER:
                            o_varValue.appendFormattedString(GT_64_BIT_POINTER_FORMAT_LOWERCASE, *(void**)varValBuffer);
                            break;

                        case HWDBGINFO_VENC_BOOLEAN:
                            o_varValue.appendFormattedString(L"%c", (*(bool*)varValBuffer) ? 'T' : 'F');
                            break;

                        case HWDBGINFO_VENC_FLOAT:
                            if (varSize == 8)
                            {
                                o_varValue.appendFormattedString(L"%g", *(double*)varValBuffer);
                            }
                            else
                            {
                                o_varValue.appendFormattedString(L"%g", *(float*)varValBuffer);
                            }

                            break;

                        case HWDBGINFO_VENC_INTEGER:
                            if (varSize < 2)
                            {
                                o_varValue.appendFormattedString(L"%d", *(gtByte*)varValBuffer);
                            }
                            else if (varSize < 4)
                            {
                                o_varValue.appendFormattedString(L"%d", *(gtInt16*)varValBuffer);
                            }
                            else if (varSize < 8)
                            {
                                o_varValue.appendFormattedString(L"%ld", *(gtInt32*)varValBuffer);
                            }
                            else
                            {
                                o_varValue.appendFormattedString(L"%lld", *(gtInt64*)varValBuffer);
                            }

                            break;

                        case HWDBGINFO_VENC_UINTEGER:
                            if (varSize < 2)
                            {
                                o_varValue.appendFormattedString(L"%u", *(gtUByte*)varValBuffer);
                            }
                            else if (varSize < 4)
                            {
                                o_varValue.appendFormattedString(L"%u", *(gtUInt16*)varValBuffer);
                            }
                            else if (varSize < 8)
                            {
                                o_varValue.appendFormattedString(L"%lu", *(gtUInt32*)varValBuffer);
                            }
                            else
                            {
                                o_varValue.appendFormattedString(L"%llu", *(gtUInt64*)varValBuffer);
                            }

                            break;

                        case HWDBGINFO_VENC_CHARACTER:
                        {
                            gtByte val = *(gtByte*)varValBuffer;
                            o_varValue.appendFormattedString(L"0x%02x (%c)", val, ((val > 0) ? val : 0));
                        }
                        break;

                        case HWDBGINFO_VENC_UCHARACTER:
                        {
                            gtUByte val = *(gtUByte*)varValBuffer;
                            o_varValue.appendFormattedString(L"0x%02x (%uc)", val, val);
                        }
                        break;

                        case HWDBGINFO_VENC_NONE:
                            o_varValue = L"N/A";
                            break;

                        default:
                            o_varValue = L"Unsupported format!";
                            GT_ASSERT(false);
                            break;
                    }

                    if (hexValueNeeded)
                    {
                        // Only these two encodings are different between hex and normal:
                        if ((HWDBGINFO_VENC_INTEGER == varEnc) || (HWDBGINFO_VENC_UINTEGER == varEnc))
                        {
                            if (varSize < 2)
                            {
                                o_pVarValueHex->appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_2_CHAR_FORMAT, *(gtUByte*)varValBuffer);
                            }
                            else if (varSize < 4)
                            {
                                o_pVarValueHex->appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_4_CHAR_FORMAT, *(gtUInt16*)varValBuffer);
                            }
                            else if (varSize < 8)
                            {
                                o_pVarValueHex->appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_8_CHAR_FORMAT, *(gtUInt32*)varValBuffer);
                            }
                            else
                            {
                                o_pVarValueHex->appendFormattedString(GT_UNSIGNED_INT_HEXADECIMAL_16_CHAR_FORMAT, *(gtUInt64*)varValBuffer);
                            }

                            hexValueNeeded = false;
                        }
                    }
                }
                else if (!isActiveWI)
                {
                    retVal = true;
                    o_varValue = HS_STR_variableInactiveWIPrivateMemory;
                }

#if AMDT_BUILD_TARGET == AMDT_DEBUG_BUILD
                else
                {
                    retVal = true;
                    o_varValue = L"DEBUG: variable found but variable values not obtained";
                }

#endif
            }
            else
            {
                retVal = true;
                o_varValue = HS_STR_variableEvalError;
            }

            if (retVal && (nullptr != o_pVarType))
            {
                bool gotType = false;

                // Get the type name:
                if (0 < varTypeLen)
                {
                    gtVector<char> varTypeBuf(varTypeLen + 1);
                    char* pVarType = &(varTypeBuf[0]);
                    err = hwdbginfo_variable_data(var, 0, nullptr, nullptr, varTypeLen, pVarType, nullptr, nullptr, nullptr, nullptr, nullptr);
                    GT_IF_WITH_ASSERT(HWDBGINFO_E_SUCCESS == err)
                    {
                        varTypeBuf[varTypeLen] = (char)0;
                        o_pVarType->fromASCIIString(pVarType);
                        gotType = !o_pVarType->isEmpty();
                    }
                }

                if (!gotType)
                {
                    // Create a type string from the encoding and size:
                    o_pVarType->makeEmpty();

                    switch (varEnc)
                    {
                        case HWDBGINFO_VENC_POINTER:    *o_pVarType = L"auto*";     break;

                        case HWDBGINFO_VENC_BOOLEAN:    *o_pVarType = L"bool";      break;

                        case HWDBGINFO_VENC_FLOAT:
                            if (varSize == 8) { *o_pVarType = L"double"; }
                            else { *o_pVarType = L"float"; }     break;

                        case HWDBGINFO_VENC_INTEGER:
                            if (varSize < 2) { *o_pVarType = L"int8"; }
                            else if (varSize < 4) { *o_pVarType = L"int16"; }
                            else if (varSize < 8) { *o_pVarType = L"int32"; }
                            else { *o_pVarType = L"int64"; }     break;

                        case HWDBGINFO_VENC_UINTEGER:
                            if (varSize < 2) { *o_pVarType = L"uint8"; }
                            else if (varSize < 4) { *o_pVarType = L"uint16"; }
                            else if (varSize < 8) { *o_pVarType = L"uint32"; }
                            else { *o_pVarType = L"uint64"; }    break;

                        case HWDBGINFO_VENC_CHARACTER:  *o_pVarType = L"char";      break;

                        case HWDBGINFO_VENC_UCHARACTER: *o_pVarType = L"uchar";     break;

                        case HWDBGINFO_VENC_NONE:       *o_pVarType = L"N/A";       break;

                        default:
                            *o_pVarType = L"N/A";
                            GT_ASSERT(false);
                            break;
                    }
                }
            }
        }
        else
        {
            retVal = true;
            o_varValue = HS_STR_variableNotFound;
        }

        if (hexValueNeeded)
        {
            *o_pVarValueHex = o_varValue;
        }

        hwdbginfo_release_variables(hDbgInfo, &var, 1);
    }

    if (OS_DEBUG_LOG_EXTENSIVE <= debugLogSev)
    {
        gtString logMsg = varName;
        logMsg.appendFormattedString(HS_STR_debugLogDebugInfoVarValue, retVal ? 'Y' : 'N').append(o_varValue);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE)
    }

    return retVal;
}

// Helper function for readability:
static inline bool hsLowLevelVariableExists(HwDbgInfo_debug hDbgInfo, HwDbgInfo_addr addr, const gtString& vNm)
{
    bool retVal = false;

    HwDbgInfo_variable v = hwdbginfo_low_level_variable(hDbgInfo, addr, false, vNm.asASCIICharArray(), nullptr);

    if (nullptr != v)
    {
        retVal = true;
        hwdbginfo_release_variables(hDbgInfo, &v, 1);
    }

    return retVal;
}

bool hsDebugInfo::ListVariables(gtVector<gtString>& o_variables) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_hDbgInfo)
    {
        HwDbgInfo_debug hDbgInfo = (HwDbgInfo_debug)m_hDbgInfo;
        HwDbgInfo_addr addr = (HwDbgInfo_addr)hsDebuggingManager::instance().GetCurrentAddress();
        size_t varCount = 0;
        HwDbgInfo_err err = hwdbginfo_frame_variables(hDbgInfo, addr, 0, false, 0, nullptr, &varCount);

        if ((HWDBGINFO_E_SUCCESS == err) && (0 < varCount))
        {
            gtVector<HwDbgInfo_variable> vars(varCount);

            for (auto& v : vars)
            {
                v = nullptr;
            }

            err = hwdbginfo_frame_variables(hDbgInfo, addr, 0, false, varCount, &(vars[0]), nullptr);

            if (HWDBGINFO_E_SUCCESS == err)
            {
                retVal = true;
                gtVector<char> nmAsChars;

                for (const auto& v : vars)
                {
                    size_t nmLen = 0;
                    err = hwdbginfo_variable_data(v, 0, nullptr, &nmLen, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

                    if ((HWDBGINFO_E_SUCCESS == err) && (0 < nmLen))
                    {
                        nmAsChars.resize(nmLen + 1);
                        err = hwdbginfo_variable_data(v, nmLen, &(nmAsChars[0]), nullptr, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
                        GT_IF_WITH_ASSERT(HWDBGINFO_E_SUCCESS == err)
                        {
                            nmAsChars[nmLen] = '\0';
                            gtString name;
                            name.fromASCIIString(&(nmAsChars[0]));
                            o_variables.push_back(name);
                        }
                    }
                }
            }

            hwdbginfo_release_variables(hDbgInfo, &(vars[0]), varCount);
        }

        // Also add HSA registers, which are considered low-level variables by the HwDbgFacilities:
        // Ranges are:
        //  - $c0..7
        //  - $s0..127
        //  - $d0..63
        //  - $q0..31

        gtString vNm;

        for (int i = 0; 127 > i; ++i)
        {
            // The only way to check for these variables is to see if they exist in the LL debug info (ISA DWARF):
            vNm.makeEmpty().appendFormattedString(L"$s%d", i);

            if (hsLowLevelVariableExists(hDbgInfo, addr, vNm))
            {
                o_variables.push_back(vNm);
            }

            if (64 <= i) { continue; }

            vNm.makeEmpty().appendFormattedString(L"$d%d", i);

            if (hsLowLevelVariableExists(hDbgInfo, addr, vNm))
            {
                o_variables.push_back(vNm);
            }

            if (32 <= i) { continue; }

            vNm.makeEmpty().appendFormattedString(L"$q%d", i);

            if (hsLowLevelVariableExists(hDbgInfo, addr, vNm))
            {
                o_variables.push_back(vNm);
            }

            if (8 <= i) { continue; }

            vNm.makeEmpty().appendFormattedString(L"$c%d", i);

            if (hsLowLevelVariableExists(hDbgInfo, addr, vNm))
            {
                o_variables.push_back(vNm);
            }
        }
    }

    if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
    {
        gtString logMsg = HS_STR_debugLogDebugInfoVarsList;
        logMsg.appendFormattedString(L"(%d)", (int)o_variables.size());

        for (const gtString& v : o_variables)
        {
            logMsg.append(',').append(' ').append(v);
        }

        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE)
    }

    return retVal;
}

bool hsDebugInfo::GetStepBreakpoints(bool stepOut, gtVector<gtUInt64>& o_bps) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_hDbgInfo)
    {
        HwDbgInfo_debug hDbgInfo = (HwDbgInfo_debug)m_hDbgInfo;
        HwDbgInfo_addr addr = (HwDbgInfo_addr)hsDebuggingManager::instance().GetCurrentAddress();

        size_t addrCount = 0;
        HwDbgInfo_err err = hwdbginfo_step_addresses(hDbgInfo, addr, stepOut, 0, nullptr, &addrCount);

        retVal = (HWDBGINFO_E_SUCCESS == err) ||
                 ((HWDBGINFO_E_NOTFOUND == err) && (0 == addrCount));
        GT_ASSERT(retVal);

        if ((HWDBGINFO_E_SUCCESS == err) && (0 < addrCount))
        {
            gtVector<HwDbgInfo_addr> stepBPs(addrCount);
            err = hwdbginfo_step_addresses(hDbgInfo, addr, stepOut, addrCount, &(stepBPs[0]), nullptr);
            retVal = (HWDBGINFO_E_SUCCESS == err);
            GT_IF_WITH_ASSERT(retVal)
            {
                o_bps.resize(addrCount);

                for (size_t i = 0; addrCount > i; ++i)
                {
                    o_bps[i] = stepBPs[i];
                }
            }
        }
    }

    return retVal;
}

bool hsDebugInfo::GetStepInBreakpoints(gtVector<gtUInt64>& o_bps) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_hDbgInfo)
    {
        HwDbgInfo_debug hDbgInfo = (HwDbgInfo_debug)m_hDbgInfo;
        size_t addrCount = 0;
        HwDbgInfo_err err = hwdbginfo_all_mapped_addrs(hDbgInfo, 0, nullptr, &addrCount);
        GT_IF_WITH_ASSERT((HWDBGINFO_E_SUCCESS == err) && (0 < addrCount))
        {
            gtVector<HwDbgInfo_addr> stepBPs(addrCount);
            err = hwdbginfo_all_mapped_addrs(hDbgInfo, addrCount, &(stepBPs[0]), nullptr);
            GT_IF_WITH_ASSERT(HWDBGINFO_E_SUCCESS == err)
            {
                o_bps.resize(addrCount);

                for (size_t i = 0; addrCount > i; ++i)
                {
                    o_bps[i] = stepBPs[i];
                }

                retVal = true;
            }
        }
    }

    return retVal;
}

void hsDebugInfo::determineSourceFilePath(const char* pSrc, size_t srcLen)
{
    gtASCIIString src(pSrc);
    GT_ASSERT(src.length() == (int)srcLen);

    // Check this project's temp folder to see if the source is already saved:
    bool foundSource = false;
    osFilePath baseDirPath = suCurrentProjectLogFilesDirectory();
    osDirectory baseDir(baseDirPath);
    gtList<osFilePath> hsailFilePaths;
    baseDir.getContainedFilePaths(HS_STR_hsailSourceFileExtensionWildcard, osDirectory::SORT_BY_DATE_DESCENDING, hsailFilePaths);

    for (const osFilePath& p : hsailFilePaths)
    {
        // Read the program:
        osFile programCodeFile;
        bool rc = programCodeFile.open(p, osChannel::OS_BINARY_CHANNEL, osFile::OS_OPEN_TO_READ);
        GT_ASSERT(rc);

        // Get the size of the program:
        unsigned long programSize = 0;
        rc = programCodeFile.getSize(programSize);
        GT_ASSERT(rc);

        // allocate a buffer:
        gtVector<char> programBuffer(programSize);

        // Read the program into the buffer:
        gtSize_t programRead;
        rc = programCodeFile.readAvailableData(&(programBuffer[0]), programSize, programRead);
        GT_ASSERT(rc);

        // Close the file:
        programCodeFile.close();

        if (&(programBuffer[0]) == src)
        {
            m_sourceFilePath = p;
            foundSource = true;
            break;
        }
    }

    // If this is a new source:
    if (!foundSource)
    {
        // Generate a new name for it:
        m_sourceFilePath = baseDirPath;
        m_sourceFilePath.setFileExtension(HS_STR_hsailSourceFileExtension);
        gtString fileName;
        bool fileExists = true;
        unsigned int programIndex = 0;

        while (fileExists)
        {
            fileName.makeEmpty().appendFormattedString(HS_STR_hsailSourceFileNameFormat, programIndex++);
            m_sourceFilePath.setFileName(fileName);

            fileExists = m_sourceFilePath.exists();
        };
    }
}

