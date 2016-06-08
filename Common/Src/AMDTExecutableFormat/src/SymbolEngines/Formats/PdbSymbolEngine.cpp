//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PdbSymbolEngine.cpp
/// \brief This file contains the class for querying PDB symbols.
///
//==================================================================================

#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtHashSet.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCriticalSectionLocker.h>
#include <AMDTOSWrappers/Include/osAtomic.h>
#include "PdbSymbolEngine.h"
#include "PeFile.h"
#pragma warning( push )
#pragma warning( disable : 4091)
#include <dbghelp.h>
#pragma warning( pop )
#include <diacreate.h>

#define UNDNAME_IA_STYLE   (UNDNAME_NO_LEADING_UNDERSCORES  |  \
                            UNDNAME_NO_MS_KEYWORDS          |  \
                            UNDNAME_NO_FUNCTION_RETURNS     |  \
                            UNDNAME_NO_ALLOCATION_MODEL     |  \
                            UNDNAME_NO_ALLOCATION_LANGUAGE  |  \
                            UNDNAME_NO_THISTYPE             |  \
                            UNDNAME_NO_ACCESS_SPECIFIERS    |  \
                            UNDNAME_NO_THROW_SIGNATURES     |  \
                            UNDNAME_NO_MEMBER_TYPE          |  \
                            UNDNAME_NO_RETURN_UDT_MODEL     |  \
                            UNDNAME_32_BIT_DECODE)

// No need to define these DiaSourceXXX for each new DIA version
// We can use DiaSource or DiaSourceAlt directly defined in DIA header
//class DECLSPEC_UUID("E60AFBEE-502D-46AE-858F-8272A09BD707") DiaSource71;
//class DECLSPEC_UUID("BCE36434-2C24-499E-BF49-8BD99B0EEB68") DiaSource80;
//class DECLSPEC_UUID("4C41678E-887B-4365-A09E-925D28DB33C2") DiaSource90;
//class DECLSPEC_UUID("B86AE24D-BF2F-4ac9-B5A2-34B14E4CE11D") DiaSource100;
//class DECLSPEC_UUID("761D3BCD-1304-41D5-94E8-EAC54E4AC172") DiaSource110;
//class DECLSPEC_UUID("3BFCEA48-620F-4B6B-81F7-B9AF75454C7D") DiaSource120;
//class DECLSPEC_UUID("E6756135-1E65-4D17-8576-610761398C3C") DiaSource140;

#define CV_SIGNATURE_NB10   '01BN'
#define CV_SIGNATURE_RSDS   'SDSR'

// CodeView header
struct CV_HEADER
{
    DWORD CvSignature; // NBxx
    LONG  Offset;      // Always 0 for NB10
};

// CodeView NB10 debug information
// (used when debug information is stored in a PDB 2.00 file)
struct CV_INFO_PDB20
{
    CV_HEADER  Header;
    DWORD      Signature;       // seconds since 01.01.1970
    DWORD      Age;             // an always-incrementing value
    BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file
};

// CodeView RSDS debug information
// (used when debug information is stored in a PDB 7.00 file)
struct CV_INFO_PDB70
{
    DWORD      CvSignature;
    GUID       Signature;       // unique identifier
    DWORD      Age;             // an always-incrementing value
    BYTE       PdbFileName[1];  // zero terminated string with the name of the PDB file
};

struct CV_DEBUG_INFO
{
    union
    {
        CV_HEADER normal;
        CV_INFO_PDB20 pdb20;
        CV_INFO_PDB70 pdb70;
    };
};

struct LimitsBundle
{
    gtUInt16 m_begin;
    gtUInt16 m_end;
};

static wchar_t* ExtractDemangledName(IDiaSymbol* pSymbol);
static bool IsDemangledName(const wchar_t* pName, unsigned len);
static size_t CountPaths(const wchar_t* pPathList, LimitsBundle* pPathListLimits, unsigned sizePathListLimits, size_t& len);
static const wchar_t* TrimPathDelimiters(const wchar_t* pPath, size_t& len);
static wchar_t* ConcatenateServerPath(wchar_t* pBuf,
                                      const wchar_t* pCache, size_t lenCache,
                                      const wchar_t* pServer, size_t lenServer);
static void ConcatenateServerPaths(wchar_t* pBuf,
                                   const wchar_t* pCache, size_t lenCache,
                                   const wchar_t* pServers,
                                   const LimitsBundle* pLimits, unsigned numLimits);
static wchar_t* BuildSearchPath(wchar_t* pBaseBuf,
                                size_t lenBaseBuf,
                                const wchar_t* pSearchPath,
                                const wchar_t* pServerList,
                                const wchar_t* pCachePath);

/// -----------------------------------------------------------------------------------------------
/// \brief Searches the file name, without the file's extension, in a given path.
///
/// \param[in] pPath The path with a file for the search.
/// \param[in,out] len The length, in characters, of the path. Set to the length of the found file name.
///
/// \return A pointer inside \a pPath, to the beginning of the actual file name.
/// -----------------------------------------------------------------------------------------------
template <typename TChar>
static const TChar* FindFileName(const TChar* pPath, size_t& len)
{
    // The beginning of the default file name is the beginning of the path (as the path might contain only the file name).
    const TChar* pFileNameBegin = pPath;

    // Set the ending to the file name to invalid (meaning that is not found yet).
    const TChar* pFileNameEnd = NULL;

    // Iterate over the path's string's characters.
    const TChar* pCh = pPath;

    for (; 0 != *pCh; ++pCh)
    {
        // If the current character is a directory separator (may it be '\\' or '/').
        if (((TChar)('\\')) == *pCh || ((TChar)('/')) == *pCh)
        {
            // Reset the beginning of the file name to the next character's position,
            // as we can trim all that is behind the current position, because it is part of the path's directory.
            pFileNameBegin = pCh + 1;
        }
        // If the current character is a file extension separator ('.').
        else if (((TChar)('.')) == *pCh)
        {
            // Reset the ending of the file name to the current character's position,
            // as we can trim all that is after the current position, because it is part of the file's extension.
            pFileNameEnd = pCh;
        }
    }

    // If the end of file name has not been set, meaning that it is 0 (NULL),
    // or if it has been set to a position before the beginning of the file's name,
    // then the file does not have an extension.
    if (pFileNameEnd <= pFileNameBegin)
    {
        // Set the end of the file name to pCh, which points to the end of the path's string,
        // as we just finished iterating over the string's characters.
        pFileNameEnd = pCh;
    }

    // The lengths of the string is the distance between the beginning of the file name and then ending (by using pointer arithmetics).
    len = pFileNameEnd - pFileNameBegin;

    // Return the pointer to the beginning of the found file name.
    return pFileNameBegin;
}


PdbSymbolEngine::PdbSymbolEngine() :
    m_pSession(NULL),
    m_pEnumSymbols(NULL),
    m_pEnumFrames(NULL),
    m_pPortableExe(NULL),
    m_needToCallCoUninitialize(false)
{
}

PdbSymbolEngine::~PdbSymbolEngine()
{
    // Clear functions info, if there is any.
    ClearFunctionsInfo();

    // Close the symbol engine, if opened.
    Close();
}

void PdbSymbolEngine::Close()
{
    if (!m_symbolsCache.empty())
    {
        for (gtMap<gtRVAddr, IDiaSymbol*>::iterator it = m_symbolsCache.begin(), itEnd = m_symbolsCache.end(); it != itEnd; ++it)
        {
            it->second->Release();
        }

        m_symbolsCache.clear();
    }

    if (!m_framesCache.empty())
    {
        for (gtMap<gtRVAddr, IDiaFrameData*>::iterator it = m_framesCache.begin(), itEnd = m_framesCache.end(); it != itEnd; ++it)
        {
            it->second->Release();
        }

        m_framesCache.clear();
    }

    ClearInlinedFunctionInfo();

    // If m_pEnumFrames has been set.
    if (NULL != m_pEnumFrames)
    {
        // Release the COM object.
        m_pEnumFrames->Release();

        // Invalidate the pointer.
        m_pEnumFrames = NULL;
    }

    // If m_pEnumSymbols has been set.
    if (NULL != m_pEnumSymbols)
    {
        // Release the COM object.
        m_pEnumSymbols->Release();

        // Invalidate the pointer.
        m_pEnumSymbols = NULL;
    }

    // If m_pSession has been set.
    if (NULL != m_pSession)
    {
        // Release the COM object.
        m_pSession->Release();

        // Invalidate the pointer.
        m_pSession = NULL;
    }

    // Uninitialize the COM library. (Should always be paired with a call to CoInitialize().)
    if (m_needToCallCoUninitialize)
    {
        CoUninitialize();
        m_needToCallCoUninitialize = false;
    }
}

void PdbSymbolEngine::ClearFunctionsInfo()
{
    m_funcsLock.lockWrite();

    // Iterate over all the functions in the m_funcsInfo map.
    for (FunctionsMap::iterator it = m_funcsInfo.begin(), itEnd = m_funcsInfo.end(); it != itEnd; ++it)
    {
        // If the function has a name.
        if (NULL != it->first.m_pName)
        {
            // Free the name's string memory with SysFreeString() as it has been allocated by SysAllocString().
            SysFreeString(it->first.m_pName);
        }
    }

    // Clear the map as it now contains only PODs.
    m_funcsInfo.clear();

    m_funcsLock.unlockWrite();
}

void PdbSymbolEngine::ClearInlinedFunctionInfo()
{
    gtHashSet<IDiaLineNumber*> tempLines;

    // m_inlineeLinesCache is map of one-to-many type. Multiple keys are mapped to same value.
    // Hence values are duplicated. Sequentially iterated free of value objects will lead to double-free error.
    // Use tempLines set to collect unique DIA object pointers and then free all.
    for (auto& it : m_inlineeLinesCache)
    {
        tempLines.insert(it.second.pLine);
    }

    m_inlineeLinesCache.clear();


    // Add the pending pLine objects to be released
    for (auto it : m_pendingLineObjectsToRelease)
    {
        tempLines.insert(it);
    }

    m_pendingLineObjectsToRelease.clear();

    // release the collected unique DIA objects
    for (auto& it : tempLines)
    {
        it->Release();
    }

    tempLines.clear();


    // clear m_inlinedFuncsInfo here
    // Iterate over all the functions in the m_inlinedFuncsInfo map.
    for (auto& it : m_inlinedFuncsInfo)
    {
        // If the function has a name.
        if (NULL != it.first.m_pName)
        {
            // Free the name's string memory with SysFreeString() as it has been allocated by SysAllocString().
            SysFreeString(it.first.m_pName);
        }
    }

    // Clear the map as it now contains only PODs.
    m_funcsInfo.clear();

    // clear m_nestedFuncMap here
    for (auto& it : m_nestedFuncMap)
    {
        it.second.clear();
    }

    m_nestedFuncMap.clear();
}

