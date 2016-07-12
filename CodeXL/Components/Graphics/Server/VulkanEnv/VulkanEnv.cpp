//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   VulkanServer.cpp
/// \brief  Implementation file for Vulkan server main entry point.
///         Initializes and destroys the Vulkan layer manager.
//==============================================================================

#define GNU_SOURCE             // for program_invocation_short_name
#include <errno.h>
#include <stdio.h>

#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include "../Common/misc.h"
#include "../Common/Linux/ServerUtils.h"
#include "VulkanEnv.h"

//-----------------------------------------------------------------------------
/// Linux Shared Library Constructor. Read in the Vulkan environment variables
/// and set them.
///
/// Only limited setup is allowed here. Trying to initialize global data, particularly
/// STL objects, will lead to a seg fault, probably because the shared library isn't
/// fully loaded at this point
//-----------------------------------------------------------------------------
__attribute__((constructor))
static void ctor()
{
    if (ServerUtils::CanBind(program_invocation_short_name))
    {
        // read in and set up the Vulkan environment variables
        gtString toolFolder;
        toolFolder.fromASCIIString(GetPerfStudioDirName());

        osFilePath envFilePath;
        envFilePath.setPath(osFilePath::OS_TEMP_DIRECTORY);
        envFilePath.appendSubDirectory(toolFolder);

        gtString filePath = envFilePath.asString();
        filePath += ENV_FILENAME;

        FILE* fp = fopen(filePath.asASCIICharArray(), "rb");
        if (fp)
        {
            char path[512] = {};
            char name[32] = {};
            int32 pathLength = 0;
            int32 nameLength = 0;

            int bytesRead = fread(&pathLength, sizeof(int32), 1, fp);
            if (bytesRead > 0)
            {
                bytesRead = fread(&nameLength, sizeof(int32), 1, fp);
                if (bytesRead > 0)
                {
                    bytesRead = fread(path, pathLength, 1, fp);
                    if (bytesRead > 0)
                    {
                        bytesRead = fread(name, nameLength, 1, fp);
                        if (bytesRead > 0)
                        {
                            gtString layerName;
                            layerName.fromASCIIString(name);

                            osEnvironmentVariable layerPath;
                            layerPath._name = L"VK_LAYER_PATH";
                            layerPath._value.fromASCIIString(path);

                            osEnvironmentVariable instanceLayerName;
                            instanceLayerName._name = L"VK_INSTANCE_LAYERS";
                            instanceLayerName._value = layerName;

                            osEnvironmentVariable deviceLayerName;
                            deviceLayerName._name = L"VK_DEVICE_LAYERS";
                            deviceLayerName._value = layerName;

                            osSetCurrentProcessEnvVariable(layerPath);
                            osSetCurrentProcessEnvVariable(instanceLayerName);
                            osSetCurrentProcessEnvVariable(deviceLayerName);
                        }
                    }
                }
            }
            fclose(fp);
        }
    }
}

//-----------------------------------------------------------------------------
/// Linux Shared Library Destructor
//-----------------------------------------------------------------------------
__attribute__((destructor))
static void dtor()
{
}

