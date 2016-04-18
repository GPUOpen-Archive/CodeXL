#ifndef kcCLICommanderOpenGL_h__
#define kcCLICommanderOpenGL_h__

// C++.
#include <map>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcCLICommander.h>
#include <AMDTKernelAnalyzerCLI/src/kcDataTypes.h>

// Backend.
#include <AMDTBackEnd/Include/beProgramBuilderOpenGL.h>

class kcCLICommanderOpenGL :
    public kcCLICommander
{
public:
    kcCLICommanderOpenGL();
    virtual ~kcCLICommanderOpenGL();

    virtual void ListAsics(Config& config, LoggingCallBackFunc_t callback) override;

    virtual void Version(Config& config, LoggingCallBackFunc_t callback) override;

    virtual void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback) override;

private:
    bool GetSupportedDevices();

    // The builder.
    beProgramBuilderOpenGL* m_pOglBuilder;

    // Unique collections of the device names.
    std::set<std::string> m_supportedDevicesCache;

    // An internal structure representing a target device's info.
    struct OpenGLDeviceInfo;
    std::map<std::string, OpenGLDeviceInfo> m_deviceInfo;
};

#endif // kcCLICommanderOpenGL_h__