bool PdbSymbolEngine::Initialize(const PeFile& pe,
                                 const IMAGE_DEBUG_DIRECTORY* pDebugDirs,
                                 unsigned debugDirsCount,
                                 const wchar_t* pSearchPath,
                                 const wchar_t* pServerList,
                                 const wchar_t* pCachePath)
{
    // Reserve a local buffer of 2048 characters, as it will be enough in the most cases.
    const size_t lenLocalBuf = 2048;
    wchar_t localBuf[lenLocalBuf];

    // Build a single search path from the 3 given strings: pSearchPath, pServerList, pCachePath.
    // We provide a local buffer for the full search path, if the BuildSearchPath() function may choose to use it,
    // otherwise, we may get a dynamically allocated string, which we need to, eventually, free.
    wchar_t* pFullSearchPath = BuildSearchPath(localBuf, lenLocalBuf, pSearchPath, pServerList, pCachePath);

    // Initialize the pointer to the PE's of which we initialize the symbol engine.
    m_pPortableExe = &pe;

    m_processInlineSamples = m_pPortableExe->IsProcessInlineInfo();
    m_aggregateInlineSamples = m_pPortableExe->IsAggregateInlinedInstances();

    // Assume failure, as success is only determined later.
    bool ret = false;

    // Try to open a session for querying the symbols within the PDB file of the given PE.
    if (OpenPdbFile(pDebugDirs, debugDirsCount, pFullSearchPath, pSearchPath))
    {
        // Clear all previous functions info, as we are about to initialize it.
        ClearFunctionsInfo();

        // Retrieve a reference to the global scope.
        IDiaSymbol* pGlobalScope;
        HRESULT hr = m_pSession->get_globalScope(&pGlobalScope);

        if (S_OK == hr)
        {
            // If successfully found function symbols, or the public symbols that reside in the PE's code sections.
            if (FindFunctionSymbols(pGlobalScope) || FindPublicCodeSymbols(pGlobalScope))
            {
                ret = (S_OK == m_pSession->getSymbolsByAddr(&m_pEnumSymbols));
            }

            // Release the COM object.
            pGlobalScope->Release();
        }
    }

    // If the search path pointer is valid and not the local buffer.
    if (localBuf != pFullSearchPath && NULL != pFullSearchPath)
    {
        // Free the dynamically allocated string.
        delete [] pFullSearchPath;
    }

    return ret;
}

bool PdbSymbolEngine::OpenPdbFile(const IMAGE_DEBUG_DIRECTORY* pDebugDirs,
                                  unsigned debugDirsCount,
                                  const wchar_t* pRemoteSearchPath,
                                  const wchar_t* pLocalSearchPath)
{
    // Make sure the symbol engine is closed, before trying to open.
    // using the m_needToCallCoUninitialize it will not call CoUninitialize the first time and close Com
    // initialized by Qt
    Close();

    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    m_needToCallCoUninitialize = true;

    // If the result is S_OK - the COM library was initialized successfully on this thread;
    // or if the result is S_FALSE - the COM library is already initialized on this thread.
    if (SUCCEEDED(hr))
    {
        //
        // Create an instance of an interface for initiating access to a source of debugging symbols.
        // Try creating the instance from top down by version.
        //

        IDiaDataSource* pDataSource = nullptr;


        //
        // Try to create by using Microsoft DIA from Microsoft Visual Studio 2013 (v12.0).
        //

        wchar_t msdiaPath[OS_MAX_PATH];
        const wchar_t* pModulePath = GetExecutableFormatModulePath();
        size_t lenPath = wcslen(pModulePath);

        const wchar_t* pFileName = L"msdia140.dll";
        const size_t lenFileName = wcslen(pFileName);

        if ((lenPath + lenFileName) < OS_MAX_PATH)
        {
            memcpy(msdiaPath, pModulePath, lenPath * sizeof(wchar_t));
            memcpy(msdiaPath + lenPath, pFileName, lenFileName * sizeof(wchar_t));
            msdiaPath[lenPath + lenFileName] = L'\0';

            //hr = NoRegCoCreate(msdiaPath, __uuidof(DiaSource140), __uuidof(IDiaDataSource), reinterpret_cast<LPVOID*>(&pDataSource));
            hr = NoRegCoCreate(msdiaPath, __uuidof(DiaSourceAlt), __uuidof(IDiaDataSource), (void**)&pDataSource);
        }
        else
        {
            hr = E_FAIL;
        }

        // If we have successfully created the debugging symbols' access interface.
        if (S_OK == hr && nullptr != pDataSource)
        {
            // Open and prepare the debug data associated with the executable file.
            hr = pDataSource->loadDataForExe(m_pPortableExe->GetFilePath(), pRemoteSearchPath, NULL);

            // If failed to open the matching PDB file.
            if (S_OK != hr)
            {
                // Try to load the PDB file assuming the executable file and the PDB have been renamed using the same name.
                hr = LoadRenamedPdbFile(pDataSource, pDebugDirs, debugDirsCount, m_pPortableExe->GetFilePath(), pLocalSearchPath);
            }

            // If the debug data associated with the executable file has been loaded successfully.
            if (S_OK == hr)
            {
                // Open a session for querying symbols.
                hr = pDataSource->openSession(&m_pSession);

                if (S_OK == hr)
                {
                    // Get the PE's load address.
                    gtVAddr loadAddress = m_pPortableExe->GetLoadAddress();

                    // If the load address is valid.
                    if (GT_INVALID_VADDR != loadAddress)
                    {
                        // Set the load address for the executable file that corresponds to the symbols in this symbol store.
                        m_pSession->put_loadAddress(loadAddress);
                    }

                    // Set the enumerator of various frame data elements contained in the data source,
                    // for later use by the Call-Stack Sampling engine.
                    m_pEnumFrames = FindEnumFrameData();
                }
            }

            // Release the COM object.
            pDataSource->Release();
        }

        // If the opening of the PDB file was unsuccessful.
        if (S_OK != hr)
        {
            // Uninitialize the COM library. (Should always be paired with a call to CoInitialize().)
            if (m_needToCallCoUninitialize)
            {
                CoUninitialize();
                m_needToCallCoUninitialize = false;
            }
        }
    }

    // Return true if there was no failure.
    return S_OK == hr;
}

