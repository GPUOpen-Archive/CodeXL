#ifndef _kcCLICommanderCL_H_
#define _kcCLICommanderCL_H_

// C++.
#include <string>
#include <set>

// Local.
#include <AMDTKernelAnalyzerCLI/src/kcCLICommander.h>
#include <AMDTKernelAnalyzerCLI/src/kcDataTypes.h>

using namespace std;

/// This is the Commander interface
class kcCLICommanderCL: kcCLICommander
{
public:

    kcCLICommanderCL();
    virtual ~kcCLICommanderCL();

    /// List the asics as got from device
    void ListAsics(Config& config, LoggingCallBackFunc_t callback);

    /// list the driver version
    void Version(Config& config, LoggingCallBackFunc_t callback);

    /// Output multiple commands for all commands that requires compilation: GetBinary, GetIL, GetISA, GetAnlysis, GetMetadata, GetDebugIL,ListKernels
    void RunCompileCommands(const Config& config, LoggingCallBackFunc_t callback);


private: // functions

    bool Init(const Config& config, LoggingCallBackFunc_t callback);
    bool InitRequestedAsicList(const Config& config);
    bool Compile(const Config& config);

    /// output for all commands that requires compilation
    void Analysis(const Config& config);
    void ListKernels(const Config& config);
    void GetILText(const Config& config);
    void GetISAText(const Config& config);
    void GetBinary(const Config& config);
    void GetMetadata(const Config& config);
    void GetDebugIL(const Config& config);

    /// Returns the list of required kernels according to the user's configurations.
    /// \param[in]  config - the configuration as given by the user.
    /// \param[out] requiredKernels - a container of the required kernels.
    void InitRequiredKernels(const Config& config, const std::set<std::string>& requiredDevices, std::vector<std::string>& requiredKernels);

private: //members
    std::set<string> m_devices;
    std::set<string> m_externalDevices;
    std::vector<GDT_GfxCardInfo> m_table;
    std::set<std::string> m_asics;

    // Holds the name of the kernels to be built.
    std::vector<std::string> m_requiredKernels;

    // True if the "--kernel all" option was specified by the user.
    bool m_isAllKernels;

    Backend* be;

};



#endif


