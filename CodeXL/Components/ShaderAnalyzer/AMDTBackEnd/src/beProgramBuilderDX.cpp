#include <CElf.h>

// C++.
#include <algorithm>
#include <iterator>

// Local.
#include <AMDTBackEnd/Include/beD3DIncludeManager.h>
#include <AMDTBackEnd/Include/beProgramBuilderDX.h>
#include <AMDTBackEnd/Include/beUtils.h>

#include <DX10AsmInterface.h>
#include <DX10AsmBuffer.h>

// This is from ADL's include directory.
#include <ADLUtil.h>
#include <customer/oem_structures.h>
#include <DeviceInfoUtils.h>

#include <D3D10ShaderObject.h>
#include <sp3.h>
#include <sp3-int.h>

// Infra.
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

using namespace std;
using namespace D3D10ShaderObject;
using namespace xlt;

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// These string constants will be relocated to  a unified DB once the
// device data and meta-data handling mechanism is revised.
const char* DEVICE_NAME_TONGA    = "Tonga";
const char* DEVICE_NAME_ICELAND  = "Iceland";

// This function returns true if the given device is
// affected by the HW issue which forces an allocation of
// a fixed number of SGPRs.
static bool IsFixedSgprAlloc(const string& deviceName)
{
    // DX only: due to a HW bug, SGPR allocation for
    // Tonga and Iceland devices should be fixed (94 prior to
    // SC interface update, which is until driver 15.10 including,
    // and 96 after SC interface update, which is after driver 15.10)
    bool ret = (deviceName.compare(DEVICE_NAME_TONGA) == 0 ||
                deviceName.compare(DEVICE_NAME_ICELAND) == 0);
    return ret;
}

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************


HRESULT beProgramBuilderDX::AmdDxGsaCompileShaderWrapper(const struct _AmdDxGsaCompileShaderInput* shaderInput, struct _AmdDxGsaCompileShaderOutput* shaderOutput)
{
    HRESULT hr = S_OK;

    __try
    {
        hr = m_TheAMDDXXModule.AmdDxGsaCompileShader(shaderInput, shaderOutput);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return ~hr;
    }

    return hr;
}


beProgramBuilderDX::beProgramBuilderDX(void) : m_TheAMDDXXModule(AMDDXXModule::s_DefaultModuleName), m_TheD3DCompileModule(D3DCompileModule::s_DefaultModuleName), m_compiledElf()
{
    m_bIsInitialized = false;
}


beProgramBuilderDX::~beProgramBuilderDX(void)
{
    m_TheAMDDXXModule.UnloadModule();
}

beKA::beStatus beProgramBuilderDX::Initialize(const string& dxModuleToLoad/* = ""*/)
{
    beStatus beRet = beStatus_SUCCESS;

    // Clear the outputs of former builds (if there are any).
    ClearFormerBuildOutputs();

    // load AMD module
    if (!m_TheAMDDXXModule.IsLoaded())
    {
        // Notice: This message receives an extra "\n", since later in the call chain, one is removed. We do want to remove them for
        // the rest of the messages, so we only remove it for initialization messages:
        stringstream ss;
        ss << "Error: " << AMDDXXModule::s_DefaultModuleName << " module not loaded.\n\n" << endl;
        LogCallBack(ss.str());
        beRet = beStatus_AMDDXX_MODULE_NOT_LOADED;
    }

    // load ms module
    if (beRet == beStatus_SUCCESS)
    {
        bool isDllLoad = false;
        int errorCode = 0;

        if (dxModuleToLoad.length() > 0)
        {
            isDllLoad = m_TheD3DCompileModule.LoadModule(dxModuleToLoad, &errorCode);
        }
        else
        {
            isDllLoad = m_TheD3DCompileModule.LoadModule();
        }

        if (!m_TheD3DCompileModule.IsLoaded())
        {
            // Check if the additional search paths should be searched.
            bool shouldSearchAdditionalPaths = !isDllLoad &&
                                               (dxModuleToLoad.find(D3DCompileModule::s_DefaultModuleName) != string::npos);

            if (shouldSearchAdditionalPaths)
            {
                gtString moduleNameAsGtStr;
                moduleNameAsGtStr << D3DCompileModule::s_DefaultModuleName;

                // Try searching in the additional directories (if any).
                for (const string& path : m_loaderSearchDirectories)
                {
                    // Build the full path to the module.
                    gtString pathAsGtStr;
                    pathAsGtStr << path.c_str();

                    osFilePath searchPath;
                    searchPath.setFileDirectory(pathAsGtStr);
                    searchPath.setFileName(moduleNameAsGtStr);

                    // Try to load the module.
                    isDllLoad = m_TheD3DCompileModule.LoadModule(searchPath.asString().asASCIICharArray(), &errorCode);

                    if (isDllLoad)
                    {
                        // We are done.
                        break;
                    }
                }
            }

            if (!isDllLoad)
            {
                // Take the relevant module's name.
                const char* pModuleName = (dxModuleToLoad.length() > 0) ?
                                          dxModuleToLoad.c_str() : D3DCompileModule::s_DefaultModuleName;

                // We failed to load the module.
                // Notice: This message receives an extra "\n", since later in the call chain, one is removed. We do want to remove them for
                // the rest of the messages, so we only remove it for initialization messages:
                stringstream ss;
                ss << "Error: " << pModuleName << " module not loaded. Error = " << errorCode << ". Please use D3D compiler version 43 or above." << endl << endl;

                LogCallBack(ss.str());

                if (errorCode == ERROR_PROC_NOT_FOUND)
                {
                    // GetProcAddress failed. This means that the D3DCompiler DLL version is incompatible with CodeXL. Probably too old.
                    beRet = beStatus_D3DCompile_MODULE_NOT_SUPPORTED;
                }
                else if (errorCode == ERROR_FILE_NOT_FOUND || errorCode == ERROR_MOD_NOT_FOUND)
                {
                    // File was not found or module could not be loaded.
                    beRet = beStatus_D3DCompile_MODULE_NOT_LOADED;
                }
                else
                {
                    beRet = beStatus_D3DCompile_MODULE_NOT_LOADED;
                }
            }
        }
    }

    if (beRet == beStatus_SUCCESS)
    {
        // Go through the list of public devices, as received from the OpenCL runtime.
        std::vector<GDT_GfxCardInfo> cardList;

        for (const std::string& publicDevice : m_publicDeviceNames)
        {
            if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(publicDevice.c_str(), cardList))
            {
                m_DXDeviceTable.insert(m_DXDeviceTable.end(), cardList.begin(), cardList.end());
                cardList.clear();
            }
        }

        std::sort(m_DXDeviceTable.begin(), m_DXDeviceTable.end(), beUtils::GfxCardInfoSortPredicate);
    }

    if (beRet == beStatus_SUCCESS)
    {
        m_bIsInitialized = true;
    }

    return beRet;
}