HRESULT PdbSymbolEngine::LoadRenamedPdbFile(IDiaDataSource* pDataSource,
                                            const IMAGE_DEBUG_DIRECTORY* pDebugDirs,
                                            unsigned debugDirsCount,
                                            const wchar_t* pExePath,
                                            const wchar_t* pSearchPath)
{
    // Assume failure, unless we have successfully loaded the PDB file.
    HRESULT hr = E_FAIL;

    // Iterate over the PE's debug directories.
    for (unsigned i = 0; i < debugDirsCount; ++i, ++pDebugDirs)
    {
        // Microsoft DIA only supports CodeView debug information.
        if (IMAGE_DEBUG_TYPE_CODEVIEW == pDebugDirs->Type)
        {
            // Translate the debug directory data's offset to an actual pointer within the mapped PE.
            const CV_DEBUG_INFO* pDebugInfo =
                reinterpret_cast<const CV_DEBUG_INFO*>(m_pPortableExe->m_pFileBase + pDebugDirs->PointerToRawData);

            // Microsoft DIA loadAndValidateDataFromPdb() function only supports CodeView RSDS debug information.
            if (CV_SIGNATURE_RSDS == pDebugInfo->normal.CvSignature)
            {
                //
                // Compare the file name of the hard coded matching PDB, withing the PE's debug information,
                // with the file name of the referenced executable.
                // If they are the same, then the assumption that the executable's file has been renamed, is incorrect,
                // and only try to actually load the PDB otherwise.
                //


                // Get the hard coded matching PDB file name.
                const char* pPdbFileName = reinterpret_cast<const char*>(pDebugInfo->pdb70.PdbFileName);

                // Calculate the length of the hard coded matching PDB file name.
                size_t lenPdbFileName = strlen(pPdbFileName);

                // Extract the actual file name with the file's extension (and trim the directories).
                pPdbFileName = FindFileName(pPdbFileName, lenPdbFileName);

                // Calculate the length of the referenced executable path.
                size_t lenExeFileName = wcslen(pExePath);

                // Extract the actual file name with the file's extension (and trim the directories).
                const wchar_t* pExeFileName = FindFileName(pExePath, lenExeFileName);

                // If the lengths of the names are the same.
                if (lenExeFileName == lenPdbFileName)
                {
                    // Convert the executable's file name to multi-byte, so we may be able to compare it with the PDB file name.
                    char mbExeFileName[OS_MAX_FNAME];

                    if (0 != WideCharToMultiByte(CP_ACP, 0, pExeFileName, static_cast<int>(lenExeFileName), mbExeFileName, OS_MAX_FNAME, NULL, NULL))
                    {
                        // If the names are the equal.
                        if (0 == memcmp(mbExeFileName, pPdbFileName, lenExeFileName))
                        {
                            // Stop searching, as the executable has not been renamed to a different name than the hard coded PDB file name.
                            break;
                        }
                    }
                }

                // Reserve a buffer for the full path of the renamed PDB file.
                wchar_t renamedPdbPath[OS_MAX_PATH];

                // Set the ending of the path to ".pdb", as this suffix will never change in our search.
                renamedPdbPath[OS_MAX_PATH - 1] = L'\0';
                renamedPdbPath[OS_MAX_PATH - 2] = L'b';
                renamedPdbPath[OS_MAX_PATH - 3] = L'd';
                renamedPdbPath[OS_MAX_PATH - 4] = L'p';
                renamedPdbPath[OS_MAX_PATH - 5] = L'.';

                // Prepend (to the file's extension) the renamed executable's file name.
                size_t posRenamedPdbPath = OS_MAX_PATH - 5 - lenExeFileName;
                memcpy(renamedPdbPath + posRenamedPdbPath, pExeFileName, lenExeFileName * sizeof(wchar_t));

                // Prepend the file's name with a directory separator, for later adding a directory path.
                renamedPdbPath[posRenamedPdbPath - 1] = L'\\';


                // The next candidate search path.
                const wchar_t* pCandidatePath = pSearchPath;

                // The currently examined search directory path.
                // First initialized to the directory in which the executable resides.
                const wchar_t* pSearchDir = pExePath;

                // The length of the currently examined search directory path.
                // First initialized to the distance between the pointer to the file's name and the beginning of the path (using pointer arithmetics).
                size_t lenSearchDir = pExeFileName - pExePath;

                for (;;)
                {
                    // If the search directory path's string is not empty.
                    if (0 != lenSearchDir)
                    {
                        // Get the last character in the search directory path.
                        wchar_t c = pSearchDir[lenSearchDir - 1];

                        // Truncate the last directory separator, if there is one, as we have already added it (before the 'while' loop).
                        lenSearchDir -= static_cast<size_t>('\\' == c || '/' == c);

                        // Prepend the search directory to renamed file name.
                        wchar_t* pRenamedPdbPath = renamedPdbPath + (posRenamedPdbPath - 1 - lenSearchDir);
                        memcpy(pRenamedPdbPath, pSearchDir, lenSearchDir * sizeof(wchar_t));

                        // Open and verify that the PDB file matches the signature information provided,
                        // and prepare the file as a debug data source.
                        hr = pDataSource->loadAndValidateDataFromPdb(pRenamedPdbPath,
                                                                     const_cast<GUID*>(&pDebugInfo->pdb70.Signature),
                                                                     pDebugInfo->pdb70.CvSignature,
                                                                     pDebugInfo->pdb70.Age);

                        // If the PDB has been successfully loaded.
                        if (S_OK == hr)
                        {
                            // Stop any further processing.
                            break;
                        }
                    }

                    // If there is no candidate path.
                    if (NULL == pCandidatePath)
                    {
                        // Stop any further processing, as we have no more directories to search in.
                        break;
                    }

                    // Update the search directory path to the beginning of the candidate path.
                    // (We still need to calculate the length of the directory path.)
                    pSearchDir = pCandidatePath;

                    // Find the next path separator to determine the next candidate path.
                    pCandidatePath = wcschr(pCandidatePath, L';');

                    // If a path separator has been found.
                    if (NULL != pCandidatePath)
                    {
                        // The length of the search directory path is the distance between the ending of the search directory path
                        // (which is the location of the next path separator) and the beginning of the search directory path
                        // (using pointer arithmetics).
                        lenSearchDir = pCandidatePath - pSearchDir;

                        // Skip the actual path separator.
                        pCandidatePath++;
                    }
                    else
                    {
                        // Because there is no path separator found, the search directory path is actually the last path in the list.
                        // Therefore, calculate the length of this string.
                        lenSearchDir = wcslen(pSearchDir);
                    }
                }

                // If the PDB has been successfully loaded.
                if (S_OK == hr)
                {
                    // Stop, as we are already done.
                    break;
                }
            }
        }
    }

    return hr;
}

bool PdbSymbolEngine::FindFunctionSymbols(IDiaSymbol* pGlobalScope)
{
    bool ret = false;

    IDiaEnumSymbols* pEnumSymbols;

    if (S_OK == m_pSession->findChildren(pGlobalScope, SymTagFunction, NULL, nsNone, &pEnumSymbols))
    {
        ULONG num;
        IDiaSymbol* pSymbol;

        if (S_OK == pEnumSymbols->Next(1UL, &pSymbol, &num))
        {
            FunctionSymbolInfo funcInfo;
            ConstructFunctionSymbolInfo(funcInfo, pSymbol, true);

            m_funcsInfo.insert(FunctionsMap::value_type(funcInfo, false));

            pSymbol->Release();

            ret = true;
        }

        pEnumSymbols->Release();
    }

    return ret;
}

bool PdbSymbolEngine::FindPublicCodeSymbols(IDiaSymbol* pGlobalScope)
{
    bool ret = false;

    IDiaEnumSymbols* pEnumSymbols;

    if (S_OK == m_pSession->findChildren(pGlobalScope, SymTagPublicSymbol, NULL, nsNone, &pEnumSymbols))
    {
        ULONG num;
        IDiaSymbol* pSymbol;

        while (S_OK == pEnumSymbols->Next(1UL, &pSymbol, &num))
        {
            DWORD rva = 0;

            if (S_OK == pSymbol->get_relativeVirtualAddress(&rva) &&
                m_pPortableExe->IsCodeSection(m_pPortableExe->LookupSectionIndex(static_cast<gtRVAddr>(rva))))
            {
                FunctionSymbolInfo funcInfo;

                // Baskar: FIXME: should we add the inlined children for this symbol?
                bool addInlinedChild = false;
                ConstructFunctionSymbolInfo(funcInfo, pSymbol, addInlinedChild);

                m_funcsInfo.insert(FunctionsMap::value_type(funcInfo, false));

                ret = true;
                pSymbol->Release();
                break;
            }

            pSymbol->Release();
        }

        pEnumSymbols->Release();
    }

    return ret;
}

bool PdbSymbolEngine::ProcessInlinedFunction(IDiaSymbol* pFunc) const
{
    bool ret = false;

    if (NULL != pFunc)
    {
        BOOL wasInlined;

        // check if the current function is inlined
        if (S_OK == pFunc->get_wasInlined(&wasInlined) && TRUE == wasInlined)
        {
            DWORD funcRva;
            ULONGLONG funcLen = 0;
            ULONG cnt = 0;

            if (S_OK == pFunc->get_relativeVirtualAddress(&funcRva) && S_OK == pFunc->get_length(&funcLen))
            {
                // find the source lines only for current inline function
                IDiaEnumLineNumbers* pLines;

                if (S_OK == m_pSession->findLinesByRVA(funcRva, static_cast<DWORD>(funcLen), &pLines))
                {
                    FunctionSymbolInfo funcInfo;
                    ConstructFunctionSymbolInfo(funcInfo, pFunc, false);

                    IDiaLineNumber* pLine = NULL;
                    IDiaSourceFile* pSourceFile = NULL;

                    // to hold the inline function RVA of the first line inlined
                    // this RVA is different than the actual function RVA as function epilogue is not considered for expansion
                    gtRVAddr inlineFuncRva = GT_INVALID_RVADDR;

                    // to hold the size of the only inlined bytes of inline function
                    // this size is different than the actual function size, as function epilogue/prologue is not included here
                    gtUInt32 inlineFuncSize = 0;

                    // iterate through the lines of current inline function
                    while (S_OK == pLines->Next(1, &pLine, &cnt))
                    {
                        // flag indicates if pLine object should be released or not within the while loop
                        bool freeLineObj = true;

                        if (NULL == pSourceFile && S_OK != pLine->get_sourceFile(&pSourceFile))
                        {
                            // unable to fetch the source file details from current line
                            pLine->Release();
                            continue;
                        }

                        DWORD lineNum;
                        IDiaSymbol* pCompiland;

                        if (S_OK == pLine->get_lineNumber(&lineNum) && S_OK == pLine->get_compiland(&pCompiland))
                        {
                            // find the inlinee/expanded line details for current inline source line
                            IDiaEnumLineNumbers* pInlineeLines;

                            if (S_OK == m_pSession->findInlineeLinesByLinenum(pCompiland, pSourceFile, lineNum, 0, &pInlineeLines))
                            {
                                // iterate through all the inlinee lines and update the map
                                IDiaLineNumber* pInlineeLine;
                                bool isCurrLineInlined = false;

                                while (S_OK == pInlineeLines->Next(1, &pInlineeLine, &cnt))
                                {
                                    DWORD inlineeRva;
                                    DWORD inlineeLen;

                                    if (S_OK == pInlineeLine->get_relativeVirtualAddress(&inlineeRva) &&
                                        S_OK == pInlineeLine->get_length(&inlineeLen))
                                    {
                                        auto it = m_inlineeLinesCache.find(static_cast<gtRVAddr>(inlineeRva));

                                        // map doesn't have any entry for this RVA key
                                        if (m_inlineeLinesCache.end() == it)
                                        {
                                            // Insert pLine to LinesCache map
                                            LineInfo lineInfo;
                                            lineInfo.len = inlineeLen;
                                            lineInfo.pLine = pLine;
                                            m_inlineeLinesCache.insert(InlineeLinesMap::value_type(static_cast<gtRVAddr>(inlineeRva), lineInfo));
                                            freeLineObj = false;
                                        }
                                        else
                                        {
                                            // map has an entry for this RVA key
                                            // need to decide which value to keep: old one or this new one.
                                            wchar_t* inlineeFuncName = NULL;

                                            // Fetch the last inlined function name from DIA
                                            IDiaSymbol* pParent;

                                            if (S_OK == m_pSession->findSymbolByRVA(inlineeRva, SymTagFunction, &pParent))
                                            {
                                                IDiaEnumSymbols* pChildren;

                                                if (S_OK == pParent->findInlineFramesByRVA(inlineeRva, &pChildren))
                                                {
                                                    IDiaSymbol* pChild;
                                                    ULONG el;

                                                    if (S_OK == pChildren->Next(1, &pChild, &el))
                                                    {
                                                        inlineeFuncName = ExtractDemangledName(pChild);
                                                        pChild->Release();
                                                    }

                                                    pChildren->Release();
                                                }

                                                pParent->Release();
                                            }

                                            if (NULL == inlineeFuncName)
                                            {
                                                pInlineeLine->Release();
                                                continue;
                                            }

                                            DWORD insertedFuncRva;
                                            it->second.pLine->get_relativeVirtualAddress(&insertedFuncRva);

                                            FunctionSymbolInfo insertedFunc;
                                            insertedFunc.m_pName = NULL;
                                            insertedFunc.m_rva = insertedFuncRva;
                                            wchar_t* insertedFuncName = NULL;

                                            auto funcIt = m_inlinedFuncsInfo.upper_bound(insertedFunc);

                                            if (m_inlinedFuncsInfo.begin() != funcIt)
                                            {
                                                --funcIt;
                                                const FunctionSymbolInfo& inlinedFunc = funcIt->first;

                                                // check if the target RVA is in the inlined function RVA range
                                                if (inlinedFunc.m_rva <= funcInfo.m_rva && funcInfo.m_rva < (inlinedFunc.m_rva + inlinedFunc.m_size))
                                                {
                                                    // skip the "[inlined] " prefix
                                                    insertedFuncName = funcIt->first.m_pName + 10;
                                                }
                                            }

                                            size_t inlineeFuncNameLen = wcslen(inlineeFuncName);

                                            if ((NULL != insertedFuncName && 0 == wcsncmp(inlineeFuncName, insertedFuncName, inlineeFuncNameLen))
                                                || (0 != wcsncmp(inlineeFuncName, funcInfo.m_pName, inlineeFuncNameLen)))
                                            {
                                                SysFreeString(inlineeFuncName);
                                                pInlineeLine->Release();
                                                continue;
                                            }
                                            else
                                            {
                                                // Replace previous pLine with current pLine in LinesCache map
                                                m_pendingLineObjectsToRelease.push_back(it->second.pLine);
                                                it->second.len = inlineeLen;
                                                it->second.pLine = pLine;
                                                SysFreeString(inlineeFuncName);
                                                freeLineObj = false;
                                            }
                                        }
                                    }

                                    pInlineeLine->Release();
                                    isCurrLineInlined = true;
                                }

                                if (true == isCurrLineInlined)
                                {
                                    DWORD lineRva;
                                    DWORD lineLen;

                                    if (S_OK == pLine->get_length(&lineLen) && S_OK == pLine->get_relativeVirtualAddress(&lineRva))
                                    {
                                        if (GT_INVALID_RVADDR == inlineFuncRva)
                                        {
                                            // initialize the RVA & size
                                            inlineFuncRva = lineRva;
                                            inlineFuncSize = lineLen;
                                        }
                                        else
                                        {
                                            // update the RVA & size
                                            // always note the lowest RVA, as the lines might not be in RVA-increasing-order
                                            inlineFuncRva = inlineFuncRva > lineRva ? lineRva : inlineFuncRva;
                                            inlineFuncSize += lineLen;
                                        }
                                    }
                                }

                                pInlineeLines->Release();
                            }

                            pCompiland->Release();
                        }

                        if (freeLineObj)
                        {
                            // pLine is not added to map, release it
                            pLine->Release();
                        }
                    }

                    osCriticalSectionLocker guard(const_cast<osCriticalSection&>(m_inlineFuncsLock));
                    // insert the inlined function details to map
                    UpdateInlinedFunctionName(funcInfo);
                    funcInfo.m_rva = inlineFuncRva;
                    funcInfo.m_size = inlineFuncSize;
                    auto res = m_inlinedFuncsInfo.insert(FunctionsMap::value_type(funcInfo, true));
                    ret = res.second;

                    if (NULL != pSourceFile)
                    {
                        pSourceFile->Release();
                    }

                    pLines->Release();
                }
            }
        }
    }

    return ret;
}

