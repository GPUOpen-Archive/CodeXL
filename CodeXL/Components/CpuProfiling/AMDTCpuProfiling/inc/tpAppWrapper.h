//------------------------------ tpAppWrapper.h ------------------------------

#ifndef __TPAPPWRAPPER_H
#define __TPAPPWRAPPER_H

// Forward declarations:

// Local:
#include <AMDTThreadProfiling/Include/tpAMDTThreadProfilingDLLBuild.h>

class tpProjectSettingsExtension;

// ----------------------------------------------------------------------------------
// Class Name:          tpAppWrapper
// General Description: This class is a single instance class used to initialize the
//                      thread profiling plugin dll
// ----------------------------------------------------------------------------------
class tpAppWrapper
{
public:

    static tpAppWrapper& Instance();
    int CheckValidity(gtString& errString);
    void initialize();

    /// Initializes all the widget items that are not registered with the creators mechanism.
    /// These widgets are responsible for its own callbacks and string
    void initializeIndependentWidgets();

protected:

    // Do not allow the use of my default constructor:
    tpAppWrapper();

    static tpAppWrapper* m_spMySingleInstance;

    // Contains the project settings extension:
    static tpProjectSettingsExtension* m_psProjectSettingsExtension;

};

extern "C"
{
    // check validity of the plugin:
    int TP_API CheckValidity(gtString& errString);

    // initialize function for this wrapper:
    void TP_API initialize();

    // Initialize other items after main window creation:
    void TP_API initializeIndependentWidgets();
};


#endif //__TPAPPWRAPPER_H