beKA::beStatus beProgramBuilderDX::CompileHLSL(const string& programSource, const DXOptions& dxOptions)
{
    // Turn options.m_Defines into what D3DCompile expects.
    D3D_SHADER_MACRO* macros = new D3D_SHADER_MACRO[dxOptions.m_defines.size() + 1];
    macros[dxOptions.m_defines.size()].Name = NULL;
    macros[dxOptions.m_defines.size()].Definition = NULL;

    // Handle the defines.
    if (!dxOptions.m_defines.empty())
    {
        int i = 0;

        for (vector<pair<string, string> >::const_iterator it = dxOptions.m_defines.begin();
             it != dxOptions.m_defines.end();
             ++it, i++)
        {
            macros[i].Name = it->first.c_str();
            macros[i].Definition = it->second.c_str();
        }
    }

    // Turn the HLSL into byte code.
    ID3DBlob* pShader    = nullptr;
    ID3DBlob* pErrorMsgs = nullptr;

    // Handle custom includes.
    ID3DInclude* pIncludeMechanism = D3D_COMPILE_STANDARD_FILE_INCLUDE;

    if (!dxOptions.m_includeDirectories.empty())
    {
        // Extract the shader's directory.
        gtString shaderFileName;
        shaderFileName << dxOptions.m_FileName.c_str();
        osFilePath tmpFilePath(shaderFileName);

        gtString shaderDirStr;
        osDirectory shaderDir;
        bool isDirExtracted = tmpFilePath.getFileDirectory(shaderDir);

        if (isDirExtracted)
        {
            shaderDirStr = shaderDir.directoryPath().asString();

            // Create an include manager.
            pIncludeMechanism = new D3DIncludeManager(shaderDirStr.asASCIICharArray(), dxOptions.m_includeDirectories);
        }
    }

    // Invoke the offline compiler.
    HRESULT result = E_FAIL;

    try
    {
        result = m_TheD3DCompileModule.D3DCompile(
                     programSource.c_str(),
                     programSource.length(),
                     LPCSTR(dxOptions.m_FileName.c_str()),   // pFileName.
                     macros,                                 // D3D_SHADER_MACRO* pDefines.
                     pIncludeMechanism,                      // ID3DInclude* pInclude.
                     dxOptions.m_Entrypoint.c_str(),
                     dxOptions.m_Target.c_str(),
                     dxOptions.m_DXFlags.flagsAsInt,         // UINT flags1.
                     0,                                      // UINT flags2.
                     &pShader,
                     &pErrorMsgs);
    }
    catch (...)
    {
        stringstream ss;
        ss << "Error: Exception in D3DCompiler. Could be wrong dll version?\n";
        LogCallBack(ss.str());
        return beStatus_D3DCompile_FAILED;
    }

    // Release resources.
    delete[] macros;

    if (pErrorMsgs != NULL)
    {
        char* errorString = (char*)pErrorMsgs->GetBufferPointer();
        size_t error_size = pErrorMsgs->GetBufferSize();
        stringstream ss;
        ss << string(errorString, error_size);
        LogCallBack(ss.str());
        pErrorMsgs->Release();
    }

    if (result != S_OK)
    {
        if (pShader != NULL)
        {
            pShader->Release();
        }

        return beStatus_D3DCompile_FAILED;
    }

    // For DX10 & DX11, we need to peel off the Microsoft headers.
    // This will get us to the executable bit that the RTL usually passes
    // down to the driver.  That's what the code below wants.
    // For DX9, the object has no header (it's all byte code).
    // But for DX9, we need to talk to a different driver and that's NYI.
    char* shaderBytes = (char*)pShader->GetBufferPointer();
    size_t shaderBytesLen = pShader->GetBufferSize();
    string sShader(shaderBytes, shaderBytesLen);

    // If we haven't done this so far for this compilation,
    // disassemble D3D offline compiler's output.
    if (dxOptions.m_bDumpMSIntermediate && m_msIntermediateText.empty())
    {
        ID3DBlob* pDisassembly = NULL;
        result = m_TheD3DCompileModule.D3DDisassemble(shaderBytes, shaderBytesLen, 0, "", &pDisassembly);

        if (result == S_OK)
        {
            shaderBytes = (char*)pDisassembly->GetBufferPointer();
            shaderBytesLen = pDisassembly->GetBufferSize();
            m_msIntermediateText = string(shaderBytes, shaderBytesLen);
        }
        else
        {
            // if failed- just report and continue
            LogCallBack("Failed to Disassemble the Blob into text");
        }
    }

    return CompileDXAsm(sShader, dxOptions);

}