// (1) This function uses DIA SDK APIs to fetch the inlined function information from the PDB file.
// For this, we need to process all the source code lines associated with the current executable.
// Hence, this function might lead to performance hit in worst cases (e.g. very large set of source files)
//
// (2) Following DIA APIs don't provide expected result:
//   IDiaSession::findInlineFramesByAddr()
//   IDiaSession::findInlineFramesByRVA()
//   IDiaSession::findInlineFramesByVA()
//   IDiaSession::findInlineeLines()
//   IDiaSession::findInlineeLinesByAddr()
//   IDiaSession::findInlineeLinesByRVA()
//   IDiaSession::findInlineeLinesByVA()
//   IDiaSession::findInlineesByName()
//   IDiaSymbol::findInlineFramesByAddr()
//   IDiaSymbol::findInlineFramesByVA()
//   IDiaSymbol::findInlineeLines()
//   IDiaSymbol::findInlineeLinesByAddr()
//   IDiaSymbol::findInlineeLinesByRVA()
//   IDiaSymbol::findInlineeLinesByVA()
//
// (3) Wendy Thrash (Microsoft Interface team) also confirmed unpredictable nature of
// current DIA inline APIs.
//
// (4) In future, if the APIs mentioned in (2) become functional then those APIs must be
// used to improve the performance of this function.
//
// (5) This function assumes that the stand alone object code of inline function is still
// present in the executable file. May be once we start using APIs noted in (2), this limitation
// would go away.
bool PdbSymbolEngine::ProcessInlinedFunctionInfo(IDiaSymbol* pGlobalScope) const
{
    bool ret = false;

    // find all the public functions
    IDiaEnumSymbols* pFunctions;

    if (m_processInlineSamples
        && (S_OK == m_pSession->findChildren(pGlobalScope, SymTagFunction, NULL, nsNone, &pFunctions)))
    {
        // iterate through the functions list
        IDiaSymbol* pFunc = NULL;
        ULONG cnt = 0;

        while (S_OK == pFunctions->Next(1, &pFunc, &cnt))
        {
            ProcessInlinedFunction(pFunc);

            pFunc->Release();
        }

        pFunctions->Release();

        ret = true;
    }

    return ret;
}

bool PdbSymbolEngine::ProcessInlinedFunctionByName(const wchar_t* pSymbolName, gtList<wchar_t*>& inlinedChildNameList) const
{
    bool ret = false;

    if (nullptr != pSymbolName)
    {
        // Retrieve a reference to the global scope.
        IDiaSymbol* pGlobalScope = nullptr;

        HRESULT hr = m_pSession->get_globalScope(&pGlobalScope);

        if (S_OK == hr)
        {
            IDiaEnumSymbols* pEnumSymbols = nullptr;

            // Replaced nsRegularExpression with nsCaseSensitive.
            // If this change impacts any particular behaviour, then revert the change.
            if (S_OK == (pGlobalScope->findChildren(SymTagNull, pSymbolName, nsCaseSensitive, &pEnumSymbols)))
            {
                IDiaSymbol* pSymbol = nullptr;
                ULONG celt = 0;

                while (S_OK == (pEnumSymbols->Next(1, &pSymbol, &celt)) && (celt == 1))
                {
                    DWORD symTag = SymTagNull;
                    pSymbol->get_symTag(&symTag);

                    DWORD symRva;

                    if (SymTagFunction == symTag
                        || SymTagThunk == symTag
                        || (SymTagPublicSymbol == symTag && S_OK == pSymbol->get_relativeVirtualAddress(&symRva)
                            && m_pPortableExe->IsCodeSection(m_pPortableExe->LookupSectionIndex(static_cast<gtRVAddr>(symRva)))))
                    {
                        ret = ProcessInlinedFunction(pSymbol);

                        // Get the inlinesite for this symbol only if
                        // current inline function is successfully processed
                        if (ret)
                        {
                            SymbolHasInlinedChild(pSymbol, inlinedChildNameList);
                        }
                    }

                    pSymbol->Release();
                }

                pEnumSymbols->Release();
            }

            pGlobalScope->Release();
        } // S_OK == hr
    }

    return ret;
}

// If the given DIA pSymbol has inlined child, then the name of inlined child will be returned
// Note: The Caller has to call SysFreeString on the returned ppInlinedChildName
//
bool PdbSymbolEngine::SymbolHasInlinedChild(IDiaSymbol* pSymbol, gtList<wchar_t*>& symbolNameList) const
{
    bool ret = false;

    if (nullptr != pSymbol)
    {
        IDiaEnumSymbols* pEnumChildren = nullptr;

        // FIXME: add lock
        // osCriticalSectionLocker guard(const_cast<osCriticalSection&>(m_diaLock));

        if (S_OK == (pSymbol->findChildren(SymTagNull, nullptr, nsNone, &pEnumChildren)))
        {
            IDiaSymbol* pChild = nullptr;
            ULONG celt = 0;

            try
            {
                while (S_OK == (pEnumChildren->Next(1, &pChild, &celt)) && (celt == 1))
                {
                    DWORD dwSymTag;

                    if ((S_OK == pChild->get_symTag(&dwSymTag)) && (SymTagInlineSite == dwSymTag))
                    {
                        // found an inline site
                        ret = true;
                        BSTR pStrName = nullptr;
                        BSTR pStrUndName = nullptr;

                        // Get the name
                        if (S_OK == pChild->get_name(&pStrName))
                        {
                            symbolNameList.push_back(pStrName);
                        }
                        else if (S_OK == pChild->get_undecoratedName(&pStrUndName))
                        {
                            symbolNameList.push_back(pStrUndName);
                        }
                    }

                    pChild->Release();
                }
            }
            catch (char* exceptionDesc)
            {
                // Exception inside Microsoft's msdia dll. Log an error
                gtString exceptionString;
                exceptionString.fromASCIIString(exceptionDesc);
                gtString errMsg(L"msdia140 exception inside symbol traversal's Next() : ");
                errMsg.append(exceptionString);
                OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
            }
            catch (...)
            {
                // Exception inside Microsoft's msdia dll. Log an error
                OS_OUTPUT_DEBUG_LOG(L"msdia140 exception inside symbol traversal's Next()", OS_DEBUG_LOG_ERROR);
            }

            pEnumChildren->Release();
        }
    }

    return ret;
}

