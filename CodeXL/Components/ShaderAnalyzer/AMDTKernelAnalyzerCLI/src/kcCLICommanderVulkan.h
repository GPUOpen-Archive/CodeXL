#ifndef __kcCLICommanderVulkan_h
#define __kcCLICommanderVulkan_h

// C++.
#include <string>
#include <sstream>
#include <vector>
#include <set>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcCLICommander.h>
#include <AMDTKernelAnalyzerCLI/src/kcDataTypes.h>

// Backend.
#include <AMDTBackEnd/Include/beProgramBuilderVulkan.h>

class kcCLICommanderVulkan :
    public kcCLICommander
{
public:
    kcCLICommanderVulkan();
    virtual ~kcCLICommanderVulkan();

    // List the supported ASICs.
    virtual void ListAsics(Config& config, LoggingCallBackFunc_t callback) override;

    // Print the Vulkan version.
    virtual void Version(Config& config, LoggingCallBackFunc_t callback) override;

    // Execute the build.
    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

private:

    // Caches the supported devices.
    bool GetSupportedDevices();

    // The builder.
    beProgramBuilderVulkan* m_pVulkanBuilder;

    // Unique collections of the device names.
    std::set<std::string> m_supportedDevicesCache;
};

#endif // __kcCLICommanderVulkan_h