beKA::beStatus beProgramBuilderDX::CompileDXAsm(const string& programSource, const DXOptions& dxOptions)
{
    // check if valid Blob first!
    D3D10_ShaderObjectHeader* pShaderHeader = (D3D10_ShaderObjectHeader*)programSource.c_str();

    if (pShaderHeader->dwSize != programSource.size())
    {
        return beStatus_Create_Bolob_FromInput_Failed;
    }

    CD3D10ShaderObject MSBlob(programSource.c_str(), programSource.length());

    const D3D10_ChunkHeader* byteCodeHeader;
    //// SHDR for DX10, SHEX for DX11.
    byteCodeHeader = MSBlob.GetSHDRChunk() != NULL ? MSBlob.GetSHDRChunk() : MSBlob.GetSHEXChunk();

    if (byteCodeHeader == NULL)
    {
        stringstream ss;
        ss << "Could not extract bytecode from compiled shader.\n";
        LogCallBack(ss.str());
        return beStatus_BYTE_CODE_EXTRACT_FAILED;
    }

    AmdDxGsaCompileShaderInput shaderInput;
    memset(&shaderInput, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compileOptions[1];
    memset(compileOptions, 0, sizeof(AmdDxGsaCompileOption));

    shaderInput.chipFamily = dxOptions.m_ChipFamily;
    shaderInput.chipRevision = dxOptions.m_ChipRevision;
    // The code directly follows the header.
    shaderInput.pShaderByteCode = (char*)(byteCodeHeader + 1);
    shaderInput.byteCodeLength = byteCodeHeader->dwChunkDataSize;
    shaderInput.pCompileOptions = compileOptions;
    shaderInput.numCompileOptions = 0;

    AmdDxGsaCompileShaderOutput shaderOutput;
    memset(&shaderOutput, 0, sizeof(AmdDxGsaCompileShaderOutput));

    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shaderOutput.size = sizeof(AmdDxGsaCompileShaderOutput);
    // For good measure, zero the output pointer.
    shaderOutput.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shaderInput, &shaderOutput);

    if (result != S_OK)
    {
        stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << endl;
        LogCallBack(ss.str());

        if (shaderOutput.pShaderBinary != NULL)
        {
            m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);
        }

        return beStatus_AmdDxGsaCompileShader_FAILED;
    }

    // Open the binaries as CElf objects.
    vector<char> elfBinary((char*)shaderOutput.pShaderBinary, (char*)shaderOutput.pShaderBinary + shaderOutput.shaderBinarySize);
    CElf* pCurrElf = new CElf(elfBinary);

    if (pCurrElf->good())
    {
        SetDeviceElf(dxOptions.m_deviceName, shaderOutput);
    }
    else
    {
        // Report about the failure.
        stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallBack(ss.str());

        // Release the resources.
        m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

        return beStatus_NO_BINARY_FOR_DEVICE;
    }

    // Free stuff.
    m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

    return beStatus_SUCCESS;
}

beKA::beStatus beProgramBuilderDX::Compile(beKA::SourceLanguage sourceLanguage, const string& programSource, const DXOptions& dxOptions)
{
    if (!m_bIsInitialized)
    {
        stringstream ss;
        ss << "DX Module not initialized";
        LogCallBack(ss.str());
        return beStatus_D3DCompile_MODULE_NOT_LOADED;
    }

    beStatus beRet = beStatus_General_FAILED;

    if (sourceLanguage == SourceLanguage_HLSL)
    {
        beRet = CompileHLSL(programSource, dxOptions);
    }
    else if (sourceLanguage == SourceLanguage_DXasm)
    {
        beRet = CompileDXAsm(programSource, dxOptions);
    }
    else if (sourceLanguage == SourceLanguage_DXasmT)
    {
        beRet = CompileDXAsmT(programSource, dxOptions);
    }
    else
    {
        stringstream ss;
        ss << "Source language not supported";
        LogCallBack(ss.str());
    }

    return beRet;
}

beKA::beStatus beProgramBuilderDX::GetKernels(const string& device, vector<string>& kernels)
{
    (void)(&device); // Unreferenced parameter
    (void)(&kernels); // Unreferenced parameter

    return beKA::beStatus_SUCCESS;
}

beKA::beStatus beProgramBuilderDX::GetBinary(const string& device, const beKA::BinaryOptions& binopts, vector<char>& binary)
{

    GT_UNREFERENCED_PARAMETER(binopts);
    binary = GetDeviceBinaryElf(device);

    return beKA::beStatus_SUCCESS;
}


beKA::beStatus beProgramBuilderDX::GetISABinary(const string& device, vector<char>& binary)
{
    const CElfSection* pTextSection = GetISATextSection(device);
    beStatus result = beStatus_Invalid;

    if (nullptr != pTextSection)
    {
        result = beStatus_SUCCESS;
        binary = pTextSection->GetData();
    }

    return result;

}