bool PdbSymbolEngine::AddInlinedChildByName(wchar_t* pSymbolName) const
{
    bool ret = false;

    if (nullptr != pSymbolName)
    {
        gtList<wchar_t*> symbolNameList;

        // check if the name contains {dtor}/{ctor}, if so then replace them with the proper class name
        gtString symbolName(pSymbolName);
        int idx = 0;

        if (-1 != (idx = symbolName.find(L"::{ctor}")))
        {
            gtString className;
            symbolName.getSubString(0, idx - 1, className);

            symbolName.replace(L"{ctor}", className, false);
        }
        else if (-1 != (idx = symbolName.find(L"::{dtor}")))
        {
            gtString className;
            symbolName.getSubString(0, idx - 1, className);
            className.prepend(L"~");

            symbolName.replace(L"{dtor}", className, false);
        }

        ret = ProcessInlinedFunctionByName(symbolName.asCharArray(), symbolNameList);
        SysFreeString(pSymbolName);

        for (auto it = symbolNameList.begin(); it != symbolNameList.end(); it++)
        {
            wchar_t* pName = (*it);
            AddInlinedChildByName(pName);
        }

        symbolNameList.clear();
    }

    return ret;
}

bool PdbSymbolEngine::AddInlinedChild(IDiaSymbol* pSymbol) const
{
    bool foundInlineChild = false;
    gtList<wchar_t*> inlinedSymbolNameList;

    SymbolHasInlinedChild(pSymbol, inlinedSymbolNameList);

    for (auto it = inlinedSymbolNameList.begin(); it != inlinedSymbolNameList.end(); it++)
    {
        BSTR pInlinedSymbolName = (*it);

        // If we could process one inlineesite, that is good enough to mark this function as "hasinlines"
        foundInlineChild |= AddInlinedChildByName(pInlinedSymbolName);
    }

    inlinedSymbolNameList.clear();

    return foundInlineChild;
}

const FunctionSymbolInfo* PdbSymbolEngine::LookupInlinedFunction(gtRVAddr rva) const
{
    const FunctionSymbolInfo* pFuncInfo = NULL;

    // if the RVA because of inline expansion, then return the inline func info
    if (IsFunctionInlined(rva))
    {
        FunctionSymbolInfo funcInfo;
        funcInfo.m_pName = NULL;
        funcInfo.m_rva = TranslateToInlineeRVA(rva);

        // search the inline function RVA in the map
        FunctionsMap::iterator itInlined = m_inlinedFuncsInfo.upper_bound(funcInfo);

        if (m_inlinedFuncsInfo.begin() != itInlined)
        {
            --itInlined;
            const FunctionSymbolInfo& inlinedFunc = itInlined->first;

            // check if the target RVA is in the inlined function RVA range
            if (inlinedFunc.m_rva <= funcInfo.m_rva && funcInfo.m_rva < (inlinedFunc.m_rva + inlinedFunc.m_size))
            {
                pFuncInfo = &itInlined->first;
            }
        }
    }

    return pFuncInfo;
}

const FunctionSymbolInfo* PdbSymbolEngine::LookupFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    // Find the given RVA's bounding function.
    const FunctionSymbolInfo* pFunc = PdbSymbolEngine::LookupBoundingFunction(rva, pNextRva, handleInline);

    // If a candidate bounding function has been found.
    if (NULL != pFunc)
    {
        // If the function just represents an entry point, meaning that its size has not been determined.
        if (0 == pFunc->m_size)
        {
            // If the executable's section in which the function's entry point is located
            // is different from the executable's section in which the given RVA is located.
            if (m_pPortableExe->LookupSectionIndex(pFunc->m_rva) != m_pPortableExe->LookupSectionIndex(rva))
            {
                // Invalidate the function's pointer,
                // as there is no function found, that may contain the given RVA in the same executable's section.
                // Note: The bounding function was only a candidate, but if it is from another section, then it is no more a valid candidate.
                pFunc = NULL;
            }
        }
        // If the bounding function has a valid size,
        // and the given RVA is not contained within the function (in the range [pFunc->m_rva, pFunc->m_rva + pFunc->m_size) ).
        else if (((pFunc->m_rva + pFunc->m_size) <= rva) && (!m_processInlineSamples || !handleInline || !IsFunctionInlined(rva)))
        {
            // Invalidate the function's pointer,
            // as there is no function found, that contains the given RVA.
            pFunc = NULL;
        }
    }

    return pFunc;
}

const FunctionSymbolInfo* PdbSymbolEngine::LookupBoundingFunction(gtRVAddr rva, gtRVAddr* pNextRva, bool handleInline) const
{
    const FunctionSymbolInfo* pFuncInfo = NULL;
    bool foundInlinedChild = false;

    if (m_processInlineSamples && handleInline)
    {
        pFuncInfo = PdbSymbolEngine::LookupInlinedFunction(rva);
    }

    // if rva doesn't match any inline function, then search it in non-inline function map
    if (NULL == pFuncInfo)
    {
        const FunctionSymbolInfo* pCandidateFuncInfo = NULL;

        FunctionSymbolInfo funcInfo;
        funcInfo.m_rva = rva;
        funcInfo.m_pName = NULL;

        m_funcsLock.lockRead();
        FunctionsMap::iterator it = m_funcsInfo.upper_bound(funcInfo);

        if (m_funcsInfo.begin() != it)
        {
            --it;

            m_funcsLock.unlockRead();

            // If the (function's) next function in the map is the same next function as in the DIA symbol engine.
            // This means that there is no other possible bounding function.
            if (it->second)
            {
                pFuncInfo = &it->first;
            }
            else
            {
                pCandidateFuncInfo = &it->first;
            }
        }
        else
        {
            m_funcsLock.unlockRead();
        }

        if (NULL == pFuncInfo)
        {
            // Find the bounding function's symbol in the DIA symbol engine.
            IDiaSymbol* pSymbol = FindBoundingFunction(rva);

            if (NULL != pSymbol)
            {
                DWORD symRva = 0;
                pSymbol->get_relativeVirtualAddress(&symRva);

                // If the DIA symbol actually represents the candidate function, the we do not need to construct it.
                if (NULL != pCandidateFuncInfo && static_cast<gtRVAddr>(symRva) == pCandidateFuncInfo->m_rva)
                {
                    // We are going to insert the directly next function into the map, so set the iterator value to 'true'.
                    it->second = true;
                    pFuncInfo = pCandidateFuncInfo;
                }
                else
                {
                    ConstructFunctionSymbolInfo(funcInfo, pSymbol, true);
                    foundInlinedChild = funcInfo.m_hasInlines;

                    m_funcsLock.lockWrite();
                    FunctionsMap::_Pairib itb = m_funcsInfo.insert(FunctionsMap::value_type(funcInfo, true));

                    // If the info was already present, then delete the current one.
                    if (!itb.second)
                    {
                        if (NULL != funcInfo.m_pName)
                        {
                            SysFreeString(funcInfo.m_pName);
                        }
                    }

                    it = itb.first;
                    pFuncInfo = &it->first;
                    m_funcsLock.unlockWrite();
                }

                // Baskar: FIXME.. why do we need to find the next symbol ?
                IDiaSymbol* pPrevSymbol = pSymbol;

                // Calculate the last RVA of the function.
                symRva = pFuncInfo->m_rva + pFuncInfo->m_size;
                symRva -= static_cast<DWORD>(0 != pFuncInfo->m_size);
                pSymbol = FindNextBoundingFunction(pPrevSymbol, symRva);

                pPrevSymbol->Release();

                if (NULL != pSymbol)
                {
                    m_funcsLock.lockRead();
                    FunctionsMap::iterator itNext = it;
                    ++itNext;

                    bool notNext = (m_funcsInfo.end() == itNext);
                    m_funcsLock.unlockRead();

                    if (notNext || S_OK != pSymbol->get_relativeVirtualAddress(&symRva) || symRva != itNext->first.m_rva)
                    {
                        ConstructFunctionSymbolInfo(funcInfo, pSymbol, true);

                        m_funcsLock.lockWrite();
                        FunctionsMap::_Pairib itb = m_funcsInfo.insert(FunctionsMap::value_type(funcInfo, false));

                        // If the info was already present, then delete the current one.
                        if (!itb.second)
                        {
                            if (NULL != funcInfo.m_pName)
                            {
                                SysFreeString(funcInfo.m_pName);
                            }
                        }

                        m_funcsLock.unlockWrite();
                    }

                    pSymbol->Release();
                }
            }
        }

        if (foundInlinedChild && handleInline)
        {
            // Again, check if this RVA belongs to an inlined child
            const FunctionSymbolInfo* pInlinedFuncInfo = PdbSymbolEngine::LookupInlinedFunction(rva);

            if (nullptr != pInlinedFuncInfo)
            {
                pFuncInfo = pInlinedFuncInfo;
            }
        }

        if (NULL != pNextRva)
        {
            if (NULL != pFuncInfo)
            {
                m_funcsLock.lockRead();
                bool found = (++it != m_funcsInfo.end());
                m_funcsLock.unlockRead();

                if (found)
                {
                    *pNextRva = it->first.m_rva;
                }
                else
                {
                    *pNextRva = GT_INVALID_RVADDR;
                }
            }
            else
            {
                *pNextRva = GT_INVALID_RVADDR;
            }
        }
    }
    else
    {
        if (NULL != pNextRva)
        {
            // we know the size of the func, set nextRva to INVALID
            // as this will be used if function size is unknown
            *pNextRva = GT_INVALID_RVADDR;
        }
    }

    return pFuncInfo;
}

IDiaSymbol* PdbSymbolEngine::FindBoundingFunction(gtRVAddr rva) const
{
    IDiaSymbol* pSymbol = NULL;

    // Make access to msdia thread-safe. OMG, using a const_cast here is despicable,
    // but in its essence this is really a const function. The only non-const behavior is
    // using the critical section for thread-safety, which in my eyes does not violate
    // the 'const' contract of the function
    osCriticalSectionLocker guard(const_cast<osCriticalSection&>(m_diaLock));

    if (S_OK == m_pEnumSymbols->symbolByRVA(rva, &pSymbol))
    {
        IDiaEnumSymbolsByAddr* pEnumSymbols;

        if (S_OK == m_pEnumSymbols->Clone(&pEnumSymbols))
        {
            ULONG num;

            do
            {
                DWORD symTag = SymTagNull;
                pSymbol->get_symTag(&symTag);

                DWORD symRva;

                if (SymTagFunction == symTag || SymTagThunk == symTag ||
                    (SymTagPublicSymbol == symTag &&
                     S_OK == pSymbol->get_relativeVirtualAddress(&symRva) &&
                     m_pPortableExe->IsCodeSection(m_pPortableExe->LookupSectionIndex(static_cast<gtRVAddr>(symRva)))))
                {
                    break;
                }

                pSymbol->Release();
                pSymbol = NULL;
            }
            while (S_OK == pEnumSymbols->Prev(1UL, &pSymbol, &num));

            pEnumSymbols->Release();
        }
        else
        {
            pSymbol->Release();
            pSymbol = NULL;
        }
    }

    return pSymbol;
}

