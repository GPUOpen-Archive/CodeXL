//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the retrieval of CL kernel source, IL,
///        ISA from the CL run-time.
//==============================================================================

#include <sstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <CElf.h>

// ADL headers
#include <ADLUtil.h>
#include <customer/oem_structures.h>

#include "../Common/StringUtils.h"
#include "../Common/FileUtils.h"
#include "../Common/Logger.h"
#include "DeviceInfoUtils.h"
#include "ACLModuleManager.h"
#include "CLKernelAssembly.h"
#include "CLUtils.h"
#include "CLFunctionDefs.h"

using namespace CLUtils;
using namespace std;
using namespace GPULogger;

std::string KernelAssembly::m_sTmpDisassemblyLoggerISA;
std::string KernelAssembly::m_sTmpDisassemblyLoggerHSAIL;
unsigned int KernelAssembly::m_sDisassembleCount;

const unsigned int MAX_EXPECTED_DISASSEMBLE_CALLBACKS = 2;


static std::string g_strISA; ///< the temporary ISA text returned by the CAL logger func

static const unsigned long KERNEL_DEVICE_INFO_BUFFER_SIZE = 512;

KernelAssembly::KernelAssembly() : m_strFilePrefix(KERNEL_ASSEMBLY_FILE_PREFIX),
    m_bOutputIL(false),
    m_bOutputISA(false),
    m_bOutputCL(false),
    m_bOutputHSAIL(false),
    m_bInitCAL(false)
{
};

KernelAssembly::~KernelAssembly()
{
    ACLModuleManager::Instance()->UnloadAllACLModules();
}

bool KernelAssembly::GetProgramBinary(cl_program     program,
                                      cl_device_id   device,
                                      vector<char>*  pBinary)
{
    // get a device count for this program.
    size_t nDevices = 0;
    cl_int err = g_realDispatchTable.GetProgramInfo(program,
                                                    CL_PROGRAM_NUM_DEVICES,
                                                    sizeof(nDevices),
                                                    &nDevices,
                                                    NULL);
    SpAssertRet(err == CL_SUCCESS) false;

    // grab the handles to all of the devices in the program.
    vector<cl_device_id> vDevices(nDevices);
    err = g_realDispatchTable.GetProgramInfo(program,
                                             CL_PROGRAM_DEVICES,
                                             sizeof(cl_device_id) * nDevices,
                                             &vDevices[0],
                                             NULL);
    SpAssertRet(err == CL_SUCCESS) false;

    // set the device index to match the relevant device
    bool   foundDevice = false;
    size_t deviceIndex = 0;

    for (size_t i = 0; i < nDevices && !foundDevice; i++)
    {
        if (vDevices[i] == device)
        {
            deviceIndex = i;
            foundDevice = true;
        }
    }

    // If this fails, we've done something very wrong
    if (!foundDevice)
    {
        return false;
    }

    // figure out the sizes of each of the binaries.
    size_t* pBinarySizes = new(std::nothrow) size_t[nDevices];
    SpAssertRet(pBinarySizes != NULL) false;

    err = g_realDispatchTable.GetProgramInfo(program,
                                             CL_PROGRAM_BINARY_SIZES,
                                             sizeof(size_t) * nDevices,
                                             pBinarySizes,
                                             NULL);
    SpAssertRet(err == CL_SUCCESS) false;

    // The slower way, until the runtime gets fixed and released
    // retrieve all of the generated binaries
    char** ppBinaries = new(std::nothrow) char* [nDevices];
    SpAssertRet(ppBinaries != NULL) false;

    pBinary->resize(pBinarySizes[deviceIndex]);

    for (size_t i = 0; i < nDevices; i++)
    {
        if (pBinarySizes[i] != 0)
        {
            // A slight speedup, which avoids the need for the trailing "copy"
            // Set the pointer for this particular item to be the first element of vBinary
            if (i == deviceIndex)
            {
                ppBinaries[deviceIndex] = &(*pBinary)[0];
            }
            else
            {
                ppBinaries[i] = new(std::nothrow) char[pBinarySizes[i]];
                SpAssertRet(ppBinaries != NULL) false;
            }
        }
        else
        {
            ppBinaries[i] = NULL;
        }
    }

    err = g_realDispatchTable.GetProgramInfo(program,
                                             CL_PROGRAM_BINARIES,
                                             sizeof(char*) * nDevices,
                                             ppBinaries,
                                             NULL);
    SpAssertRet(err == CL_SUCCESS) false;

    for (size_t i = 0; i < nDevices; i++)
    {
        if ((pBinarySizes[i] != 0) && (i != deviceIndex))
        {
            delete[] ppBinaries[i];
        }
    }

    // clean up
    delete[] pBinarySizes;
    delete[] ppBinaries;

    return true;
}