beKA::beStatus beProgramBuilderDX::GetKernelILText(const string& device, const string& kernel, string& il)
{
    // For DX shaders, we currently cannot disassemble the IL binary section.
    // For now, we extract D3D ASM code.
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    il.clear();
    beStatus beRet = GetIntermediateMSBlob(il);
    return beRet;
}

beKA::beStatus beProgramBuilderDX::GetKernelISAText(const string& device, const string& shaderName, string& isa)
{
    // Not implemented: see beProgramBuilderDX::GetDxShaderD3DASM.
    // The current inheritance architecture where beProgramBuilderDX and
    // beProgramBuilderCL share the same interface for ISA extraction does not hold.
    // This mechanism will be refactored.
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(shaderName);
    GT_UNREFERENCED_PARAMETER(isa);
    return beStatus_Invalid;
}

beKA::beStatus beProgramBuilderDX::GetStatistics(const string& device, const string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(kernel);

    beStatus ret = beKA::beStatus_General_FAILED;

    // There is no symbol table.  We just get the .stats  section.
    CElf* pElf = GetDeviceElf(device);

    if (pElf != nullptr)
    {
        // Get the .stats section.
        CElfSection* pStatsSection = pElf->GetSection(".stats");

        if (pStatsSection != nullptr)
        {
            // This is the binary image.
            const vector<char>& sectionData = pStatsSection->GetData();

            const AmdDxGsaCompileStats* pStats = reinterpret_cast<const AmdDxGsaCompileStats*>(sectionData.data());

            if (pStats != nullptr)
            {
                (void)memset(&analysis, 0, sizeof(analysis));

                if (!IsFixedSgprAlloc(device))
                {
                    analysis.numSGPRsUsed = pStats->numSgprsUsed;
                }
                else
                {
                    // Assign the fixed value. This is due to a HW bug. The fixed values are: 94 (excluding
                    // the 2 VCC registers) until driver 15.10 (including), and 96 afterwards (to be handled).
                    const unsigned SGPRs_USED_TONGA_ICELAND = 94;
                    analysis.numSGPRsUsed = SGPRs_USED_TONGA_ICELAND;
                }

                analysis.numSGPRsAvailable = pStats->availableSgprs;
                analysis.numVGPRsUsed = pStats->numVgprsUsed;
                analysis.numVGPRsAvailable = pStats->availableVgprs;
                analysis.LDSSizeUsed = pStats->usedLdsBytes;
                analysis.LDSSizeAvailable = pStats->availableLdsBytes;
                analysis.maxScratchRegsNeeded = pStats->usedScratchBytes;
                analysis.numAluInst = pStats->numAluInst;
                analysis.numControlFlowInst = pStats->numControlFlowInst;
                analysis.numTfetchInst = pStats->numTfetchInst;

                // We are done.
                ret = beKA::beStatus_SUCCESS;
            }
        }
        else
        {
            ret = beStatus_NoStatSectionInElfPossibleOldDxDriver;
        }
    }

    return ret;
}

bool beProgramBuilderDX::IsInitialized()
{
    return beKA::beStatus_SUCCESS;
}


void beProgramBuilderDX::ClearFormerBuildOutputs()
{
    // Clear the ELF sections.
    for (auto& devElfPair : m_compiledElf)
    {
        CelfBinaryPair& celfCelfBinaryPair = devElfPair.second;
        delete celfCelfBinaryPair.first;
        celfCelfBinaryPair.first = nullptr;
        celfCelfBinaryPair.second.clear();
    }

    m_compiledElf.clear();

    // Clear the DSASM text.
    m_msIntermediateText.clear();
}

void beProgramBuilderDX::ReleaseProgram()
{
    ClearFormerBuildOutputs();
}

beKA::beStatus beProgramBuilderDX::GetDeviceTable(vector<GDT_GfxCardInfo>& table)
{
    table = m_DXDeviceTable;
    return beStatus_SUCCESS;
}

bool beProgramBuilderDX::CompileOK(string& device)
{
    (void)(&device); // Unreferenced parameter

    return true;
}

/////////////////////////////////////////////////////
// File scoped global declarations
/////////////////////////////////////////////////////
static bool g_bWroteFiles = false;
// flag to denote whether "SHDR" token needs to be added
// to the output binary dump file.
bool g_bSHDR = false;
unsigned long dwSHDRToken = 0x52444853; /* "SHDR" */
bool g_bBinIsHex = false;
static string g_szTxtFile;
static string g_szBinFile;
static string g_szFullBinFile;
static string g_szRegFile;
static string g_szHexFile;
static string g_szFullHexFile;
static string g_szConversion;
static string test_asm;
static string test_hlsl;
static int g_hlsl_flag = 0;
bool g_bNoDebug = false;

static FILE* g_fp = NULL;
/////////////////////////////////////////////////////
// Callback functions
/////////////////////////////////////////////////////
void* XLT_STDCALL allocSysMem(void* pHandle, unsigned int dwSizeInBytes)
{
    (void)(pHandle); // Unreferenced parameter

    return malloc(dwSizeInBytes);
}

void XLT_STDCALL freeSysMem(void* pHandle, void* lpAddress)
{
    (void)(pHandle); // Unreferenced parameter

    free(lpAddress);
}

int XLT_STDCALL outputString(XLT_PVOID pHandle, const char* pTranslatedString, ...)
{
    (void)(pHandle); // Unreferenced parameter

    char pszFormattedBuffer[1024];
    va_list va;

    va_start(va, pTranslatedString);
    vsprintf(pszFormattedBuffer, pTranslatedString, va);
    va_end(va);

    //g_szResultTxtFile += pszFormattedBuffer;

    return 0;
}