IDiaSymbol* PdbSymbolEngine::FindNextBoundingFunction(IDiaSymbol* pPrevSymbol, gtRVAddr rva) const
{
    DWORD prevSymRva = 0;
    pPrevSymbol->get_relativeVirtualAddress(&prevSymRva);

    bool stop = false;
    ULONG num;
    IDiaSymbol* pSymbol = NULL;
    // Make access to msdia thread-safe. OMG, using a const_cast here is despicable,
    // but in its essence this is really a const function. The only non-const behavior is
    // using the critical section for thread-safety, which in my eyes does not violate
    // the 'const' contract of the function
    osCriticalSectionLocker guard(const_cast<osCriticalSection&>(m_diaLock));

    while (!stop && S_OK == m_pEnumSymbols->Next(1UL, &pSymbol, &num))
    {
        DWORD symRva = 0;
        pSymbol->get_relativeVirtualAddress(&symRva);

        if (rva < symRva)
        {
            DWORD symTag = SymTagNull;
            pSymbol->get_symTag(&symTag);

            if (SymTagFunction == symTag || SymTagThunk == symTag ||
                (SymTagPublicSymbol == symTag &&
                 m_pPortableExe->IsCodeSection(m_pPortableExe->LookupSectionIndex(static_cast<gtRVAddr>(symRva)))))
            {
                break;
            }
        }

        // The enumeration is cyclic, so prevent starting from the beginning.
        if (symRva <= prevSymRva)
        {
            if (symRva == prevSymRva)
            {
                DWORD prevSymId = 0;
                pPrevSymbol->get_symIndexId(&prevSymId);

                DWORD symId = 0;
                pSymbol->get_symIndexId(&symId);

                if (prevSymId == symId)
                {
                    stop = true;
                }
            }
            else
            {
                stop = true;
            }
        }

        pSymbol->Release();
        pSymbol = NULL;
    }

    return pSymbol;
}

gtRVAddr PdbSymbolEngine::LoadSymbol(const wchar_t* pName, gtUInt32 size)
{
    DWORD rva = GT_INVALID_RVADDR;

    if (NULL != m_pSession)
    {
        IDiaSymbol* pGlobalScope;

        if (S_OK == m_pSession->get_globalScope(&pGlobalScope))
        {
            IDiaEnumSymbols* pEnumSymbols;

            if (S_OK == m_pSession->findChildren(pGlobalScope, SymTagNull, pName, nsCaseSensitive, &pEnumSymbols))
            {
                const ULONGLONG refSize = static_cast<ULONGLONG>(size);
                ULONG num;
                IDiaSymbol* pSymbol;

                while (S_OK == pEnumSymbols->Next(1UL, &pSymbol, &num))
                {
                    ULONGLONG symSize;

                    if (S_OK == pSymbol->get_length(&symSize) && symSize == refSize)
                    {
                        if (S_OK == pSymbol->get_relativeVirtualAddress(&rva))
                        {
                            pSymbol->Release();
                            break;
                        }
                    }

                    pSymbol->Release();
                }

                pEnumSymbols->Release();
            }
        }
    }

    return static_cast<gtRVAddr>(rva);
}

HRESULT PdbSymbolEngine::FindFrameInterface(gtVAddr va, IDiaFrameData** ppFrame) const
{
    HRESULT hr = E_NOTIMPL;

    if (NULL != m_pEnumFrames)
    {
        if (NULL == ppFrame)
        {
            hr = E_POINTER;
        }
        else
        {
            gtRVAddr rva = m_pPortableExe->VaToRva(va);

            m_funcsLock.lockRead();
            gtMap<gtRVAddr, IDiaFrameData*>::iterator it = m_framesCache.find(rva);
            bool found = (it != m_framesCache.end());
            m_funcsLock.unlockRead();

            if (!found)
            {
                hr = m_pEnumFrames->frameByVA(va, ppFrame);

                if (NULL != *ppFrame)
                {
                    m_funcsLock.lockWrite();
                    gtMap<gtRVAddr, IDiaFrameData*>::_Pairib itb = m_framesCache.insert(make_pair(rva, *ppFrame));

                    if (!itb.second)
                    {
                        (*ppFrame)->Release();
                    }

                    m_funcsLock.unlockWrite();

                    it = itb.first;
                    found = true;
                }
            }

            if (found)
            {
                *ppFrame = it->second;
                (*ppFrame)->AddRef();
                hr = S_OK;
            }
        }
    }

    return hr;
}

HRESULT PdbSymbolEngine::FindSymbolInterface(gtVAddr va, IDiaSymbol** ppSymbol) const
{
    HRESULT hr = E_NOTIMPL;

    if (NULL != m_pSession)
    {
        if (NULL == ppSymbol)
        {
            hr = E_POINTER;
        }
        else
        {
            gtRVAddr rva = m_pPortableExe->VaToRva(va);

            m_funcsLock.lockRead();
            gtMap<gtRVAddr, IDiaSymbol*>::iterator it = m_symbolsCache.find(rva);
            bool found = (it != m_symbolsCache.end());
            m_funcsLock.unlockRead();

            if (!found)
            {
                *ppSymbol = FindFunctionSymbol(rva);

                if (NULL == *ppSymbol)
                {
                    hr = m_pSession->findSymbolByVA(va, SymTagThunk, ppSymbol);

                    if (NULL == *ppSymbol)
                    {
                        hr = m_pSession->findSymbolByVA(va, SymTagPublicSymbol, ppSymbol);
                    }
                }

                if (NULL != *ppSymbol)
                {
                    m_funcsLock.lockWrite();
                    gtMap<gtRVAddr, IDiaSymbol*>::_Pairib itb = m_symbolsCache.insert(make_pair(rva, *ppSymbol));

                    if (!itb.second)
                    {
                        (*ppSymbol)->Release();
                    }

                    m_funcsLock.unlockWrite();

                    it = itb.first;
                    found = true;
                }
            }

            if (found)
            {
                *ppSymbol = it->second;
                (*ppSymbol)->AddRef();
                hr = S_OK;
            }
        }
    }

    return hr;
}

IDiaEnumFrameData* PdbSymbolEngine::FindEnumFrameData() const
{
    IDiaEnumFrameData* pEnumFrameData = NULL;

    if (NULL != m_pSession)
    {
        IDiaEnumTables* pEnumTables;

        if (S_OK == m_pSession->getEnumTables(&pEnumTables))
        {
            ULONG num;
            IDiaTable* pTable;

            while (S_OK == pEnumTables->Next(1UL, &pTable, &num))
            {
                HRESULT hr = pTable->QueryInterface(&pEnumFrameData);
                pTable->Release();

                if (S_OK == hr)
                {
                    break;
                }
            }

            pEnumTables->Release();
        }
    }

    return pEnumFrameData;
}

IDiaSymbol* PdbSymbolEngine::FindFunctionSymbol(gtRVAddr rva) const
{
    IDiaSymbol* pSymbol = NULL;
    HRESULT hr = S_FALSE;

    try
    {
        // Make access to msdia thread-safe. OMG, using a const_cast here is despicable,
        // but in its essence this is really a const function. The only non-const behavior is
        // using the critical section for thread-safety, which in my eyes does not violate
        // the 'const' contract of the function
        osCriticalSectionLocker guard(const_cast<osCriticalSection&>(m_diaLock));

        // I see msdia120 crash inside this call, so wrap it in a try-catch block
        hr = m_pEnumSymbols->symbolByRVA(rva, &pSymbol);
    }
    catch (char* exceptionDesc)
    {
        // Exception inside Microsoft's msdia120 dll. Log an error
        gtString exceptionString;
        exceptionString.fromASCIIString(exceptionDesc);
        gtString errMsg(L"msdia140 exception inside symbolbyRVA() : ");
        errMsg.append(exceptionString);
        OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
    }
    catch (...)
    {
        // Exception inside Microsoft's msdia120 dll. Log an error
        OS_OUTPUT_DEBUG_LOG(L"msdia140 exception inside symbolbyRVA()", OS_DEBUG_LOG_ERROR);
    }

    if (S_OK == hr)
    {
        do
        {
            IDiaSymbol* pSymbolParent = NULL;

            DWORD tag = SymTagNull;
            pSymbol->get_symTag(&tag);

            if (tag == SymTagFunction)
            {
                break;
            }

            pSymbol->get_lexicalParent(&pSymbolParent);

            pSymbol->Release();
            pSymbol = pSymbolParent;
        }
        while (NULL != pSymbol);
    }

    return pSymbol;
}

// This function is currently not used as the target DIA API findInlineFramesByRVA()
// doesn't return any inlined function information. In future, if this API returns
// expected result, use this function to process inline function information.
IDiaSymbol* PdbSymbolEngine::FindInlineSymbol(gtRVAddr rva) const
{
    IDiaSymbol* pSymbol = FindFunctionSymbol(rva);

    if (NULL != pSymbol)
    {
        IDiaEnumSymbols* pEnumSymbols;
        HRESULT hr = m_pSession->findInlineFramesByRVA(pSymbol, rva, &pEnumSymbols);
        pSymbol->Release();
        pSymbol = NULL;

        if (S_OK == hr)
        {
            ULONG num;
            pEnumSymbols->Next(1UL, &pSymbol, &num);
            pEnumSymbols->Release();
        }
    }

    return pSymbol;
}