void KernelAssembly::DisassembleLogFunction(const char* pMsg, size_t size)
{
    SP_UNREFERENCED_PARAMETER(size);

    if (m_sDisassembleCount == 0)
    {
        m_sTmpDisassemblyLoggerISA = pMsg;
    }
    else if (m_sDisassembleCount == 1)
    {
        m_sTmpDisassemblyLoggerHSAIL = pMsg;
    }

    m_sDisassembleCount++;
}

bool SaveBifToFile(const char* pszFileName, ACLModule* mod, aclBinary* pBin)
{
    if (pBin == NULL || mod == NULL)
    {
        return false;
    }

    // write to file is not exposed yet
    //return mod.WriteToFile(pBin, szFileName) == ACL_SUCCESS;
    char* pszBin;
    size_t nSize;

    if (mod->WriteToMem(pBin, reinterpret_cast<void**>(&pszBin), &nSize) != ACL_SUCCESS)
    {
        return false;
    }

    ofstream fout(pszFileName, ios::binary);
    fout.write(pszBin, nSize);
    fout.close();

    return true;
}

bool GetCPUISA(CElf& elf, string& strISAOut)
{
    const CElfSection* pAstextSection = elf.GetSection(".astext");

    if (pAstextSection == NULL)
    {
        // ERROR
        Log(logERROR, "Failed to retrieve astext section.\n");
        return false;
    }
    else
    {
        const vector<char> data(pAstextSection->GetData());
        strISAOut = string(data.begin(), data.end());
    }

    return true;
}

bool KernelAssembly::DoesUseHSAILPath(const string& strDeviceName, cl_device_id device)
{

    bool retVal = true;

    // HSAIL path is not used on Linux 32-bit regardless of driver version, hardware family
#ifdef _LINUX
#ifdef X86
    retVal = false;
#endif
#endif

    if (retVal)
    {
        // HSAIL path is not used on SI hardware
        bool isSiFamily = false;
        AMDTDeviceInfoUtils::Instance()->IsSIFamily(strDeviceName.c_str(), isSiFamily);

        if (isSiFamily)
        {
            retVal = false;
        }

        if (retVal)
        {
            size_t versionLength;
            cl_int clRet = g_realDispatchTable.GetDeviceInfo(device, CL_DRIVER_VERSION, 0, NULL, &versionLength);

            if (clRet == CL_SUCCESS)
            {
                char* pszVersion = new(std::nothrow) char[versionLength];
                SpAssertRet(pszVersion != NULL) false;

                clRet = g_realDispatchTable.GetDeviceInfo(device, CL_DRIVER_VERSION, versionLength, pszVersion, NULL);

                if (clRet == CL_SUCCESS)
                {
                    // version string is of the format: "2000.3 (VM)"
                    string strOclVersion = string(pszVersion);
                    size_t spacePos = strOclVersion.find(" ");

                    if (string::npos != spacePos)
                    {
                        strOclVersion = strOclVersion.substr(0, spacePos);
                    }

                    int nOclVersion;

                    if (StringUtils::Parse(strOclVersion, nOclVersion))
                    {
                        retVal = nOclVersion >= 1983; // HSAIL path became the default for OCL 1.2 kernels starting with OCL driver version 1983
                    }
                }

                delete[] pszVersion;
            }
        }
    }

    return retVal;
}