int XLT_STDCALL outputBinary(XLT_PVOID pHandle,
                             const void* pszTranslatedProgram,
                             unsigned int nTranslatedProgramSize)
{
    (void)(pHandle); // Unreferenced parameter

    return 0;

    xlt::DX10AsmBuffer output;

    // original code (don't need it):
#if 0
    int nDwords = nTranslatedProgramSize / 4;
    unsigned long* ptr = (unsigned long*)pszTranslatedProgram;

    print_info("-------- DX10 Binary ----------\n");

    if (g_bSHDR)
    {
        print_info("0x%08X\n", dwSHDRToken);
    }

    for (int i = 0; i < nDwords; i++)
    {
        print_info("0x%08X\n", *ptr);
        ptr++;
    }

    print_info("-----------------------------\n");
#endif

    output.attach((char*)(const_cast<void*>(pszTranslatedProgram)),
                  nTranslatedProgramSize);

    // output.save( g_szDstFile.c_str() );
    if ((nTranslatedProgramSize != 0) && (pszTranslatedProgram != NULL))
    {
        ofstream ofs;

        ofs.open(g_szBinFile.c_str(), ios::binary | ios::out);

        if (!ofs.is_open())
        {
            // print_info("cannnot open %s\n",
            //   g_szBinFile.c_str());
            return -1;
        }

        if (g_bSHDR)
        {
            unsigned long dwSHDRToken1 = 0x6C04000; /* version 4.0 */
            ofs.write((char*)&dwSHDRToken, 4);
            ofs.write((char*)&dwSHDRToken1, 4);
        }

        ofs.write((char*)pszTranslatedProgram, nTranslatedProgramSize);
        ofs.close();

        // This code was commented and cleaned up as part of static analysis.
        // TODO: Uncomment if desired for debug purposes.
        //if (g_bSHDR)
        //{
        //    // Add the 'SHDR' DWORD size
        //    nTranslatedProgramSize += 4;
        //}

        //if (g_bNoDebug == false)
        //{
        //    print_info("Wrote %d bytes to %s\n", nTranslatedProgramSize, g_szBinFile.c_str());
        //}

        return 0;
    }
    else
    {
        printf("Invalid Buffer, unable to write to file\n");

        return -1;
    }//end if else
}

string* beProgramBuilderDX::s_pTranslatedProgram;
int* beProgramBuilderDX::s_pipTranslatedProgramSize;

int XLT_STDCALL outputBinaryFull(XLT_PVOID pHandle,
                                 const void* pszTranslatedProgram,
                                 unsigned int nTranslatedProgramSize)
{
    (void)(pHandle); // Unreferenced parameter

    string temp((char*)(const_cast<void*>(pszTranslatedProgram)), nTranslatedProgramSize);
    (*beProgramBuilderDX::s_pTranslatedProgram) = temp;
    *beProgramBuilderDX::s_pipTranslatedProgramSize = (int)nTranslatedProgramSize;

    return 1;

    // continue here if want to save the file

    xlt::DX10AsmBuffer output;

#if 0
    int nDwords = nTranslatedProgramSize / 4;
    unsigned long* ptr = (unsigned long*)pszTranslatedProgram;

    print_info("-------- DX10 Binary ----------\n");

    if (g_bSHDR)
    {
        print_info("0x%08X\n", dwSHDRToken);
    }

    for (int i = 0; i < nDwords; i++)
    {
        print_info("0x%08X\n", *ptr);
        ptr++;
    }

    print_info("-----------------------------\n");
#endif

    output.attach((char*)(const_cast<void*>(pszTranslatedProgram)),
                  nTranslatedProgramSize);

    // output.save( g_szDstFile.c_str() );
    if ((nTranslatedProgramSize != 0) && (pszTranslatedProgram != NULL))
    {
        if (!g_szFullBinFile.empty())
        {
            ofstream ofs;

            ofs.open(g_szFullBinFile.c_str(), ios::binary | ios::out);

            if (!ofs.is_open())
            {
                //print_info("cannnot open %s\n",
                //    g_szFullBinFile.c_str());
                return -1;
            }

            ofs.write((char*)pszTranslatedProgram, nTranslatedProgramSize);
            ofs.close();

            /*
            if ( g_bNoDebug == false ) print_info( "Wrote %d bytes to %s\n", nTranslatedProgramSize, g_szFullBinFile.c_str() );
            */
        }

        if (!g_szFullHexFile.empty())
        {
            ofstream ofs;

            ofs.open(g_szFullHexFile.c_str(), ios::binary | ios::out);

            if (!ofs.is_open())
            {
                //print_info("cannnot open %s\n",
                //    g_szFullHexFile.c_str());
                return -1;
            }

            DWORD* pdwTranslatedProgram = (DWORD*)pszTranslatedProgram;
            DWORD dwTranslatedProgramSize = nTranslatedProgramSize / sizeof(DWORD);

            for (DWORD i = 0; i < dwTranslatedProgramSize; i++)
            {
                char szText[13];
                sprintf(szText, "0x%08x, ", *pdwTranslatedProgram++);
                ofs << szText;

                if ((i & 3) == 3)
                {
                    ofs << "\n";
                }
            }

            ofs.close();

            /*
            if ( g_bNoDebug == false ) print_info( "Wrote %d bytes to %s\n", nTranslatedProgramSize, g_szFullBinFile.c_str() );
            */
        }

        return 0;
    }
    else
    {
        printf("Invalid Buffer, unable to write to file\n");

        return -1;
    }//end if else
}