// This looks into the codexl's internal structures
bool PdbSymbolEngine::IsFunctionInlined(gtRVAddr rva) const
{
    bool ret = false;

    if (m_processInlineSamples)
    {
        // search in the map based on RVA to find the bounding function
        InlineeLinesMap::iterator lit = m_inlineeLinesCache.upper_bound(rva);

        if (lit != m_inlineeLinesCache.begin())
        {
            --lit;
            DWORD len = 0;
            len = lit->second.len;

            if (lit->first <= rva && rva < (lit->first + len))
            {
                // check if RVA is within the function RVA range
                ret = true;
            }
        }
    }

    return ret;
}

bool PdbSymbolEngine::FindSourceLine(gtRVAddr rva, SourceLineInfo& sourceLine, bool inludeInline)
{
    bool ret = false;

    if (NULL != m_pSession)
    {
        /*
        IDiaSymbol* pInlineSymbol = FindInlineSymbol(rva);

        if (NULL != pInlineSymbol)
        {
            hr = pInlineSymbol->findInlineeLinesByRVA(rva, 1, &pEnumLines);
            pInlineSymbol->Release();
        }
        else
        */

        // search the function first in the inline function info maps.
        if (inludeInline && IsFunctionInlined(rva))
        {
            InlineeLinesMap::iterator lit = m_inlineeLinesCache.upper_bound(rva);

            if (lit != m_inlineeLinesCache.begin())
            {
                --lit;
                DWORD len = 0;
                len = lit->second.len;

                if (rva < lit->first || (lit->first + len) <= rva)
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

            // valid inline function found in map. Update SourceLineInfo.
            IDiaLineNumber* pInlineeLine = lit->second.pLine;
            DWORD val;
            pInlineeLine->get_relativeVirtualAddress(&val);
            sourceLine.m_rva = val;
            sourceLine.m_offset = rva - val;

            sourceLine.m_line = lit->second.len;
            sourceLine.m_filePath[0] = L'\0';

            IDiaSourceFile* pSourceFile;

            if (S_OK == pInlineeLine->get_sourceFile(&pSourceFile))
            {
                BSTR pFileName = nullptr;

                if (S_OK == pSourceFile->get_fileName(&pFileName))
                {
                    UINT len = SysStringLen(pFileName);

                    if (OS_MAX_PATH <= len)
                    {
                        len = OS_MAX_PATH - 1;
                    }

                    memcpy(sourceLine.m_filePath, pFileName, sizeof(wchar_t) * len);
                    sourceLine.m_filePath[len] = L'\0';
                    SysFreeString(pFileName);
                }

                pSourceFile->Release();
            }

            ret = true;
        }
        else
        {
            HRESULT hr;
            IDiaEnumLineNumbers* pEnumLines;
            hr = m_pSession->findLinesByRVA(rva, 1, &pEnumLines);

            if (S_OK == hr)
            {
                IDiaLineNumber* pLineNumber;

                if (S_OK == pEnumLines->Item(0, &pLineNumber))
                {
                    DWORD val;
                    pLineNumber->get_relativeVirtualAddress(&val);
                    sourceLine.m_rva = val;
                    sourceLine.m_offset = rva - val;

                    pLineNumber->get_lineNumber(&val);
                    sourceLine.m_line = val;

                    sourceLine.m_filePath[0] = L'\0';
                    IDiaSourceFile* pSourceFile;

                    if (S_OK == pLineNumber->get_sourceFile(&pSourceFile))
                    {
                        BSTR pFileName = nullptr;

                        if (S_OK == pSourceFile->get_fileName(&pFileName))
                        {
                            UINT len = SysStringLen(pFileName);

                            if (OS_MAX_PATH <= len)
                            {
                                len = OS_MAX_PATH - 1;
                            }

                            memcpy(sourceLine.m_filePath, pFileName, sizeof(wchar_t) * len);
                            sourceLine.m_filePath[len] = L'\0';
                            SysFreeString(pFileName);
                        }

                        pSourceFile->Release();
                    }

                    pLineNumber->Release();
                    ret = true;
                }

                pEnumLines->Release();
            }
        }
    }

    return ret;
}

bool PdbSymbolEngine::EnumerateSourceLineInstances(const wchar_t* pSourceFilePath, SrcLineInstanceMap& srcLineInstanceMap, bool handleInline)
{
    bool ret = false;

    if (NULL != m_pSession)
    {
        IDiaEnumSourceFiles* pEnumSources;

        if (S_OK == m_pSession->findFile(NULL, pSourceFilePath, nsNone, &pEnumSources))
        {
            LONG countSources;
            ret = (S_OK == pEnumSources->get_Count(&countSources));

            if (ret && 0 < countSources)
            {
                IDiaSourceFile* pSource;

                if (S_OK == pEnumSources->Item(0, &pSource))
                {
                    IDiaEnumSymbols* pEnumCompilands;

                    if (S_OK == pSource->get_compilands(&pEnumCompilands))
                    {
                        ULONG num;
                        IDiaSymbol* pCompiland;

                        while (S_OK == pEnumCompilands->Next(1UL, &pCompiland, &num))
                        {
                            IDiaEnumLineNumbers* pEnumLines;

                            if (S_OK == m_pSession->findLines(pCompiland, pSource, &pEnumLines))
                            {
                                IDiaLineNumber* pLine;

                                while (S_OK == pEnumLines->Next(1UL, &pLine, &num))
                                {
                                    DWORD line, rva;

                                    if (S_OK == pLine->get_lineNumber(&line) && S_OK == pLine->get_relativeVirtualAddress(&rva))
                                    {
                                        if (handleInline && m_processInlineSamples && IsFunctionInlined(rva))
                                        {
                                            // map the caller RVA to inline function source line num
                                            InlineeLinesMap::iterator lit = m_inlineeLinesCache.upper_bound(rva);

                                            if (lit != m_inlineeLinesCache.begin())
                                            {
                                                --lit;
                                                lit->second.pLine->get_lineNumber(&line);
                                            }
                                        }

                                        srcLineInstanceMap[rva] = line;
                                    }

                                    pLine->Release();
                                }

                                pEnumLines->Release();
                            }

                            pCompiland->Release();
                        }

                        pEnumCompilands->Release();
                    }

                    pSource->Release();
                }
            }

            pEnumSources->Release();
        }
    }

    return ret;
}

// As of now prepend [inlined] to inlined function name
// In future it should be:
// Aggregate case:  [inlined] foo()
// Individual case: [inlined] foo()@bar()+200, where 200 is the function offset
void PdbSymbolEngine::UpdateInlinedFunctionName(FunctionSymbolInfo& funcInfo) const
{
    if (m_processInlineSamples)
    {
        gtString inlinedName = L"[inlined] ";
        inlinedName += funcInfo.m_pName;

#if 0

        if (false == m_aggregateInlineSamples)
        {
            // append "@parent-function()+offset"
            IDiaSymbol* func = FindFunctionSymbol(funcInfo.m_rva);
            BSTR funcName;
            func->get_name(&funcName);
            inlinedName += L"@";
            inlinedName += funcName;
            SysFreeString(funcName);
            func->Release();
            inlinedName += L"+";
            DWORD parentVA;
            func->get_relativeVirtualAddress(&parentVA);
            gtVAddr offset = static_cast<gtVAddr>(parentVA) - funcInfo.m_rva;
            wchar_t offsetStr[20];
            swprintf_s(offsetStr, L"%x", offset);
            inlinedName += offsetStr;
        }

#endif

        SysFreeString(funcInfo.m_pName);
        funcInfo.m_pName = SysAllocString(inlinedName.asCharArray());
    }
}

// This function translates rva of caller function to inlinee function rva
gtRVAddr PdbSymbolEngine::TranslateToInlineeRVA(gtRVAddr rva) const
{
    gtRVAddr inlineeRVA = rva;

    // if the rva belongs to inlined function, then return corresponding RVA
    // else return original RVA.
    if (true == m_processInlineSamples && true == m_aggregateInlineSamples)
    {
        // inlined samples are aggregated
        // caller rva need to be converted to inlinee rva
        InlineeLinesMap::iterator lit = m_inlineeLinesCache.upper_bound(rva);

        if (lit != m_inlineeLinesCache.begin())
        {
            --lit;
            DWORD len = 0;
            len = lit->second.len;

            if (lit->first <= rva && rva < (lit->first + len))
            {
                // calculate the offset RVA from the starting RVA of corresponding source line
                gtRVAddr offset = rva - lit->first;
                DWORD mRva;

                if (S_OK == lit->second.pLine->get_relativeVirtualAddress(&mRva))
                {
                    // add offset to get the appropriate inlineeRVA.
                    inlineeRVA = mRva + offset;
                }
            }
        }
    }

    return inlineeRVA;
}

// This function is called only when rva is not added to map.
bool PdbSymbolEngine::ProcessNestedInlinedFunctionInfo(gtRVAddr rva) const
{
    IDiaSymbol* pParent;
    gtVector<gtRVAddr> rvaList;

    // Make access to msdia thread-safe. OMG, using a const_cast here is despicable,
    // but in its essence this is really a const function. The only non-const behavior is
    // using the critical section for thread-safety, which in my eyes does not violate
    // the 'const' contract of the function
    osCriticalSectionLocker guard(const_cast<osCriticalSection&>(m_diaLock));

    const FunctionSymbolInfo* pFuncInfo = LookupBoundingFunction(rva, NULL, false);

    if (NULL != pFuncInfo && pFuncInfo->m_hasInlines)
    {
        // Can we use here m_pEnumSymbols->symbolByRVA()?
        if (S_OK == m_pSession->findSymbolByRVA(rva, SymTagFunction, &pParent))
        {
            DWORD funcRva;
            pParent->get_relativeVirtualAddress(&funcRva);

            // Add pFunc to vector
            rvaList.push_back(static_cast<gtRVAddr>(funcRva));

            IDiaEnumSymbols* pChildren;

            if (S_OK == pParent->findInlineFramesByRVA(rva, &pChildren))
            {
                LONG count = 0L;

                if (S_OK == pChildren->get_Count(&count) && count > 0L)
                {
                    for (LONG i = count - 1; i >= 0L; --i)
                    {
                        IDiaSymbol* pChild;

                        if (S_OK != pChildren->Item(i, &pChild))
                        {
                            continue;
                        }

                        gtRVAddr tRva;

                        if (0L == i)
                        {
                            tRva = TranslateToInlineeRVA(rva);
                        }
                        else
                        {
                            DWORD calleeRva;
                            pChild->get_relativeVirtualAddress(&calleeRva);
                            tRva = TranslateToInlineeRVA(static_cast<gtRVAddr>(calleeRva));
                        }

                        rvaList.push_back(tRva);
                        pChild->Release();
                    }
                }

                pChildren->Release();
            }

            pParent->Release();
        }
    }

    m_nestedFuncMap[rva] = rvaList;

    return true;
}

// This function is called only when rva is an inlined rva.
gtVector<gtRVAddr> PdbSymbolEngine::FindNestedInlineFunctions(gtRVAddr rva) const
{
    gtVector<gtRVAddr> list;

    if (m_processInlineSamples)
    {
        auto it = m_nestedFuncMap.find(rva);

        if (m_nestedFuncMap.end() == it)
        {
            // rva not addded to map. Add it now
            ProcessNestedInlinedFunctionInfo(rva);

            // search the map again after processing
            it = m_nestedFuncMap.find(rva);
        }

        if (m_nestedFuncMap.end() != it)
        {
            // copy the pre-processed list
            list = it->second;
        }
        else
        {
            // failed to find any list. just return the same rva back.
            list.push_back(rva);
        }
    }

    return list;
}

void PdbSymbolEngine::ConstructFunctionSymbolInfo(FunctionSymbolInfo& funcInfo, IDiaSymbol* pSymbol, bool addInlinedChild) const
{
    funcInfo.m_pName = ExtractDemangledName(pSymbol);

    FUNCSYM_OFFSET_SUPPORT(LONG offset = 0L;)
    FUNCSYM_OFFSET_SUPPORT(pSymbol->get_offset(&offset);)
    FUNCSYM_OFFSET_SUPPORT(funcInfo.m_offset = static_cast<gtUInt32>(offset);)

    DWORD symRva = 0;
    pSymbol->get_relativeVirtualAddress(&symRva);
    funcInfo.m_rva = symRva;

    ULONGLONG length = 0ULL;
    pSymbol->get_length(&length);
    funcInfo.m_size = static_cast<gtUInt32>(length);

    bool hasInlineSite = false;

    if (addInlinedChild && m_processInlineSamples)
    {
        hasInlineSite = AddInlinedChild(pSymbol);
    }

    funcInfo.m_hasInlines = hasInlineSite;
    funcInfo.m_funcId = AtomicAdd(m_nextFuncId, 1);
}

static wchar_t* ExtractDemangledName(IDiaSymbol* pSymbol)
{
    BSTR pName = NULL;

    if ((S_OK == pSymbol->get_undecoratedNameEx(UNDNAME_IA_STYLE, &pName) && SysStringLen(pName) > 0) ||
        S_OK == pSymbol->get_name(&pName))
    {
        unsigned lenMangledName = SysStringLen(pName);

        if (!IsDemangledName(pName, lenMangledName))
        {
            wchar_t demangledName[2048];
            lenMangledName = SymbolEngine::DemangleExternCNameVS(pName, lenMangledName, demangledName, 2048);

            if (0U != lenMangledName)
            {
                SysFreeString(pName);
                pName = SysAllocStringLen(demangledName, lenMangledName);
            }
        }
    }

    return pName;
}

static bool IsDemangledName(const wchar_t* pName, unsigned len)
{
    bool ret = false;

    // The minimum length is 3: "x()"
    if (3U <= len)
    {
        if (!(L'0' <= pName[len - 1U] && pName[len - 1U] <= L'9') && L'@' != pName[0])
        {
            const wchar_t* pChar = pName + len;
            pName++;

            while (--pChar != pName)
            {
                if (L')' == *pChar)
                {
                    ret = true;
                    break;
                }

                if (L'@' == *pChar)
                {
                    break;
                }
            }
        }
    }

    return ret;
}

static size_t CountPaths(const wchar_t* pPathList, LimitsBundle* pPathListLimits, unsigned sizePathListLimits, size_t& len)
{
    size_t count = 0;
    const wchar_t* pNextPath;
    const wchar_t* pPath = pPathList;

    while (NULL != (pNextPath = wcschr(pPath, L';')))
    {
        if (pPath != pNextPath)
        {
            if (count < sizePathListLimits)
            {
                pPathListLimits->m_begin = static_cast<gtUInt16>(pPath - pPathList);
                pPathListLimits->m_end = static_cast<gtUInt16>(pNextPath - pPathList);
                ++pPathListLimits;
            }

            len += pNextPath - pPath;
            ++count;
        }

        pPath = pNextPath + 1;
    }

    if (L'\0' != *pPath)
    {
        pNextPath = wcschr(pPath, L'\0');

        if (count < sizePathListLimits)
        {
            pPathListLimits->m_begin = static_cast<gtUInt16>(pPath - pPathList);
            pPathListLimits->m_end = static_cast<gtUInt16>(pNextPath - pPathList);
            ++pPathListLimits;
        }

        len += pNextPath - pPath;
        ++count;
    }

    return count;
}

static const wchar_t* TrimPathDelimiters(const wchar_t* pPath, size_t& len)
{
    if (NULL != pPath)
    {
        // Skip lonely delimiters
        while (L';' == *pPath || isspace(*pPath))
        {
            ++pPath;
        }

        len = wcslen(pPath);

        // Trim lonely delimiters
        while (0 != len)
        {
            wchar_t wc = pPath[len - 1];

            if (!(L';' == wc || isspace(wc)))
            {
                break;
            }

            --len;
        }
    }

    return pPath;
}


static wchar_t* ConcatenateServerPath(wchar_t* pBuf,
                                      const wchar_t* pCache, size_t lenCache,
                                      const wchar_t* pServer, size_t lenServer)
{
    memcpy(pBuf, L"SRV*", sizeof(wchar_t) * 4);
    pBuf += 4;

    if (0 != lenCache)
    {
        memcpy(pBuf, pCache, sizeof(wchar_t) * lenCache);
        pBuf += lenCache;

        *pBuf++ = L'*';
    }

    memcpy(pBuf, pServer, sizeof(wchar_t) * lenServer);
    pBuf += lenServer;

    *pBuf++ = L';';
    return pBuf;
}

static void ConcatenateServerPaths(wchar_t* pBuf,
                                   const wchar_t* pCache, size_t lenCache,
                                   const wchar_t* pServers,
                                   const LimitsBundle* pLimits, unsigned numLimits)
{
    for (unsigned i = 0; i < numLimits; ++i)
    {
        gtUInt16 begin = pLimits[i].m_begin;
        size_t lenServer = pLimits[i].m_end - begin;
        pBuf = ConcatenateServerPath(pBuf, pCache, lenCache, pServers + begin, lenServer);
    }

    // Process all the paths in the list that are not in the Limits list
    pServers += pLimits[numLimits - 1].m_end;

    if (L'\0' != *pServers)
    {
        const wchar_t* pNextPath;
        const wchar_t* pPath = pServers + 1;

        while (NULL != (pNextPath = wcschr(pPath, L';')))
        {
            if (pPath != pNextPath)
            {
                size_t lenServer = pNextPath - pPath;
                pBuf = ConcatenateServerPath(pBuf, pCache, lenCache, pPath, lenServer);
            }

            pPath = pNextPath + 1;
        }

        if (L'\0' != *pPath)
        {
            pNextPath = wcschr(pPath, L'\0');
            size_t lenServer = pNextPath - pPath;
            pBuf = ConcatenateServerPath(pBuf, pCache, lenCache, pPath, lenServer);
        }
    }
}

static wchar_t* BuildSearchPath(wchar_t* pBaseBuf,
                                size_t lenBaseBuf,
                                const wchar_t* pSearchPath,
                                const wchar_t* pServerList,
                                const wchar_t* pCachePath)
{
    size_t lenSearch = 0;
    pSearchPath = TrimPathDelimiters(pSearchPath, lenSearch);

    wchar_t* pSearchBuf = pBaseBuf;
    *pSearchBuf = L'\0';

    const unsigned sizeServerListLimits = 16;
    LimitsBundle pServerListLimits[sizeServerListLimits];
    size_t lenServers = 0;

    size_t countServers;

    if (NULL != pServerList && L'\0' != *pServerList)
    {
        countServers = CountPaths(pServerList, pServerListLimits, sizeServerListLimits, lenServers);
    }
    else
    {
        countServers = 0;
    }

    if (0 != countServers)
    {
        size_t lenCache = (NULL != pCachePath) ? wcslen(pCachePath) : 0;

        // Layout: [ pSearchPath ";" (countServers * [ "SRV*" pCachePath "*" pServer ";" ]) ]
        // The last delimiter is replaced by '\0'
        size_t lenTotal = lenSearch + static_cast<size_t>(0 != lenSearch) +
                          (lenServers + countServers * (4 + lenCache + 1 + 1));

        if (lenTotal > lenBaseBuf)
        {
            pSearchBuf = new wchar_t[lenTotal];
        }

        if (NULL != pSearchBuf)
        {
            if (0 != lenSearch)
            {
                memcpy(pSearchBuf, pSearchPath, sizeof(wchar_t) * lenSearch);
                pSearchBuf[lenSearch] = L';';
                ++lenSearch;
            }

            unsigned numServerListLimits = static_cast<unsigned>(countServers);

            if (numServerListLimits > sizeServerListLimits)
            {
                numServerListLimits = sizeServerListLimits;
            }

            ConcatenateServerPaths(pSearchBuf + lenSearch,
                                   pCachePath, lenCache,
                                   pServerList,
                                   pServerListLimits, numServerListLimits);

            pSearchBuf[lenTotal - 1] = L'\0';
        }
    }
    else if (0 != lenSearch)
    {
        size_t lenTotal = lenSearch + 1;

        if (lenTotal > lenBaseBuf)
        {
            pSearchBuf = new wchar_t[lenTotal];
        }

        if (NULL != pSearchBuf)
        {
            memcpy(pSearchBuf, pSearchPath, sizeof(wchar_t) * lenSearch);
            pSearchBuf[lenSearch] = L'\0';
        }
    }

    return pSearchBuf;
}