bool KernelAssembly::GenerateKernelFilesFromACLModule(ACLModule*         pAclModule,
                                                      aclCompiler*       pAclCompiler,
                                                      std::vector<char>& vBinary,
                                                      const std::string& strKernelFunction,
                                                      const std::string& strKernelHandle,
                                                      const std::string& strOutputDir,
                                                      bool               isGPU,
                                                      bool               usesHSAILPath)
{
    bool bRet = false;
    std::string strFilename;

    // Compiler lib path
    acl_error err;
    aclBinary* pBin = pAclModule->ReadFromMem(&vBinary[0], vBinary.size(), &err);
    SpAssertRet(err == ACL_SUCCESS) false;

    if (m_bOutputISA || m_bOutputHSAIL)
    {
        m_sTmpDisassemblyLoggerISA.clear();
        m_sTmpDisassemblyLoggerHSAIL.clear();
        m_sDisassembleCount = 0;

        if (isGPU)
        {
            string strKernelName;
            string strKernelNameAlt;

            // For HSAIL kernels, try the "&__OpenCL..." kernel name first, as that is the most-likely kernel symbol name
            // For non-HSAIL kernels, try the undecorated kernel name first, as that is the most-likely kernel symbol name
            // In both cases, fall back to the other name if the most-likely name fails
            if (usesHSAILPath)
            {
                strKernelName = "&__OpenCL_" + strKernelFunction + "_kernel";
                strKernelNameAlt = strKernelFunction;
            }
            else
            {
                strKernelName = strKernelFunction;
                strKernelNameAlt = "&__OpenCL_" + strKernelFunction + "_kernel";
            }

            err = pAclModule->Disassemble(pAclCompiler, pBin, strKernelName.c_str(), DisassembleLogFunction);

            if (err != ACL_SUCCESS)
            {
                m_sTmpDisassemblyLoggerISA.clear();
                m_sTmpDisassemblyLoggerHSAIL.clear();
                m_sDisassembleCount = 0;
                err = pAclModule->Disassemble(pAclCompiler, pBin, strKernelNameAlt.c_str(), DisassembleLogFunction);
            }

            SpAssert(m_sDisassembleCount <= MAX_EXPECTED_DISASSEMBLE_CALLBACKS);

            bRet = ACL_SUCCESS == err;

            if (m_bOutputISA && !m_sTmpDisassemblyLoggerISA.empty() && ACL_SUCCESS == err)
            {
                strFilename = strOutputDir + m_strFilePrefix + strKernelHandle + ".isa";
                bRet = FileUtils::WriteFile(strFilename, m_sTmpDisassemblyLoggerISA);
            }

            if (m_bOutputHSAIL && !m_sTmpDisassemblyLoggerHSAIL.empty() && ACL_SUCCESS == err)
            {
                strFilename = strOutputDir + m_strFilePrefix + strKernelHandle + ".hsail";
                bRet = FileUtils::WriteFile(strFilename, m_sTmpDisassemblyLoggerHSAIL);
            }
        }
        else if (m_bOutputISA)
        {
            //astext
            const char* pAsText;
            size_t pAsTextSize;
            pAsText = reinterpret_cast<const char*>(pAclModule->ExtractSection(pAclCompiler, pBin, &pAsTextSize, aclASTEXT, &err));
            string strISA;
            bool bISA = false;

            if (pAsText != NULL && err == ACL_SUCCESS)
            {
                strISA = pAsText;
                bISA = true;
            }
            else
            {
                // Fallback to CELF path
                CElf elf(vBinary);

                if (elf.good())
                {
                    bISA = GetCPUISA(elf, strISA);
                }
            }

            if (bISA)
            {
                strFilename = strOutputDir + m_strFilePrefix + strKernelHandle + ".isa";
                bRet = FileUtils::WriteFile(strFilename, strISA);
            }
        }

#ifdef _DEBUG
        // dump original kernel
        ofstream fout("kernel2x.elf", ios::binary);
        fout.write(&vBinary[0], vBinary.size());
        fout.close();
        // dump the kernel binary out for investigation
        SaveBifToFile("kernel30.elf", pAclModule, pBin);
#endif
    }

    if (bRet && m_bOutputIL)
    {
        string amdilName = "__AMDIL_" + strKernelFunction + "_text";
        const char* pIL;
        size_t nILSize;
        pIL = reinterpret_cast<const char*>(pAclModule->ExtractSymbol(pAclCompiler, pBin, &nILSize, aclINTERNAL, amdilName.c_str(), &err));

        if (pIL != NULL && err == ACL_SUCCESS)
        {
            strFilename = strOutputDir + m_strFilePrefix + strKernelHandle + ".il";
            bRet = FileUtils::WriteFile(strFilename, pIL);
        }
    }

    err = pAclModule->BinaryFini(pBin);
    SpAssertRet(err == ACL_SUCCESS) false;

    return bRet;
}