/// end callbacks


beKA::beStatus beProgramBuilderDX::CompileDXAsmT(const string& programSource, const DXOptions& dxOptions)
{
    // lets convert the source, which is DX ASM as text to DX ASM binary which is what DX driver expects.
    XLT_PROGINFO xltInput;
    XLT_CALLBACKS xltCallbacks;

    memset(&xltCallbacks, 0, sizeof(XLT_CALLBACKS));

    xltCallbacks.eXltMode = E_XLT_NORMAL;
    xltCallbacks.pHandle = NULL;
    xltCallbacks.AllocateSysMem = allocSysMem;
    xltCallbacks.FreeSysMem = freeSysMem;
    xltCallbacks.OutputString = outputString;
    xltCallbacks.OutputBinary = outputBinary;
    xltCallbacks.OutputBinaryFull = outputBinaryFull;
    xltCallbacks.Assert = NULL;
    xltCallbacks.flags = g_hlsl_flag;
    xltInput.pBuffer = (char*)programSource.c_str();
    xltInput.nBufferSize = (int)programSource.length() + 1;

    char*  pTheBinBuffer;
    string stemp;
    int nBufferSize = 0;
    unsigned int stempsize;

    s_pipTranslatedProgramSize = &nBufferSize;
    s_pTranslatedProgram = &stemp;

    DX10AsmText2Stream(&xltInput, &xltCallbacks, pTheBinBuffer, stempsize);

    // const D3D10_ChunkHeader* byteCodeHeader = ( D3D10_ChunkHeader* ) pTheBinBuffer;

    AmdDxGsaCompileShaderInput shaderInput;
    memset(&shaderInput, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compileOptions[1];
    memset(compileOptions, 0, sizeof(AmdDxGsaCompileOption));

    shaderInput.chipFamily = dxOptions.m_ChipFamily;
    shaderInput.chipRevision = dxOptions.m_ChipRevision;
    shaderInput.pShaderByteCode = (char*)(pTheBinBuffer);  // The code directly follows the header.
    shaderInput.byteCodeLength = stempsize;
    shaderInput.pCompileOptions = compileOptions;
    shaderInput.numCompileOptions = 0;

    AmdDxGsaCompileShaderOutput shaderOutput;
    memset(&shaderOutput, 0, sizeof(AmdDxGsaCompileShaderOutput));
    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shaderOutput.size = sizeof(AmdDxGsaCompileShaderOutput);
    // For good measure, zero the output pointer.
    shaderOutput.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shaderInput, &shaderOutput);

    if (result != S_OK)
    {
        stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << endl;
        LogCallBack(ss.str());

        if (shaderOutput.pShaderBinary != NULL)
        {
            m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);
        }

        return beStatus_AmdDxGsaCompileShader_FAILED;
    }

    // Crack open the binaries as CElf objects.
    vector<char> elfBinary((char*)shaderOutput.pShaderBinary, (char*)shaderOutput.pShaderBinary + shaderOutput.shaderBinarySize);
    CElf* pElf = new CElf(elfBinary);

    if (pElf->good())
    {
        // Store the ELF section in the relevant container.
        SetDeviceElf(dxOptions.m_deviceName, shaderOutput);
    }
    else
    {
        // Inform the user about the failure.
        stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallBack(ss.str());

        // Release the resources.
        m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

        return beStatus_NO_BINARY_FOR_DEVICE;
    }

    // Free stuff.
    m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

    return beStatus_SUCCESS;

}

beKA::beStatus beProgramBuilderDX::GetIntermediateMSBlob(string& IntermediateMDBlob)
{
    IntermediateMDBlob = m_msIntermediateText;
    return beStatus_SUCCESS;
}

void beProgramBuilderDX::SetIntermediateMSBlob(const string& intermediateMSCode)
{
    m_msIntermediateText = intermediateMSCode;
}

void beProgramBuilderDX::AddDxSearchDir(const string& dir)
{
    if (find(m_loaderSearchDirectories.begin(),
             m_loaderSearchDirectories.end(), dir) == m_loaderSearchDirectories.end())
    {
        m_loaderSearchDirectories.push_back(dir);
    }
}

static int GetShaderType(const string& target)
{
    int shaderType = SP3_SHTYPE_NONE;

    if (target.compare("ps_") != string::npos)
    {
        shaderType = SP3_SHTYPE_PS;
    }
    else if (target.compare("vs_") != string::npos)
    {
        shaderType = SP3_SHTYPE_VS;
    }
    else if (target.compare("cs_") != string::npos)
    {
        shaderType = SP3_SHTYPE_CS;
    }
    else if (target.compare("gs_") != string::npos)
    {
        shaderType = SP3_SHTYPE_GS;
    }
    else if (target.compare("es_") != string::npos)
    {
        shaderType = SP3_SHTYPE_ES;
    }
    else if (target.compare("hs_") != string::npos)
    {
        shaderType = SP3_SHTYPE_HS;
    }
    else if (target.compare("ls_") != string::npos)
    {
        shaderType = SP3_SHTYPE_LS;
    }

    return shaderType;
}

