//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suInterceptionFunctions.cpp
///
//==================================================================================

//------------------------------ suInterceptionFunctions.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// Local:
#include <AMDTServerUtilities/Include/suInterceptionFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

void suTestFunction()
{
    return;
}

// ---------------------------------------------------------------------------
// Name:        gsExtensionsManager::getSpiesDirectory
// Description:
//   Returns the path of the "spies" directory.
//   Spies are DLLs that are used for DLL replacement.
// Arguments: spiesDirectory - Will get the spies directory.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        13/5/2007
// ---------------------------------------------------------------------------
bool suGetSpiesDirectory(gtString& spiesDirectory, bool isRunningInStandaloneMode)
{
    bool retVal = false;

    // Get the environment spies directory:
    bool rc1 = osGetCurrentProcessEnvVariableValue(OS_STR_envVar_spiesDirectory, spiesDirectory);

    if (rc1 == true)
    {
        retVal = true;
    }
    else
    {
        // If we are running in a "standalone" mode:
        if (isRunningInStandaloneMode)
        {
            // Allow debugging the OpenGL sever in a standalone mode:

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            {
                if (rc1 == false)
                {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    {
                        spiesDirectory = L"o:\\debug\\bin\\examples\\teapot";
                        retVal = true;
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                    {
                        // Get the current user home directory:
                        gtString currUserName;
                        bool rc2 = osGetCurrentProcessEnvVariableValue(L"USER", currUserName);
                        GT_ASSERT(rc2)
                        {
                            // Set the spies directory according to gremedy's work convention:
                            spiesDirectory = L"/home/";
                            spiesDirectory += currUserName;
                            spiesDirectory += L"/work/driveo/debug/bin/spies";

                            retVal = true;
                        }
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                    {
#ifdef _GR_IPHONE_DEVICE_BUILD
                        {
                            // Uri, 15/10/09 - we currently assume the CodeXL spy is inside the current bundle:
                            osFilePath currentAppDir;
                            retVal = osGetCurrentApplicationPath(currentAppDir);

                            if (retVal)
                            {
                                currentAppDir.clearFileExtension();
                                currentAppDir.clearFileName();
                                spiesDirectory = currentAppDir.asString();
                            }
                        }
#else // ndef _GR_IPHONE_DEVICE_BUILD
                        {
                            // Get the current user home directory:
                            gtString currUserName;
                            bool rc2 = osGetCurrentProcessEnvVariableValue("USER", currUserName);
                            GT_ASSERT(rc2)
                            {
                                // Set the spies directory according to gremedy's work convention:
                                spiesDirectory = "/Users/";
                                spiesDirectory += currUserName;
                                spiesDirectory += "/work/driveo/debug/bin/CodeXL_d.app/Contents/MacOS/spies";

                                retVal = true;
                            }
                        }
#endif // _GR_IPHONE_DEVICE_BUILD
                    }
#endif // AMDT_BUILD_TARGET
                }
            }
#elif AMDT_BUILD_CONFIGURATION == AMDT_RELEASE_BUILD
            {
                if (rc1 == false)
                {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    {
                        spiesDirectory = L"o:\\release\\bin\\examples\\teapot";
                        retVal = true;
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                    {
                        // Get the current user home directory:
                        gtString currUserName;
                        bool rc2 = osGetCurrentProcessEnvVariableValue(L"USER", currUserName);
                        GT_ASSERT(rc2)
                        {
                            // Set the spies directory according to gremedy's work convention:
                            spiesDirectory = L"/home/";
                            spiesDirectory += currUserName;
                            spiesDirectory += L"/work/driveo/release/bin/spies";

                            retVal = true;
                        }
                    }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                    {
#ifdef _GR_IPHONE_DEVICE_BUILD
                        {
                            // Uri, 15/10/09 - we currently assume the CodeXL spy is inside the current bundle:
                            osFilePath currentAppDir;
                            retVal = osGetCurrentApplicationPath(currentAppDir);

                            if (retVal)
                            {
                                currentAppDir.clearFileExtension();
                                currentAppDir.clearFileName();
                                spiesDirectory = currentAppDir.asString();
                            }
                        }
#else // ndef _GR_IPHONE_DEVICE_BUILD
                        {
                            // Get the current user home directory:
                            gtString currUserName;
                            bool rc2 = osGetCurrentProcessEnvVariableValue("USER", currUserName);
                            GT_ASSERT(rc2)
                            {
                                // Set the spies directory according to gremedy's work convention:
                                spiesDirectory = "/Users/";
                                spiesDirectory += currUserName;
                                spiesDirectory += "/work/driveo/release/bin/CodeXL.app/Contents/MacOS/spies";

                                retVal = true;
                            }
                        }
#endif // _GR_IPHONE_DEVICE_BUILD
                    }
#else
#error Unknown Linux variant!
#endif // AMDT_BUILD_TARGET
                }
            }
#endif // AMDT_BUILD_CONFIGURATION
        }
        else
        {
#ifdef _GR_IPHONE_DEVICE_BUILD
            {
                // Uri, 15/10/09 - we currently assume the CodeXL spy is inside the current bundle:
                osFilePath currentAppDir;
                retVal = osGetCurrentApplicationPath(currentAppDir);

                if (retVal)
                {
                    currentAppDir.clearFileExtension();
                    currentAppDir.clearFileName();
                    spiesDirectory = currentAppDir.asString();
                }
            }
#endif
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suHandleCRInSources
// Description: Works around a bug in the AMD compilers, by replacing each CR
//              character which is not followed by a LF with a CRLF.
//              This is only done if needed, on both OpenGL and OpenCL sources.
// Arguments:   sourceCount - number of source strings.
//              sources - the source strings.
//              sourceLengths - numeric values or 0 for null-terminated
//              o_modifiedSource - the output string if needed.
// Return Val: bool - was a change required.
// Author:      Uri Shomroni
// Date:        16/07/2015
// ---------------------------------------------------------------------------
bool SU_API suHandleCRInSources(unsigned int sourceCount, const char* const* sources, unsigned int* sourceLengths, gtASCIIString& o_modifiedSource)
{
    bool retVal = false;

    // Go over all the input strings:
    for (unsigned int i = 0; i < sourceCount; ++i)
    {
        const char* currentSource = sources[i];
        unsigned int l = (nullptr != sourceLengths) ? sourceLengths[i] : 0;

        // Variables used in the analysis:
        unsigned int j = 0;
        unsigned int j_ = 0;
        bool lastCharWasCR = false;

        // Go over the source string:
        bool goOn = (nullptr != currentSource);

        while (goOn)
        {
            const char& currentChar = currentSource[j];
            bool wasUnmatchedCR = lastCharWasCR && ('\x0A' != currentChar);

            if (wasUnmatchedCR)
            {
                // If this is the first time we've encountered an unmatched CR:
                if (!retVal)
                {
                    // Start by copying all the strings up to this point:
                    o_modifiedSource.makeEmpty();

                    for (unsigned int i_ = 0; i_ < i; ++i_)
                    {
                        // Handle null strings and lengths specified:
                        const char* currentSource_ = sources[i_];

                        if (nullptr != currentSource_)
                        {
                            // Consider the length parameter:
                            unsigned int l_ = (nullptr != sourceLengths) ? sourceLengths[i_] : 0;

                            if (l_ > 0)
                            {
                                o_modifiedSource.append(currentSource_, l_);
                            }
                            else
                            {
                                o_modifiedSource.append(currentSource_);
                            }
                        }
                    }

                    // Now copy the start of the current string:
                    o_modifiedSource.append(currentSource, j); // Note that j > 0 since we saw at least the CR.
                    j_ = j;

                    // Finally, note that we need to replace the string:
                    retVal = true;
                }
                else // retVal
                {
                    // Copy everything up to this point:
                    o_modifiedSource.append(&(currentSource[j_]), j - j_);
                    j_ = j;
                }

                // Follow it with a LF:
                o_modifiedSource.append('\x0A');
            }

            // Update the last character. Stop at a null character:
            lastCharWasCR = ('\x0D' == currentChar);
            goOn = goOn && ('\0' != currentChar);

            ++j;

            if (0 < l)
            {
                goOn = goOn && (j < l);
            }
        }

        // If we're creating a new string, copy the current string's remainder:
        if (retVal)
        {
            // Consider the length parameter:
            if (l > 0)
            {
                o_modifiedSource.append(&(currentSource[j_]), l - j_);
            }
            else
            {
                o_modifiedSource.append(&(currentSource[j_]));
            }
        }
    }

    return retVal;
}