bool KernelAssembly::GenerateKernelFiles(std::vector<char>& vBinary,
                                         const std::string& strKernelFunction,
                                         const std::string& strKernelHandle,
                                         const std::string& strOutputDir,
                                         bool               isGPU)
{
    bool bRet = true;

    ACLModule* pAclModuleHSAIL = nullptr;
    aclCompiler* pAclCompilerHSAIL = nullptr;
    ACLModule* pAclModuleAMDIL = nullptr;
    aclCompiler* pAclCompilerAMDIL = nullptr;

    // attempt to load the HSAIL-based ACL module
    bool hsailACLLoaded = ACLModuleManager::Instance()->GetACLModule(true, pAclModuleHSAIL, pAclCompilerHSAIL);

    // if the HSAIL ACL module was loaded, then try to use it to extract the kernel files.
    if (hsailACLLoaded)
    {
        bRet = GenerateKernelFilesFromACLModule(pAclModuleHSAIL, pAclCompilerHSAIL, vBinary, strKernelFunction, strKernelHandle, strOutputDir, isGPU, true);
    }

    // if using the HSAIL ACL module failed for one of the following reasons, then try the AMDIL ACL module:
    //   1) the HSAIL ACL module failed to load (as indicated by "hsailACLLoaded == false")
    //   2) the HSAIL ACL module failed to extract the kernel files (as indicated by "bRet == false")
    if (!hsailACLLoaded || !bRet)
    {
        ACLModuleManager::Instance()->GetACLModule(false, pAclModuleAMDIL, pAclCompilerAMDIL);
        bRet = GenerateKernelFilesFromACLModule(pAclModuleAMDIL, pAclCompilerAMDIL, vBinary, strKernelFunction, strKernelHandle, strOutputDir, isGPU, false);
    }

    return true;
}

bool KernelAssembly::Generate(const cl_command_queue& commandQueue,
                              const cl_kernel&        kernel,
                              const std::string&      strKernelFunction,
                              const std::string&      strKernelHandle,
                              const std::string&      strOutputDir)
{
    std::map<std::string, KernelInfo>::iterator it = m_assemblyGenerated.find(strKernelHandle);

    if (it != m_assemblyGenerated.end())
    {
        // the kernel assembly for this kernel has been generated
        return true;
    }

    // get the device id
    cl_device_id device;

    if (CL_SUCCESS != g_realDispatchTable.GetCommandQueueInfo(commandQueue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &device, NULL))
    {
        // error getting the device id
        return false;
    }

    // get the device name
    std::string strDeviceName;

    if (CL_SUCCESS != CLUtils::GetDeviceName(device, strDeviceName))
    {
        // error getting the device name
        return false;
    }

    bool isGPU = CLUtils::IsDeviceType(device, CL_DEVICE_TYPE_GPU);

    KernelInfo kernelInfo;

    if (isGPU)
    {
        if (CLUtils::QueryKernelInfo(kernel, strDeviceName, device, kernelInfo))
        {
            // save the kernel handle so we don't have to regenerate the kernel binary files for the same call.
            m_assemblyGenerated[ strKernelHandle ] = kernelInfo;
        }
    }

    vector<char> binary;
    cl_program program;
    bool bRet = g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(cl_program), &program, NULL) == CL_SUCCESS;

    bRet |= GetProgramBinary(program, device, &binary);

    if (bRet)
    {
        bRet &= GenerateKernelFiles(binary, strKernelFunction, strKernelHandle, strOutputDir, isGPU);
    }

    // Output the CL kernel source
    if (m_bOutputCL)
    {
        bRet &= DumpCLSource(kernel, strKernelHandle, strOutputDir);
    }

    return bRet;
}