const CElfSection* beProgramBuilderDX::GetISATextSection(const string& deviceName) const
{
    // Get the relevant ELF section for the required device.
    const CElfSection* result = nullptr;
    CElf* pElf = GetDeviceElf(deviceName);

    if (pElf != nullptr)
    {
        // There is no symbol table.  We just need the .text section.
        const string CODE_SECTION_NAME(".text");
        result = pElf->GetSection(CODE_SECTION_NAME);
    }

    return result;
}

const CElfSection* beProgramBuilderDX::GetILSection(const std::string& deviceName) const
{
    // Get the relevant ELF section for the required device.
    const CElfSection* result = nullptr;
    CElf* pElf = GetDeviceElf(deviceName);

    if (pElf != nullptr)
    {
        // There is no symbol table.  We just need the .text section.
        const string IL_SECTION_NAME(".amdil");
        result = pElf->GetSection(IL_SECTION_NAME);
    }

    return result;
}

beKA::beStatus beProgramBuilderDX::GetDxShaderISAText(const string& deviceName, const string& shader, const string& target, string& isaBuffer)
{
    beKA::beStatus ret = beStatus_Invalid;

    // Get the relevant ELF section for the required device.
    const CElfSection* pTextSection = GetISATextSection(deviceName);

    if (pTextSection != nullptr)
    {
        // This is the binary image.
        const vector<char>& sectionData = pTextSection->GetData();

        // Use the sp3 library to do the disassembly.
        // This only works with SI & CI for now.
        // See on that matter: sc/Src/R1000/R1000Disassembler.cpp.
        struct sp3_context* pDisasmState = sp3_new();
        char* pDisassembledShader = nullptr;
        sp3_vma* pVm = nullptr;

        // Set the hardware generation.
        for (const GDT_GfxCardInfo& cardInfo : m_DXDeviceTable)
        {
            if (cardInfo.m_szCALName == deviceName)
            {
                switch (cardInfo.m_generation)
                {
                    case GDT_HW_GENERATION_SOUTHERNISLAND:
                        sp3_setasic(pDisasmState, "SI");
                        break;

                    case GDT_HW_GENERATION_SEAISLAND:
                        sp3_setasic(pDisasmState, "CI");
                        break;

                    case GDT_HW_GENERATION_VOLCANICISLAND:
                        sp3_setasic(pDisasmState, "VI");
                        break;

                    default:
                        // Should not happen.
                        GT_ASSERT_EX(false, L"Unknown HW generation.");
                        sp3_setasic(pDisasmState, "SI");
                        break;
                }
            }
        }

        // Prepare the tokens.
        unsigned* pIsaTokens = (unsigned*)&sectionData[0];

        if (pIsaTokens != nullptr)
        {
            size_t isaTokensLen = sectionData.size() / 4;
            pVm = sp3_vm_new_ptr(0, isaTokensLen, pIsaTokens);

            // Prepare the flags.
            unsigned disasmFlags = 0;
            disasmFlags |= SP3DIS_FORCEVALID;

            // Get the shader type.
            const int shaderType = GetShaderType(target);

            // Disassemble the code.
            pDisassembledShader = sp3_disasm(pDisasmState, pVm, 0, shader.c_str(),
                                             shaderType, NULL, (unsigned)isaTokensLen, disasmFlags);

            if (pDisassembledShader != nullptr)
            {

                // Fill the buffer.
                isaBuffer = string(pDisassembledShader);

                // We are done.
                ret = beKA::beStatus_SUCCESS;
            }
        }

        // Release the memory.
        sp3_close(pDisasmState);
        UsePlatformNativeLineEndings(isaBuffer);
        sp3_free(pDisassembledShader);
        sp3_vm_free(pVm);
    }

    else
    {
        ret = beStatus_NO_ISA_FOR_DEVICE;
    }

    return ret;
}

beKA::beStatus beProgramBuilderDX::GetDxShaderIL(const std::string& device, const std::string& shader,
    const std::string& target, std::string& ilBuffer)
{
    beKA::beStatus ret = beStatus_Invalid;

    // Get the relevant ELF section for the required device.
    const CElfSection* pAmdilSection = GetILSection(device);

    if (pAmdilSection != nullptr)
    {
        // This is the binary image.
        const vector<char>& sectionData = pAmdilSection->GetData();

        // Use the sp3 library to do the disassembly.
        // This only works with SI & CI for now.
        // See on that matter: sc/Src/R1000/R1000Disassembler.cpp.
        struct sp3_context* pDisasmState = sp3_new();
        char* pDisassembledShader = nullptr;
        sp3_vma* pVm = nullptr;

        // Prepare the tokens.
        unsigned* pIlBytes = (unsigned*)&sectionData[0];

        if (pIlBytes != nullptr)
        {
            size_t ilBytesLen = sectionData.size() / 4;
            pVm = sp3_vm_new_ptr(0, ilBytesLen, pIlBytes);

            // Prepare the flags.
            unsigned disasmFlags = 0;
            disasmFlags |= SP3DIS_FORCEVALID;

            // Get the shader type.
            const int shaderType = GetShaderType(target);

            // Disassemble the code.
            pDisassembledShader = sp3_disasm(pDisasmState, pVm, 0, shader.c_str(),
                shaderType, NULL, (unsigned)ilBytesLen, disasmFlags);

            if (pDisassembledShader != nullptr)
            {

                // Fill the buffer.
                ilBuffer = string(pDisassembledShader);

                // We are done.
                ret = beKA::beStatus_SUCCESS;
            }
        }

        // Release the memory.
        sp3_close(pDisasmState);
        UsePlatformNativeLineEndings(ilBuffer);
        sp3_free(pDisassembledShader);
        sp3_vm_free(pVm);
    }

    else
    {
        ret = beStatus_NO_ISA_FOR_DEVICE;
    }

    return ret;
}

