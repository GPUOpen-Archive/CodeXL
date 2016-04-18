//------------------------------ kaAppWrapper.h ------------------------------

#ifndef __KAAPPWRAPPER_H
#define __KAAPPWRAPPER_H

// Forward declarations:

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>

class kaAnalyzeSettingsPage;
class kaEventObserver;
class kaExecutionMode;
class kaProjectSettingsExtension;
class kaProjectSettingsExtensionOther;
class kaProjectSettingsShaderExtension;
class kaMDIViewCreator;
class kaBuildToolbar;

class KA_API kaAppWrapper
{
public:

    static kaAppWrapper& instance();
    int CheckValidity(gtString& errString);
    void initialize();
    void initializeIndependentWidgets();

    static kaBuildToolbar* buildToolbar() { return m_psBuildToolbar; }

    // marks if the prerequisite of this plugin were met needs access
    static bool s_loadEnabled;

protected:

    // Do not allow the use of my default constructor:
    kaAppWrapper();

    static kaAppWrapper* m_spMySingleInstance;

    // target device settings page:
    static kaAnalyzeSettingsPage* m_spAnalyzeSettingsPage;

    // Contain the application event observer:
    static kaEventObserver* m_spApplicationEventObserver;

    // Contain the KA execution mode:
    static kaExecutionMode* m_spKAExecutionMode;

    // Contains the project settings extension
    static kaProjectSettingsExtension* m_psProjectSettingsExtension;

    // Contains the project settings extension
    static kaProjectSettingsExtensionOther* m_psProjectSettingsExtensionOther;

    // Contains the project settings extension
    static kaProjectSettingsShaderExtension* m_psProjectSettingsShaderExtension;

    // Contains the views creator
    static kaMDIViewCreator* m_psMDIViewCreator;

    // Contains the toolbar
    static kaBuildToolbar* m_psBuildToolbar;
};

extern "C"
{
    // check validity of the plugin:
    int KA_API CheckValidity(gtString& errString);

    // initialize function for this wrapper:
    void KA_API initialize();

    // Initialize other items after main window creation:
    void KA_API initializeIndependentWidgets();
};


#endif //__GWGDEBUGGERAPPWRAPPER_H