bool KernelAssembly::DumpCLSource(const cl_kernel&   kernel,
                                  const std::string& strKernelHandle,
                                  const std::string& strOutputDir) const
{
    cl_int result = 0;

    // get the CL program handle
    cl_program program;
    result |= g_realDispatchTable.GetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(cl_program), &program, NULL);

    // Get the CL kernel source size
    size_t size;
    result |= g_realDispatchTable.GetProgramInfo(program, CL_PROGRAM_SOURCE, 0, NULL, &size);

    // Get the CL kernel source
    char* pszSource = new(std::nothrow) char[ size + 1 ];
    SpAssertRet(pszSource != NULL) false;

    result |= g_realDispatchTable.GetProgramInfo(program, CL_PROGRAM_SOURCE, size, pszSource, NULL);

    if (CL_SUCCESS != result)
    {
        delete[] pszSource;
        return false;
    }

    std::string strSource(pszSource);
    delete[] pszSource;

    std::string strFilename = strOutputDir + m_strFilePrefix + strKernelHandle + ".cl";
    // remove carriage return
    strSource.erase(std::remove(strSource.begin(), strSource.end(), '\r'), strSource.end());

    if (strSource.empty())
    {
        return false;
    }

    // write the CL text to a file
    if (!FileUtils::WriteFile(strFilename, strSource) ||
        CL_SUCCESS != result)
    {
        // something is wrong
        return false;
    }

    return true;
}

const KernelInfo& KernelAssembly::GetKernelInfo(std::string& strKernelName) const
{
    std::map<std::string, KernelInfo>::const_iterator it = m_assemblyGenerated.find(strKernelName);

    // For CPU device, we can't get KernelInfo, return default one. No need to assert
    //SpAssert( it != m_assemblyGenerated.end() );
    if (it != m_assemblyGenerated.end())
    {
        return it->second;
    }
    else
    {
        Log(logWARNING, "Default SCStat used\n");
        return m_kernelInfoDefault;
    }
}

bool KernelAssembly::ParseISASI(const std::string& strISA,
                                KernelInfo& kiOut) const
{
    if (strISA.empty())
    {
        return false;
    }

    // parse one line at a time
    std::string strLine;
    std::istringstream iss(strISA);
    std::stringstream ss;
    std::string strSkipToken, strSkipEqual;

    while (!std::getline(iss, strLine).eof())
    {
        // trim string
        boost::trim(strLine);

        if (strLine.length() <= 0)
        {
            // skip empty line
            continue;
        }

        if (strLine[0] == ';')
        {
            // skip comment line
            continue;
        }

        /*
              if (std::string::npos != strLine.find("NumVgprs"))
              {
                 // found the number of GPRs line
                 ss.clear();
                 ss.str("");
                 ss << strLine;

                 size_t numGPRs;

                 if (!(ss >> strSkipToken >> strSkipEqual >> numGPRs).fail())
                 {
                    SpAssert(kiOut.m_nUsedGPRs == numGPRs);
                    kiOut.m_nUsedGPRs = numGPRs;
                 }
              }
        */
        if (std::string::npos != strLine.find("ScratchSize"))
        {
            // found the number of scratch register line
            ss.clear();
            ss.str("");
            ss << strLine;

            size_t numScratchRegs;

            if (!(ss >> strSkipToken >> strSkipEqual >> numScratchRegs).fail())
            {
                //SpAssert(kiOut.m_nScratchReg == numScratchRegs);
                kiOut.m_nScratchReg = numScratchRegs;
                break; // stop iterating the lines in the file after we find the number of scratch registers
            }
        }

        /*
              if (std::string::npos != strLine.find("NumSgprs"))
              {
                 // found the stack size line
                 ss.clear();
                 ss.str("");
                 ss << strLine;

                 size_t numSGPRs;

                 if (!(ss >> strSkipToken >> strSkipEqual >> numSGPRs).fail())
                 {
                    kiOut.m_nUsedScalarGPRs = numSGPRs;
                 }
              }
        */
    }

    return true;
}