bool beProgramBuilderDX::GetIsaSize(const string& isaAsText, size_t& sizeInBytes) const
{
    // The length in characters of a 32-bit instruction in text format.
    const size_t INSTRUCTION_32_LENGTH = 8;
    const size_t INSTRUCTION_32_SIZE_IN_BYTES = 4;

    // The length in characters of a 64-bit instruction in text format.
    const size_t INSTRUCTION_64_LENGTH = 16;
    const size_t INSTRUCTION_64_SIZE_IN_BYTES = 8;

    bool ret = false;
    sizeInBytes = 0;

    size_t posBegin = isaAsText.rfind("//");

    if (posBegin != string::npos)
    {
        size_t posFirst32BitEnd = isaAsText.find(':', posBegin);

        // Get past the "// " prefix.
        posBegin += 3;

        // Determine the length of the PC string.
        size_t pcLength = (posFirst32BitEnd - posBegin);

        if (posFirst32BitEnd != string::npos &&  pcLength > 0)
        {
            GT_IF_WITH_ASSERT(posBegin + pcLength < isaAsText.size())
            {
                // Get the first 32 bit of the final instruction.
                const string& instrFirst32bit = isaAsText.substr(posBegin, pcLength);

                // Convert the PC of the final instruction. This will indicate
                // how many bytes we used until the final instruction (excluding
                // the final instruction).
                sizeInBytes = stoul(instrFirst32bit, nullptr, 16);

                // Get past the prefix.
                posFirst32BitEnd += 2;

                // Find the end of the final instruction line.
                size_t posEnd = isaAsText.find("\r\n", posBegin);

                if (posEnd != string::npos && posFirst32BitEnd < posEnd)
                {
                    // Extract the final instruction as text.
                    const string& instructionAsText = isaAsText.substr(posFirst32BitEnd, posEnd - posFirst32BitEnd);
                    const size_t numOfCharactersInInstr = instructionAsText.size();

                    if (numOfCharactersInInstr == INSTRUCTION_32_LENGTH)
                    {
                        // The final instruction is a 32-bit instruction.
                        sizeInBytes += INSTRUCTION_32_SIZE_IN_BYTES;
                        ret = true;
                    }
                    else if (numOfCharactersInInstr == (INSTRUCTION_64_LENGTH + 1))
                    {
                        // The final instruction is a 64-bit instruction.
                        sizeInBytes += INSTRUCTION_64_SIZE_IN_BYTES;
                        ret = true;
                    }
                    else
                    {
                        // We shouldn't get here.
                        GT_ASSERT_EX(false, L"Unknown instruction size");
                    }
                }
            }
        }
    }

    return ret;
}

bool beProgramBuilderDX::GetWavefrontSize(const string& deviceName, size_t& wavefrontSize) const
{
    bool ret = false;
    wavefrontSize = 0;

    // Extract the device info.
    GDT_DeviceInfo s_deviceInfo;

    if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), s_deviceInfo))
    {
        wavefrontSize = s_deviceInfo.m_nWaveSize;
        ret = true;
    }

    return ret;
}


string beProgramBuilderDX::ToLower(const string& str) const
{
    string result;
    transform(str.begin(), str.end(),
              inserter(result, result.begin()), ::tolower);
    return result;
}

void beProgramBuilderDX::SetDeviceElf(const string& deviceName, const AmdDxGsaCompileShaderOutput& shaderOutput)
{
    if (!deviceName.empty())
    {
        // First, convert the device name to lower case.
        string deviceNameLowerCase = ToLower(deviceName);

        // Update the container.
        CelfBinaryPair& celfpair = m_compiledElf[deviceNameLowerCase];

        celfpair.second.assign((char*)shaderOutput.pShaderBinary, (char*)shaderOutput.pShaderBinary + shaderOutput.shaderBinarySize);
        // Open the binaries as CElf objects.
        celfpair.first = new CElf(celfpair.second);
    }
}

bool beProgramBuilderDX::GetDeviceElfBinPair(const string& deviceName, CelfBinaryPair& elfBinPair) const
{
    bool result =  false;


    if (!deviceName.empty())
    {
        // First, convert the device name to lower case.
        string deviceNameLowerCase = ToLower(deviceName);

        // Look for the relevant element.
        auto iter = m_compiledElf.find(deviceNameLowerCase);

        if (iter != m_compiledElf.end())
        {
            elfBinPair = iter->second;
            result = true;
        }
    }

    return result;
}

CElf* beProgramBuilderDX::GetDeviceElf(const string& deviceName) const
{
    CElf* pRet = nullptr;
    CelfBinaryPair elfBinPair;

    if (GetDeviceElfBinPair(deviceName, elfBinPair))
    {
        pRet = elfBinPair.first;
    }

    return pRet;
}

vector<char> beProgramBuilderDX::GetDeviceBinaryElf(const string& deviceName) const
{
    vector<char> result;
    CelfBinaryPair elfBinPair;

    if (GetDeviceElfBinPair(deviceName, elfBinPair))
    {
        result = elfBinPair.second;
    }

    return result;
}

void beProgramBuilderDX::SetPublicDeviceNames(const std::set<std::string>& publicDeviceNames)
{
    m_publicDeviceNames = publicDeviceNames;
}
