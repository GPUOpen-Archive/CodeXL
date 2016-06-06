//------------------------------ osModule.cpp ------------------------------

// Mach:
#include <fcntl.h>
#include <mach-o/arch.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osModule.h>

#define OS_FILE_HEADER_BUFFER_SIZE 1024


// ---------------------------------------------------------------------------
// Name:        osGetModuleArchitecture
// Description: Gets the module's architectures
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        31/8/2009
// ---------------------------------------------------------------------------
bool osGetModuleArchitectures(const osFilePath& modulePath, gtVector<osModuleArchitecture>& archs)
{
    bool retVal = false;
    archs.clear();

    osFilePath executableBinary = modulePath;
    gtString moduleExtension;
    modulePath.getFileExtension(moduleExtension);

    // If we are checking a bundle, get its executable:
    if ((moduleExtension == OS_MAC_APPLICATION_BUNDLE_FILE_EXTENSION) && modulePath.isDirectory())
    {
        executableBinary = osGetExecutableFromMacApplicationBundle(modulePath.asString());
    }

    GT_IF_WITH_ASSERT(executableBinary.isRegularFile())
    {
        // Code based off http://developer.apple.com/mac/library/samplecode/CheckExecutableArchitecture/listing1.html:
        int fileDescriptor = ::open(executableBinary.asString().asCharArray(), O_RDONLY, 0777);
        GT_IF_WITH_ASSERT(fileDescriptor >= 0)
        {
            gtByte* pFileHeader = new gtByte[OS_FILE_HEADER_BUFFER_SIZE];


            gtSize_t readBytes = ::read(fileDescriptor, pFileHeader, OS_FILE_HEADER_BUFFER_SIZE);
            GT_IF_WITH_ASSERT(readBytes > 0)
            {
                // Get the MAGIC number which represents what kind of file is this (FAT / Mach / Mach64 header + big / little endian)
                gtUInt32 fileMagicNumber = *((gtUInt32*)pFileHeader);

                // If the MAGIC number is reversed, swap each 4 bytes:
                if ((fileMagicNumber == FAT_CIGAM) || (fileMagicNumber == MH_CIGAM) || (fileMagicNumber == MH_CIGAM_64))
                {
                    for (int i = 0; i < (int)readBytes; i += 4)
                    {
                        *(gtUInt32*)(pFileHeader + i) = OSSwapInt32(*(gtUInt32*)(pFileHeader + i));
                    }
                }

                int numberOfArchitectures = 0;
                fat_arch* pFatArchitectures = NULL;
                bool wasArchAllocated = false;

                switch (fileMagicNumber)
                {
                    case FAT_MAGIC:
                    case FAT_CIGAM:
                    {
                        static const gtSize_t sizeOfFatHeader = sizeof(fat_header);
                        GT_IF_WITH_ASSERT(readBytes >= sizeOfFatHeader)
                        {
                            // Read the FAT header, and find out how many architectures are supported:
                            fat_header* pFatHeader = (fat_header*)pFileHeader;

                            // The FAT architecture structs follow immediately after the header:
                            pFatArchitectures = (fat_arch*)(pFileHeader + sizeOfFatHeader);
                            numberOfArchitectures = pFatHeader->nfat_arch;

                            // Truncate the number of architectures if the buffer was too short:
                            int maxNumberOfArchitecturesPossible = (int)((readBytes - sizeOfFatHeader) / sizeof(fat_arch));

                            if (numberOfArchitectures > maxNumberOfArchitecturesPossible)
                            {
                                OS_OUTPUT_DEBUG_LOG(L"FAT header too large, ignoring some architectures", OS_DEBUG_LOG_DEBUG);
                                numberOfArchitectures = maxNumberOfArchitecturesPossible;
                            }

                        }
                    }
                    break;

                    case MH_MAGIC:
                    case MH_CIGAM:
                    {
                        GT_IF_WITH_ASSERT(readBytes >= sizeof(mach_header))
                        {
                            // Read the mach header to find the cpu type
                            mach_header* pMachHeader = (mach_header*)pFileHeader;
                            pFatArchitectures = new fat_arch;

                            ::memset(pFatArchitectures, 0, sizeof(fat_arch));
                            pFatArchitectures->cputype = pMachHeader->cputype;
                            pFatArchitectures->cpusubtype = pMachHeader->cpusubtype;

                            // Note that the struct was allocated now and that it as just one arch type
                            numberOfArchitectures = 1;
                            wasArchAllocated = true;
                        }
                    }
                    break;

                    case MH_MAGIC_64:
                    case MH_CIGAM_64:
                    {
                        GT_IF_WITH_ASSERT(readBytes >= sizeof(mach_header_64))
                        {
                            // Read the mach 64 header to find the cpu type
                            mach_header_64* pMachHeader64 = (mach_header_64*)pFileHeader;
                            pFatArchitectures = new fat_arch;

                            ::memset(pFatArchitectures, 0, sizeof(fat_arch));
                            pFatArchitectures->cputype = pMachHeader64->cputype;
                            pFatArchitectures->cpusubtype = pMachHeader64->cpusubtype;

                            // Note that the struct was allocated now and that it as just one arch type
                            numberOfArchitectures = 1;
                            wasArchAllocated = true;
                        }
                    }
                    break;

                    default:
                    {
                        // unexpected value
                        GT_ASSERT(false);
                    }
                    break;
                }

                if ((numberOfArchitectures > 0) && (pFatArchitectures != NULL))
                {
                    // Mark the success:
                    retVal = true;

                    // Get the architecture info structs for the architecures we wish to test for.
                    // The names come from man arch(3). We could use CPU_TYPE_XXX and CPU_SUBTYPE_XXX_ALL,
                    // but this way is more robust:
                    const NXArchInfo* pI386ArchInfo = NXGetArchInfoFromName("i386");
                    const NXArchInfo* pX86_64ArchInfo = NXGetArchInfoFromName("x86_64");

                    // Count how many architectures we found:
                    int numberOfMatchedArchitectures = 0;

                    GT_IF_WITH_ASSERT(pI386ArchInfo != NULL)
                    {
                        fat_arch* pMatchedArch = NXFindBestFatArch(pI386ArchInfo->cputype, pI386ArchInfo->cpusubtype, pFatArchitectures, numberOfArchitectures);

                        if (pMatchedArch != NULL)
                        {
                            // The file contains an i386 architecture:
                            archs.push_back(OS_I386_ARCHITECTURE);
                            numberOfMatchedArchitectures++;
                        }
                    }

                    GT_IF_WITH_ASSERT(pX86_64ArchInfo != NULL)
                    {
                        fat_arch* pMatchedArch = NXFindBestFatArch(pX86_64ArchInfo->cputype, pX86_64ArchInfo->cpusubtype, pFatArchitectures, numberOfArchitectures);

                        if (pMatchedArch != NULL)
                        {
                            // The file contains an i386 architecture:
                            archs.push_back(OS_X86_64_ARCHITECTURE);
                            numberOfMatchedArchitectures++;
                        }
                    }

                    // Fill the vector with items for the architectures we didn't match:
                    for (int i = numberOfMatchedArchitectures; i < numberOfArchitectures; i++)
                    {
                        archs.push_back(OS_UNSUPPORTED_ARCHITECTURE);
                    }

                    // Make sure we got the right number of vector items:
                    GT_ASSERT((int)archs.size() == numberOfArchitectures);
                }

                if (wasArchAllocated)
                {
                    delete pFatArchitectures;
                    pFatArchitectures = NULL;
                }
            }

            delete[] pFileHeader;
            pFileHeader = NULL;
        }
    }

    return retVal;
}

